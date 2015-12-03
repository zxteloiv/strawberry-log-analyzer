// Worker.cpp
// Implements Worker class.
//

#include "stdafx.h"
#include "Worker.h"
#include "AggregatorManager.h"
#include "Aggregator.h"
#include "RecordDepository.h"
#include "QuestionTree.h"
#include "..\\Toolkit\\Toolkit.h"
#include "..\\BaseUtils\\BaseUtils.h"

#include "DBAccessLogic.h"

#include <atlbase.h>
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

using namespace BaseUtils;
using namespace ProcessCore;
using namespace Toolkit;

const TCHAR* Worker::s_szElemResult = _T("result");
const TCHAR* Worker::s_szElemQuestion = _T("question");
const TCHAR* Worker::s_szAttrQuestionID = _T("id");
const TCHAR* Worker::s_szAttrRootAggID = _T("root_agg");

Worker::Worker() {
	m_pAggMgr = NULL;
	m_pDepo = NULL;
	m_pDBConn = NULL;
	m_nRequestID = 0;
	m_nLogID = 0;
	m_pWriter = NULL;
	m_pWriterOutput = NULL;
	m_pStream = NULL;
	m_pQTreeList = NULL;
}

Worker::~Worker() {
	DELETE_POINTER(m_pAggMgr);
	DELETE_POINTER(m_pDBConn);
	DELETE_POINTER(m_pDepo);
	RELEASE_COM_OBJECT(m_pWriter);
	RELEASE_COM_OBJECT(m_pWriterOutput);
	RELEASE_COM_OBJECT(m_pStream);
	QuestionTree::CleanQTreeList(&m_pQTreeList);	// after this call, m_pQTreeList will be set NULL

}

bool Worker::Init(int nRequestID) {
	bool bSuccess = false;
	m_nRequestID = nRequestID;
	m_pAggMgr = new AggregatorManager;
	m_pDepo = new RecordDepository;
	m_pDBConn = new ADOConnector;
	PROCESS_ERROR(m_pAggMgr != NULL && m_pDepo != NULL && m_pDBConn != NULL);

	// Get ready for reading database 
	PROCESS_ERROR(m_pDBConn->Init());
	PROCESS_ERROR(m_pDBConn->Open());

	// After initialize the Worker, start to prepare for request processing.
	PROCESS_ERROR(DBAccessLogic::UpdateRequestToPreparing(nRequestID, m_pDBConn));
	// Fetch LogID and RequestXML by RequestID
	PROCESS_ERROR(DBAccessLogic::GetLogIDByRequestID(m_nRequestID, &m_nLogID, m_pDBConn));
	PROCESS_ERROR(m_pDepo->Init(m_nLogID));
	PROCESS_ERROR(m_pDepo->IsLogAlive());	// the log must exist, otherwise further access to database is not needed
	TCHAR* pXmlString = NULL;
	PROCESS_ERROR(DBAccessLogic::GetRequestXMLByRequestID(m_nRequestID, &pXmlString, m_pDBConn));

	// Initialize the aggregators
	PROCESS_ERROR(m_pAggMgr->Init());
	PROCESS_ERROR(m_pAggMgr->BuildAggListFromXML(pXmlString));
	PROCESS_ERROR(QuestionTree::BuildQTreeListFromXML(pXmlString, &m_pQTreeList));

	bSuccess = true;
Exit0:
	// After building QuestionTree and Aggregator Graph, the request definition XML string is no longer needed.
	DBAccessLogic::FreeRequestXML(pXmlString);
	if (!bSuccess) {
		DELETE_POINTER(m_pAggMgr);
		DELETE_POINTER(m_pDepo);
		DELETE_POINTER(m_pDBConn);
	}
	return bSuccess;
}

bool Worker::DoStuff() {
	bool bSuccess = false;
	PROCESS_ERROR(DBAccessLogic::UpdateRequestToProcessing(m_nRequestID, m_pDBConn));

	PROCESS_ERROR(StartSavingResult());

	Aggregator* pRootAgg = NULL;

	int nQuestionID = 0;
	for (QUESTITER iter = m_pQTreeList->begin(); iter != m_pQTreeList->end(); ++iter) {
		const QuestionTreeNode* pRootNode = (*iter)->GetRootNode();
		PROCESS_ERROR(m_pAggMgr->CreateAggregator(&pRootAgg, pRootNode));
		// Read depository for the first time in case of any aggregator that needs accumulation exists
		PROCESS_ERROR(m_pDepo->StartDepository());
		while (m_pDepo->HasAnyMoreRecords()) {
			_Record_Ptr pRec = m_pDepo->GetARecord();
			if (NULL == pRec)
				continue;

			// Preprocess record.
			// Currently not implemented

			PROCESS_ERROR(pRootAgg->ReceiveRecord(pRec));
			m_pDepo->ReleaseRecord(pRec);
		}
		PROCESS_ERROR(m_pDepo->ShutdownDepository());

		// In the previous loop in which we read the log file, all the ranges in all aggregators are
		// inserted successfully. Then reset the range values because a range may not exist and therefore
		// has not do the aggregate when a record is received by an aggregator. That leads the inaccurancy
		// of the aggregated result.
		PROCESS_ERROR(pRootAgg->ResetAggregateValue());

		// Reading the depository from the beginning again, and in this time, the accumulation will
		// be performed correctly.
		PROCESS_ERROR(m_pDepo->StartDepository());
		while (m_pDepo->HasAnyMoreRecords()) {
			_Record_Ptr pRec = m_pDepo->GetARecord();
			if (NULL == pRec)
				continue;
			PROCESS_ERROR(pRootAgg->ReceiveRecord(pRec));
			m_pDepo->ReleaseRecord(pRec);
		}

		PROCESS_ERROR(m_pDepo->ShutdownDepository());

		// save result before release all related aggregators for this question
		PROCESS_ERROR(SaveQuestionResult(nQuestionID, pRootNode->m_nAggID));

		m_pAggMgr->ReleaseDerivedAggs();
		++nQuestionID;
	}

	PROCESS_ERROR(EndSavingResultAndUpdateDB());

	PROCESS_ERROR(DBAccessLogic::UpdateRequestToSuccess(m_nRequestID, m_pDBConn));
	bSuccess = true;
Exit0:
	if (!bSuccess)
		DBAccessLogic::UpdateRequestToFailed(m_nRequestID, m_pDBConn);
	return bSuccess;
}

bool Worker::StartSavingResult() {
	// initialize the Xml Writer object
	TCHAR szTempFile[MAX_PATH] = _T("");
	RETURN_ON_FAIL(0 < _stprintf_s(szTempFile, MAX_PATH, _T("output%d.xml"), m_nRequestID));
	PROCESS_ERROR(S_OK == SHCreateStreamOnFile(szTempFile, STGM_CREATE | STGM_WRITE, &m_pStream));
	PROCESS_ERROR(S_OK == CreateXmlWriter(__uuidof(IXmlWriter), (void**)&m_pWriter, NULL));
	PROCESS_ERROR(S_OK == CreateXmlWriterOutputWithEncodingCodePage(m_pStream, NULL, 1200, &m_pWriterOutput));
	PROCESS_ERROR(S_OK == (m_pWriter->SetOutput(m_pWriterOutput)));
	PROCESS_ERROR(S_OK == m_pWriter->SetProperty(XmlWriterProperty_Indent, TRUE));

	// write the <result> element as the start
	PROCESS_ERROR(S_OK == m_pWriter->WriteStartElement(NULL, s_szElemResult, NULL));
	
	return true;
Exit0:
	RELEASE_COM_OBJECT(m_pWriter);
	RELEASE_COM_OBJECT(m_pWriterOutput);
	RELEASE_COM_OBJECT(m_pStream);
	DeleteFile(szTempFile);
	return false;
}

bool Worker::EndSavingResultAndUpdateDB() {
	bool bSuccess = false;
	PROCESS_ERROR(S_OK == m_pWriter->WriteEndDocument());
	PROCESS_ERROR(S_OK == m_pWriter->Flush());

	TCHAR szTempFile[MAX_PATH] = _T("");
	PROCESS_ERROR(0 < _stprintf_s(szTempFile, MAX_PATH, _T("output%d.xml"), m_nRequestID));
	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	hTempFile = CreateFile(szTempFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	RETURN_ON_FAIL(hTempFile != INVALID_HANDLE_VALUE);

	DWORD dwError = NO_ERROR;
	DWORD dwFileSizeHigh = 0;
	DWORD dwFileSize = 0;
	DWORD dwReadBytes = 0;				// bytes read by ReadFile API
	const int UTF16LE_BOM_LEN = 2;		// a UTF-16LE BOM is 2 bytes
	BYTE UTF16LE_BOM[UTF16LE_BOM_LEN];	// store the UTF-16LE BOM in this file
	TCHAR* pXmlBuf = NULL;	// used to store the XML string to write to database
	// character count of the XML string buffer, including the null-teminator
	int	ccXmlBuf = 0;

	// Get File size
	dwFileSize = GetFileSize(hTempFile, &dwFileSizeHigh);
	PROCESS_ERROR(NO_ERROR == (dwError = GetLastError()));
	// calculate the buffer size, excluding the BOM and including the null-terminator
	ccXmlBuf = (dwFileSize - UTF16LE_BOM_LEN) / sizeof(TCHAR) + 1;
	// Read BOM
	PROCESS_ERROR(TRUE == ReadFile(hTempFile, UTF16LE_BOM, UTF16LE_BOM_LEN, &dwReadBytes, NULL));
	PROCESS_ERROR(UTF16LE_BOM_LEN == dwReadBytes);
	// Read remaining file
	dwFileSize -= UTF16LE_BOM_LEN;
	PROCESS_ERROR(NULL != (pXmlBuf = new TCHAR[ccXmlBuf]));
	memset(pXmlBuf, 0, ccXmlBuf * sizeof(TCHAR));
	PROCESS_ERROR(TRUE == ReadFile(hTempFile, pXmlBuf, dwFileSize, &dwReadBytes, NULL));
	PROCESS_ERROR(dwReadBytes == dwFileSize);
	// the buffer includes the FF FE BOM of UTF-16LE now.

	PROCESS_ERROR(DBAccessLogic::SaveResultForRequest(m_nRequestID, m_nLogID, pXmlBuf, m_pDBConn));

	bSuccess = true;
Exit0:
	RELEASE_COM_OBJECT(m_pWriter);
	RELEASE_COM_OBJECT(m_pWriterOutput);
	RELEASE_COM_OBJECT(m_pStream);
	DELETE_ARRAY_POINTER(pXmlBuf);
	if (hTempFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hTempFile);
		DeleteFile(szTempFile);
	}
	return bSuccess;
}

bool Worker::SaveQuestionResult(int nQuestionID, int nRootAggID) {
	RETURN_ON_FAIL(m_pWriter != NULL);
	// write the <question> element
	RETURN_ON_FAIL(S_OK == m_pWriter->WriteStartElement(NULL, s_szElemQuestion, NULL));
	// id="" in <question>
	TCHAR szNumBuf[12] = _T("");	// the maximum integer has 10 digits, a string with length of 12 is quite enough
	RETURN_ON_FAIL(0 == _itot_s(nQuestionID, szNumBuf, 12, 10));
	RETURN_ON_FAIL(S_OK == m_pWriter->WriteAttributeString(NULL, s_szAttrQuestionID, NULL, szNumBuf));
	// ATTRIBUTE root_agg, the id of the root aggregator for this question
	RETURN_ON_FAIL(0 == _itot_s(nRootAggID, szNumBuf, 12, 10));
	RETURN_ON_FAIL(S_OK == m_pWriter->WriteAttributeString(NULL, s_szAttrRootAggID, NULL, szNumBuf));

	// Inside <question> element, write all those aggregators
	RETURN_ON_FAIL(m_pAggMgr->SaveQuestionResult(m_pWriter));

	RETURN_ON_FAIL(S_OK == m_pWriter->WriteFullEndElement());	// write </question>
	
	return true;
}
