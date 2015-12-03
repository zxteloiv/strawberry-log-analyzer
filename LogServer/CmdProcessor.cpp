// CmdProcessor.cpp
// Implementation of CmdProcessor class.
//

#include "stdafx.h"
#include "CmdProcessor.h"
#include "DBAccessLogic.h"
#include "LogExchangeProtocol.h"
#include "RecordDef.h"
#include "FieldDef.h"
#include "FixedLengthCompare.h"
#include "..\\BaseUtils\\BaseUtils.h"
#include <list>
#include <vector>
#include <string>
#include <errno.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define DELETE_ANSI_CHECKERS_VECTOR(X)	\
	if ((X) != NULL) {	\
		for (LogServer::ANSIFLCITER iter = (X)->begin(); iter != (X)->end(); ++iter) {	\
				DELETE_POINTER(*iter);	\
		}	\
		DELETE_POINTER((X));	\
	}

#define DELETE_WIDE_CHECKERS_VECTOR(X)	\
	if ((X) != NULL) {	\
		for (LogServer::WIDEFLCITER iter = (X)->begin(); iter != (X)->end(); ++iter) {	\
				DELETE_POINTER(*iter);	\
		}	\
		DELETE_POINTER((X));	\
	}

using namespace LogServer;

const TCHAR* CodePage::cp_string[] = {
	_T("shift_jis"),
	_T("gb2312"),
	_T("big5"),
	_T("hangul"),
	_T("ascii"),
	_T("gb18030"),
	_T("utf7"),
	_T("utf8")
};

UINT CodePage::GetCodePageID(const TCHAR* szCodePage) {
	if (0 == _tcsicmp(szCodePage, _T("shift_jis")))
		return CodePage::Shift_JIS_ANSI;
	else if (0 == _tcsicmp(szCodePage, _T("gb2312")))
		return CodePage::GB2312_ANSI;
	else if (0 == _tcsicmp(szCodePage, _T("big5")))
		return CodePage::BIG5_ANSI;
	else if (0 == _tcsicmp(szCodePage, _T("hangul")))
		return CodePage::Unified_Hangul_ANSI;
	else if (0 == _tcsicmp(szCodePage, _T("ascii")))
		return CodePage::ASCII_US;
	else if (0 == _tcsicmp(szCodePage, _T("gb18030")))
		return CodePage::GB18030;
	else if (0 == _tcsicmp(szCodePage, _T("utf8")))
		return CodePage::UTF8;
	else // error
		return 0;
};

CmdProcessor::CmdProcessor() {
	m_pRecDef = NULL;
	m_pDBConn = NULL;
	m_pFile = NULL;
	m_szFilename = NULL;
	m_nVersion = 0;
	m_nCmdID = CMD_ID_INVALID;
	m_nParamCount = 0;
	m_nCmdSize = 0;
	m_nBOM = 0;
	m_pResponseBuf = NULL;
	m_nResponseSize = 0;
	m_pAnsiRecStartChecker = NULL;
	m_pAnsiRecEndChecker = NULL;
	m_pAnsiFieldStartCheckers = NULL;
	m_pAnsiFieldEndCheckers = NULL;
	m_pWideRecStartChecker = NULL;
	m_pWideRecEndChecker = NULL;
	m_pWideFieldStartCheckers = NULL;
	m_pWideFieldEndCheckers = NULL;
}

CmdProcessor::~CmdProcessor() {
	CloseCFile();
	m_pDBConn->Close();
	DELETE_POINTER(m_pRecDef);
	DELETE_POINTER(m_pDBConn);
}

bool CmdProcessor::Init() {
	RETURN_ON_FAIL(NULL != (m_pDBConn = new Toolkit::ADOConnector));
	PROCESS_ERROR(m_pDBConn->Init());
	PROCESS_ERROR(m_pDBConn->Open());
	return true;
Exit0:
	DELETE_POINTER(m_pDBConn);
	return false;
}

bool CmdProcessor::ParseCmdBuf(
			const char*	pBuf,
			int			nBufSize,
			int*		pVersion,
			int*		pCmdID,
			int*		pCmdParamCount,
			int*		pSize)
{
	RETURN_ON_FAIL(nBufSize >= CMD_HEADER_SIZE);
	// Return when the buffer is not a command whose first bit inside flag byte is 0
	char options = pBuf[FLAG_OPTION_OFFSET];
	RETURN_ON_FAIL(OPTION_CMD == (options & CMD_MASK_RTN));

	int nVersion = (int)((BYTE)pBuf[FLAG_VERSION_OFFSET]);
	int nCmdID = (int)((BYTE)pBuf[FLAG_CMDID_OFFSET]);
	RETURN_ON_FAIL(nCmdID == CMD_ID_CLOSEREQUEST || nCmdID == CMD_ID_EXTENDCMD_0
		|| nCmdID == CMD_ID_HASMORERECORD || nCmdID == CMD_ID_LOADRECORD
		|| nCmdID == CMD_ID_REQUESTLOG || nCmdID == CMD_ID_VALIDATE);
	int nParamCount = (int)((BYTE)pBuf[FLAG_PARAM_OFFSET]);
	int nSize = 0;

	// Copy the header size integer from the buffer, which starts at address (pBuf + 4)
	RETURN_ON_FAIL(0 == memcpy_s(&nSize, sizeof(nSize), pBuf + FLAG_CMD_SIZE_OFFSET, FLAG_CMD_SIZE_LEN));

	m_nVersion = nVersion;
	m_nCmdID = nCmdID;
	m_nParamCount = nParamCount;
	m_nCmdSize = nSize;

	if (pVersion)
		*pVersion = m_nVersion;
	if (pCmdID)
		*pCmdID = m_nCmdID;
	if (pCmdParamCount)
		*pCmdParamCount = m_nParamCount;
	if (pSize)
		*pSize = m_nCmdSize;
	
	return true;
}

bool CmdProcessor::ExecuteCommand(const char*	pParamBuf,
								  int			nParamBufSize,
								  const char*	pHeaderBuf,
								  int			nHeaderBufSize)
{
	if (pHeaderBuf) {
		RETURN_ON_FAIL(ParseCmdBuf(pHeaderBuf, nHeaderBufSize));
	}
	switch (m_nCmdID) {
	case CMD_ID_VALIDATE:
		RETURN_ON_FAIL(BuildResponseWithBool(DispatchParamWithInt(&CmdProcessor::ValidateLog, pParamBuf, nParamBufSize)));
		break;
	case CMD_ID_REQUESTLOG:
		RETURN_ON_FAIL(BuildResponseWithBool(DispatchParamWithInt(&CmdProcessor::RequestForLog, pParamBuf, nParamBufSize)));
		break;
	case CMD_ID_HASMORERECORD:
		RETURN_ON_FAIL(BuildResponseWithBool(DispatchNoParam(&CmdProcessor::HasMoreRecords, pParamBuf, nParamBufSize)));
		break;
	case CMD_ID_LOADRECORD:
		// in this case, the buffer shall be built by the LoadRecord on its own.
		RETURN_ON_FAIL(DispatchParamWithInt(&CmdProcessor::LoadRecord, pParamBuf, nParamBufSize));
		break;
	case CMD_ID_CLOSEREQUEST:
		RETURN_ON_FAIL(BuildResponseWithBool(DispatchNoParam(&CmdProcessor::CloseRequest, pParamBuf, nParamBufSize)));
		break;
	}
	return true;
}

bool CmdProcessor::DispatchNoParam(BOOL_FUNC_VOID pFun,
								   const char *pParamBuf,
								   int nBufSize)
{
	// the buffer must have exactly the length as the header is
	RETURN_ON_FAIL(pParamBuf != NULL && CMD_HEADER_SIZE == nBufSize);
	return ((this->*pFun)());
}

// DispatchParamWithInt
// This function is used to extract an integer parameter from parameter buffer.
bool CmdProcessor::DispatchParamWithInt(BOOL_FUNC_INT pFun,
										const char *pParamBuf,
										int nBufSize)
{
	int buf_cursor = 0;
	// Function will fail when the command parameter is empty or not enough.
	// Extract the length of the first parameter which is following the length variable
	RETURN_ON_FAIL(pParamBuf != NULL && FLAG_CMD_SIZE_LEN < nBufSize);
	int nParamSize = 0;
	RETURN_ON_FAIL(0 == memcpy_s(&nParamSize, sizeof(nParamSize), pParamBuf + buf_cursor, FLAG_CMD_SIZE_LEN));
	buf_cursor += FLAG_CMD_SIZE_LEN;

	// Buffer is enough for the first variable.
	// Extract the true parameter.
	RETURN_ON_FAIL(FLAG_CMD_SIZE_LEN + nParamSize <= nBufSize);
	int nParam = 0;
	RETURN_ON_FAIL(0 == memcpy_s(&nParam, sizeof(nParam), pParamBuf + buf_cursor, nParamSize));

	// Dispatch the parameter to the designated function
	RETURN_ON_FAIL((this->*pFun)(nParam));
	
	return true;
}

void CmdProcessor::CleanResponse() {
	DELETE_ARRAY_POINTER(m_pResponseBuf);
	m_nResponseSize = 0;
}

bool CmdProcessor::BuildResponseWithBool(bool rtn) {
	CleanResponse();
	int nRtnSize = sizeof(rtn);
	// only one parameter will be inserted into the response buffer
	int nResponseSize = CMD_HEADER_SIZE + sizeof(nRtnSize) + nRtnSize;
	m_pResponseBuf = new char[nResponseSize];
	RETURN_ON_FAIL(m_pResponseBuf != NULL);
	m_pResponseBuf[FLAG_VERSION_OFFSET] = (char)((BYTE)m_nVersion);
	m_pResponseBuf[FLAG_OPTION_OFFSET] = (char)((BYTE)CMD_MASK_RTN);
	m_pResponseBuf[FLAG_CMDID_OFFSET] = (char)((BYTE)m_nCmdID);
	m_pResponseBuf[FLAG_PARAM_OFFSET] = (char)((BYTE)1);	// there's only 1 parameter
	
	PROCESS_ERROR(0 == memcpy_s(m_pResponseBuf + FLAG_CMD_SIZE_OFFSET,
		FLAG_CMD_SIZE_LEN, &nResponseSize, sizeof(nResponseSize)));

	int offset = CMD_HEADER_SIZE;
	PROCESS_ERROR(0 == memcpy_s(m_pResponseBuf + offset, CMD_PARAM_SIZE_LEN, &nRtnSize, sizeof(nRtnSize)));
	offset += sizeof(nRtnSize);
	PROCESS_ERROR(0 == memcpy_s(m_pResponseBuf + offset, nResponseSize - offset, &rtn, nRtnSize));

	m_nResponseSize = nResponseSize;
	return true;
Exit0:
	CleanResponse();
	return false;
}

bool CmdProcessor::ValidateLog(int nLogID) {
	// Read log path string from database, and check whether the path is accessible or not.
	TCHAR* pFilename = NULL;
	RETURN_ON_FAIL(DBAccessLogic::GetLogFilenameByLogID(nLogID, &pFilename, m_pDBConn));
	
	bool res = (TRUE == ::PathFileExists(pFilename));

	DBAccessLogic::FreeLogFilenameString(&pFilename);	
	return res;
}

bool CmdProcessor::RequestForLog(int nLogID) {
	bool bSuccess = false;
	// Read log path string from database, and open the log file
	RETURN_ON_FAIL(DBAccessLogic::GetLogFilenameByLogID(nLogID, &m_szFilename, m_pDBConn));
	PROCESS_ERROR(TRUE == PathFileExists(m_szFilename));
	TCHAR* pXmlString = NULL;
	PROCESS_ERROR(DBAccessLogic::GetLogDefXMLByLogID(nLogID, &pXmlString, m_pDBConn));
	PROCESS_ERROR(NULL != (m_pRecDef = RecordDef::ParseDefFromXML(pXmlString)));

	PROCESS_ERROR(0 == _tfopen_s(&m_pFile, m_szFilename, _T("rb")));
	
	PROCESS_ERROR(CheckAndSkipBOM(&m_nBOM));
	// Check for every case in which the default wchar_t RecordDef object has to be converted into ANSI version.
	if (m_nBOM == NO_BOM || m_nBOM == UTF8_BOM) {
		PROCESS_ERROR(m_pRecDef->ToAnsiDef(&m_pAnsiRecDef, CodePage::GetCodePageID(m_pRecDef->GetEncoding())));
		PROCESS_ERROR(PrepareCheckers(true));
	} else {	// No convert
		PROCESS_ERROR(PrepareCheckers(false));
	}

	bSuccess = true;
Exit0:
	if (!bSuccess) {
		DBAccessLogic::FreeLogFilenameString(&m_szFilename);
		CloseCFile();
		DELETE_POINTER(m_pRecDef);
		DELETE_POINTER(m_pAnsiRecDef);
		CleanCheckers();
	}
	DBAccessLogic::FreeLogDefXML(&pXmlString);
	return true;
}

bool CmdProcessor::HasMoreRecords() {
	// whether the log file end was met.
	RETURN_ON_FAIL(m_pFile != NULL);
	return (0 == feof(m_pFile));	// file end is not met yet.
}

bool CmdProcessor::LoadRecord(int nCount) {
	RETURN_ON_FAIL(m_pFile != NULL);
	// Because there's only 1 byte for holding the parameter count value. The max is 255. But in the
	// ReponseBuffer to LoadRecord command, the parameter count means how many field count there are
	// in all 
	int nFieldCount = m_pRecDef->GetFieldCount();
	const int MAX_PARAM_CAPACITY = (int)((BYTE)(0xFF));
	nCount = ((nCount * nFieldCount > MAX_PARAM_CAPACITY) ? MAX_PARAM_CAPACITY / nFieldCount : nCount);

	// Check for encoding.
	// If the file has BOM, then use the BOM as encoding, otherwise, use the user defined encoding.
	if (m_nBOM == UTF16LE_BOM) {
		return ReadRecordUTF16LE(nCount);
	} else if (m_nBOM == UTF16BE_BOM) {
		return ReadRecordUTF16BE(nCount);
	} else if (m_nBOM == UTF32LE_BOM) {
		return ReadRecordUTF32LE(nCount);
	} else if (m_nBOM == UTF32BE_BOM) {
		return ReadRecordUTF32BE(nCount);
	} else {	// Variable encoding uses ansi ver, too
		// check encoding
		// currently encoding is not supported in LogDefinitionXML yet, just take it as ansi
		return ReadRecordANSI(nCount);
	}
	
	return true;
}

bool CmdProcessor::CloseRequest() {
	RETURN_ON_FAIL(m_pFile != NULL);
	// close the file & reset filename
	CloseCFile();
	if (m_szFilename != NULL) {
		DBAccessLogic::FreeLogFilenameString(&m_szFilename);
	}
	m_nBOM = 0;
	DELETE_POINTER(m_pRecDef);
	DELETE_POINTER(m_pAnsiRecDef);
	CleanCheckers();
	return true;
}

bool CmdProcessor::CheckAndSkipBOM(int* pBom) {
	RETURN_ON_FAIL(m_pFile != NULL && pBom != NULL);
	// Here plays a trick on the BOM: Although it's not a readable string, we still use char[] to store
	// it and use the "" initialize it easily.
	char buffer[4] = "";
	RETURN_ON_FAIL(0 != fread_s(buffer, sizeof(buffer), sizeof(buffer), 1, m_pFile));

	if (buffer[0] == 0xEE && buffer[1] == 0xBB && buffer[2] == 0xBF) {	// UTF-8
		RETURN_ON_FAIL(0 == fseek(m_pFile, 3, SEEK_SET));
		*pBom = UTF8_BOM;
		return true;
	}
	if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xFE && buffer[3] == 0xFF) {	// UTF-32 BE //?
		RETURN_ON_FAIL(0 == fseek(m_pFile, 4, SEEK_SET));
		*pBom = UTF32BE_BOM;
		return true;
	}
	if (buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0x00 && buffer[3] == 0x00) {	// UTF-32 LE	//?
		RETURN_ON_FAIL(0 == fseek(m_pFile, 4, SEEK_SET));
		*pBom = UTF32LE_BOM;
		return true;
	}
	if (buffer[0] == 0xFE && buffer[1] == 0xFF) {	// UTF-16 BE //?
		RETURN_ON_FAIL(0 == fseek(m_pFile, 2, SEEK_SET));
		*pBom = UTF16BE_BOM;
		return true;
	}
	if (buffer[0] == 0xFF && buffer[1] == 0xFE) {	// UTF-16 LE BOM
		RETURN_ON_FAIL(0 == fseek(m_pFile, 2, SEEK_SET));
		*pBom = UTF16LE_BOM;
		return true;
	}

	// No Bom
	RETURN_ON_FAIL(0 == fseek(m_pFile, 0, SEEK_SET));
	*pBom = NO_BOM;

	return true;
}

bool CmdProcessor::ReverseEndian(void* pStart, int len) {
	int middle = len / 2;
	for (int byte_id = 0; byte_id < middle; ++byte_id) {
		// for the left half bytes, find its corresponding byte in the right half to swap;
		int another_byte_id = len - 1 - byte_id;
		char* p1 = (char*)pStart + byte_id;
		char* p2 = (char*)pStart + another_byte_id;
		(*p1) = (*p1) ^ (*p2);
		(*p2) = (*p1) ^ (*p2);
		(*p1) = (*p1) ^ (*p2);
	}

	return true;
}

bool CmdProcessor::PrepareCheckers(bool ansi_ver) {
	CleanCheckers();
	FixedLengthCompareA* pFlcA = NULL;
	FixedLengthCompareW* pFlcW = NULL;
	if (ansi_ver) {
		RETURN_ON_FAIL(m_pAnsiRecDef != NULL);
		PROCESS_ERROR(NULL != (m_pAnsiRecStartChecker = new FixedLengthCompareA));
		PROCESS_ERROR(NULL != (m_pAnsiRecEndChecker = new FixedLengthCompareA));
		PROCESS_ERROR(NULL != (m_pAnsiFieldStartCheckers = new ANSIFLCVECT));
		PROCESS_ERROR(NULL != (m_pAnsiFieldEndCheckers = new ANSIFLCVECT));
		PROCESS_ERROR(m_pAnsiRecStartChecker->Init(m_pAnsiRecDef->GetStartCb(), m_pAnsiRecDef->GetStartSymbol()));
		PROCESS_ERROR(m_pAnsiRecEndChecker->Init(m_pAnsiRecDef->GetEndCb(), m_pAnsiRecDef->GetEndSymbol()));
		for (int nFieldDefID = 0; nFieldDefID < m_pAnsiRecDef->GetFieldCount(); ++nFieldDefID) {
			const IFieldDefAnsi* pFieldDef = m_pAnsiRecDef->GetFieldDefAt(nFieldDefID);
			PROCESS_ERROR(NULL != pFieldDef);
			PROCESS_ERROR(NULL != (pFlcA = new FixedLengthCompareA));
			PROCESS_ERROR(NULL != (pFlcA->Init(pFieldDef->GetStartCb(), pFieldDef->GetStartSymbol())));
			m_pAnsiFieldStartCheckers->push_back(pFlcA);
			PROCESS_ERROR(NULL != (pFlcA = new FixedLengthCompareA));
			PROCESS_ERROR(NULL != (pFlcA->Init(pFieldDef->GetEndCb(), pFieldDef->GetEndSymbol())));
			m_pAnsiFieldEndCheckers->push_back(pFlcA);		
		}
	} else {
		RETURN_ON_FAIL(m_pRecDef != NULL);
		PROCESS_ERROR(NULL != (m_pWideRecStartChecker = new FixedLengthCompareW));
		PROCESS_ERROR(NULL != (m_pWideRecEndChecker = new FixedLengthCompareW));
		PROCESS_ERROR(NULL != (m_pWideFieldStartCheckers = new WIDEFLCVECT));
		PROCESS_ERROR(NULL != (m_pWideFieldEndCheckers = new WIDEFLCVECT));
		PROCESS_ERROR(m_pWideRecStartChecker->Init(m_pRecDef->GetStartCch(), m_pRecDef->GetStartSymbol()));
		PROCESS_ERROR(m_pWideRecEndChecker->Init(m_pRecDef->GetEndCch(), m_pRecDef->GetEndSymbol()));
		for (int nFieldDefID = 0; nFieldDefID < m_pRecDef->GetFieldCount(); ++nFieldDefID) {
			const IFieldDef* pFieldDef = m_pRecDef->GetFieldDefAt(nFieldDefID);
			PROCESS_ERROR(NULL != pFieldDef);
			PROCESS_ERROR(NULL != (pFlcW = new FixedLengthCompareW));
			PROCESS_ERROR(NULL != (pFlcW->Init(pFieldDef->GetStartCch(), pFieldDef->GetStartSymbol())));
			m_pWideFieldStartCheckers->push_back(pFlcW);
			PROCESS_ERROR(NULL != (pFlcW = new FixedLengthCompareW));
			PROCESS_ERROR(NULL != (pFlcW->Init(pFieldDef->GetEndCch(), pFieldDef->GetEndSymbol())));
			m_pWideFieldEndCheckers->push_back(pFlcW);		
		}
	}
	return true;
Exit0:
	DELETE_POINTER(pFlcA);
	DELETE_POINTER(pFlcW);
	CleanCheckers();
	return false;
}

void CmdProcessor::CleanCheckers() {
	DELETE_POINTER(m_pAnsiRecStartChecker);
	DELETE_POINTER(m_pAnsiRecEndChecker);
	DELETE_POINTER(m_pWideRecStartChecker);
	DELETE_POINTER(m_pWideRecEndChecker);

	DELETE_ANSI_CHECKERS_VECTOR(m_pAnsiFieldStartCheckers);
	DELETE_ANSI_CHECKERS_VECTOR(m_pAnsiFieldEndCheckers);

	DELETE_WIDE_CHECKERS_VECTOR(m_pWideFieldStartCheckers);
	DELETE_WIDE_CHECKERS_VECTOR(m_pWideFieldEndCheckers);
}

bool CmdProcessor::ReadRecordUTF16LE(int nCount) {
	
	return true;
}

bool CmdProcessor::ReadRecordUTF16BE(int nCount) {
	return true;
}

bool CmdProcessor::ReadRecordUTF32LE(int nCount) {
	return true;
}

bool CmdProcessor::ReadRecordUTF32BE(int nCount) {
	return true;
}

bool CmdProcessor::ReadRecordANSI(int nCount) {
	bool bSuccess = false;
	RECORD_TEMP_LIST record_list;
	std::string strBuffer;
	// use a buffer because std::string can accept only an array of char rather than a single char
	char aBufferAppend[2] = { 0x00, 0x00 };
	BaseUtils::_Record_Ptr pRec = NULL;

	while (nCount--) {
		PROCESS_ERROR(NULL != (pRec = new BaseUtils::Record) && pRec->Init());
		
		// check for record start
		if (!UseRecordStartChecker(m_pAnsiRecStartChecker))
			break;

		// check field start and end symbols in the log file for each field in a record
		for (int field_id = 0; field_id < m_pAnsiRecDef->GetFieldCount(); ++field_id) {
			const IFieldDefAnsi* pFieldDef = m_pAnsiRecDef->GetFieldDefAt(field_id);
			FixedLengthCompareA* pFlcStart = m_pAnsiFieldStartCheckers->at(field_id);
			FixedLengthCompareA* pFlcEnd = m_pAnsiFieldEndCheckers->at(field_id);
			// check field start
			if (!UseFieldStartChecker(pFieldDef, pFlcStart))
				break;

			// here comes with field content, but it must be checked with field end, too
			strBuffer.clear();
			while (!feof(m_pFile)) {
				aBufferAppend[0] = fgetc(m_pFile);
				strBuffer.append(aBufferAppend);
				if (pFlcEnd->AppendAndTest(aBufferAppend[0])) {
					// remove the last several characters in string buffer because they consist of Field End Symbol
					strBuffer.erase(strBuffer.size() - pFieldDef->GetEndCb(), pFieldDef->GetEndCb());
					break;
				}
			}

			// produce the record with the string
			BaseUtils::Field* pField = GetFieldByString(pFieldDef, strBuffer.c_str());
			PROCESS_ERROR(pField != NULL);
			pRec->AddField(pField);	// field on heap is added to the Record class instance, and will be removed
									// when the record object is deleted at the end of this function.
		}

		// check for record end
		if (!UseRecordEndChecker(m_pAnsiRecEndChecker))
			break;

		// a record is inserted to list only when it is fully read out
		record_list.push_back(pRec);
		pRec = NULL;
	}	// while (nCount--)

	// the record is finished, build the binary buffer to be sent to the client then.
	PROCESS_ERROR(BuildResponseWithRecords(&record_list));

	bSuccess = true;
Exit0:
	DELETE_POINTER(pRec);
	RELEASE_STD_CONTAINER_CONTENT(RECORD_TEMP_LIST, &record_list);
	return bSuccess;
}


// UseRecordStartChecker
// Return:
//		true if the start checker is found and skipped, and following is the field string,
//		false if the file end is met or the parameters cannot be found.
bool CmdProcessor::UseRecordStartChecker(FixedLengthCompareA* pFlc) {
	RETURN_ON_FAIL(m_pFile != NULL && pFlc != NULL);
	char ch = 0x00;
	pFlc->Reset();
	if (m_pAnsiRecDef->GetStartCb() > 0) {
		while (!feof(m_pFile)) {
			ch = fgetc(m_pFile);
			if (pFlc->AppendAndTest(ch))
				break;
		}
		if (feof(m_pFile))
			return false;
	}
	return true;
}

bool CmdProcessor::UseRecordStartChecker(FixedLengthCompareW* pFlc) {
	return true;
}

bool CmdProcessor::UseRecordEndChecker(FixedLengthCompareA* pFlc) {
	RETURN_ON_FAIL(m_pFile != NULL && pFlc != NULL);
	char ch = 0x00;
	pFlc->Reset();
	if (m_pAnsiRecDef->GetEndCb() > 0) {
		while (!feof(m_pFile)) {
			ch = fgetc(m_pFile);
			if (pFlc->AppendAndTest(ch))
				break;
		}
		if (feof(m_pFile))
			return false;
	}
	return true;
}

bool CmdProcessor::UseRecordEndChecker(FixedLengthCompareW* pFlc) {
	return true;
}
bool CmdProcessor::UseFieldStartChecker(const IFieldDefAnsi* pFieldDef, FixedLengthCompareA* pFlcStart) {
	RETURN_ON_FAIL(m_pFile != NULL && pFieldDef != NULL && pFlcStart != NULL);
	char ch = 0x00;
	if (pFieldDef->GetStartCb() > 0) {
		while (!feof(m_pFile)) {
			ch = fgetc(m_pFile);
			if (pFlcStart->AppendAndTest(ch))
				break;
		}
		if (feof(m_pFile))
			return false;
	}
	return true;
}
bool CmdProcessor::UseFieldStartChecker(const IFieldDef*, FixedLengthCompareW*) {
	return true;
}

BaseUtils::Field* CmdProcessor::GetFieldByString(const IFieldDefAnsi* pFieldDef, const char* szValue) {
	using namespace BaseUtils;
	BaseUtils::Field::FieldType type = pFieldDef->GetFieldType();
	BaseUtils::Field* pField = NULL;
	TCHAR* wide_value = NULL;
	if (type == BaseUtils::Field::integer) {
		int value = atoi(szValue);
		PROCESS_ERROR(errno != EINVAL && errno != ERANGE);
		pField = new IntField(value);			
	} else if (type == BaseUtils::Field::real) {
		Real value = atof(szValue);
		PROCESS_ERROR(errno != EINVAL && errno != ERANGE);
		pField = new RealField(value);
	} else if (type == BaseUtils::Field::string) {
		int nCodePageID = CodePage::GetCodePageID(m_pRecDef->GetEncoding());
		int len = MultiByteToWideChar(nCodePageID, NULL, szValue, -1, NULL, 0);
		// To avoid using string copy in constructor of StringField class, allocate a new buffer and assign
		// it to the Field object. The constructor will directly use that TCHAR string rather than make a
		// deep copy. So the wide_value shall not be released when this function is successfully completed.
		PROCESS_ERROR(NULL != (wide_value = new TCHAR[len]));
		PROCESS_ERROR(len == MultiByteToWideChar(nCodePageID, NULL, szValue, -1, wide_value, len));
		pField = new StringField(wide_value);
	} else if (type == BaseUtils::Field::timestamp) {
		BaseUtils::Timestamp time_value;
		int nCodePageID = CodePage::GetCodePageID(m_pRecDef->GetEncoding());
		int len = MultiByteToWideChar(nCodePageID, NULL, szValue, -1, NULL, 0);
		PROCESS_ERROR(NULL != (wide_value = new TCHAR[len]));
		PROCESS_ERROR(len == MultiByteToWideChar(nCodePageID, NULL, szValue, -1, wide_value, len));
		PROCESS_ERROR(Timestamp::ParseTimestampFromString(wide_value, &time_value));
		pField = new TimestampField(time_value);
		DELETE_ARRAY_POINTER(wide_value);
	}
	return pField;
Exit0:
	DELETE_ARRAY_POINTER(wide_value);
	return NULL;
}

BaseUtils::Field* CmdProcessor::GetFieldByString(const IFieldDef* pFieldDef, const TCHAR* szValue) {
	using namespace BaseUtils;
	BaseUtils::Field::FieldType type = pFieldDef->GetFieldType();
	BaseUtils::Field* pField = NULL;
	if (type == BaseUtils::Field::integer) {
		int value = _ttoi(szValue);
		PROCESS_ERROR(errno != EINVAL && errno != ERANGE);
		pField = new IntField(value);			
	} else if (type == BaseUtils::Field::real) {
		Real value = _tstof(szValue);
		PROCESS_ERROR(errno != EINVAL && errno != ERANGE);
		pField = new RealField(value);
	} else if (type == BaseUtils::Field::string) {
		TCHAR* copied_value = NULL;
		int len = _tcslen(szValue) + 1;
		PROCESS_ERROR(NULL != (copied_value = new TCHAR[len]));
		if (0 != _tcscpy_s(copied_value, len, szValue)) {
			DELETE_ARRAY_POINTER(copied_value);
		} else
			pField = new StringField(copied_value);
	} else if (type == BaseUtils::Field::timestamp) {
		BaseUtils::Timestamp time_value;
		PROCESS_ERROR(Timestamp::ParseTimestampFromString(szValue, &time_value));
		pField = new TimestampField(time_value);
	}
Exit0:
	return pField;
}

bool CmdProcessor::BuildResponseWithRecords(RECORD_TEMP_LIST_PTR pList) {
	CleanResponse();
	int __len = pList->size();
	RETURN_ON_FAIL(CalculateResponseSize(pList));
	RETURN_ON_FAIL(NULL != (m_pResponseBuf = new char[m_nResponseSize]));
	// All records must have a fixed number of fields.
	PROCESS_ERROR(SetResponseHeaderWithRecords(pList->size() * m_pRecDef->GetFieldCount()));
	int nFieldCount = m_pRecDef->GetFieldCount();
	int offset = FLAG_VERSION_OFFSET + CMD_HEADER_SIZE;

	int nRecID = 0;
	for (RECORD_TEMP_ITER iter = pList->begin(); iter != pList->end(); ++iter) {	// for each record
		for (int nFieldID = 0; nFieldID < nFieldCount; ++nFieldID) {	// for each field in a record
			BaseUtils::_Field_Ptr pField = (*iter)->GetField(nFieldID);
			PROCESS_ERROR(pField != NULL);
			if (pField->GetType() == BaseUtils::Field::integer) {
				PROCESS_ERROR(WriteIntFieldToBuffer(offset, pField, nRecID));
			} else if (pField->GetType() == BaseUtils::Field::real) {
				PROCESS_ERROR(WriteRealFieldToBuffer(offset, pField, nRecID));
			} else if (pField->GetType() == BaseUtils::Field::string) {
				PROCESS_ERROR(WriteStringFieldToBuffer(offset, pField, nRecID));
			} else if (pField->GetType() == BaseUtils::Field::timestamp) {
				PROCESS_ERROR(WriteTimeFieldToBuffer(offset, pField, nRecID));
			} else
				PROCESS_ERROR(false);
		}
		nRecID++;
	}


	return true;
Exit0:
	CleanResponse();
	return false;
}

bool CmdProcessor::CalculateResponseSize(RECORD_TEMP_LIST_PTR pList) {
	// check whole size
	int nResponseSize = CMD_HEADER_SIZE;
	int nFieldCount = m_pRecDef->GetFieldCount();
	for (RECORD_TEMP_ITER iter = pList->begin(); iter != pList->end(); ++iter) {	// for each record
		for (int nFieldID = 0; nFieldID < nFieldCount; ++nFieldID) {	// for each field in a record
			nResponseSize += FIELD_SIZE_LEN + FIELD_RECORD_ID_LEN + FIELD_TYPE_LEN;
			BaseUtils::_Field_Ptr pField = NULL;
			pField = (*iter)->GetField(nFieldID);
			if (pField->GetType() == BaseUtils::Field::integer) {
				nResponseSize += FIELD_VALUE_SIZE_LEN + FIELD_INT_VALUE_SIZE;
			} else if (pField->GetType() == BaseUtils::Field::real) {
				nResponseSize += FIELD_VALUE_SIZE_LEN + FIELD_REAL_VALUE_SIZE;
			} else if (pField->GetType() == BaseUtils::Field::string) {
				nResponseSize += FIELD_VALUE_SIZE_LEN;
				BaseUtils::StringField* pStringField = dynamic_cast<BaseUtils::StringField*>(pField);
				RETURN_ON_FAIL(pStringField != NULL);
				int len = _tcslen(pStringField->GetValueString());
				nResponseSize += CharCountToSize(len + 1);	// the size includes null-terminator
			} else if (pField->GetType() == BaseUtils::Field::timestamp) {
				nResponseSize += FIELD_TIME_VALUE_SIZE;		// the time field doesn't need a size descriptor		
			} else
				RETURN_ON_FAIL(false);	// error
		}
	}

	m_nResponseSize = nResponseSize;
	return true;
}

bool CmdProcessor::SetResponseHeaderWithRecords(int nTotalFieldCount) {
	RETURN_ON_FAIL(m_pResponseBuf != NULL && m_nResponseSize >= CMD_HEADER_SIZE);
	RETURN_ON_FAIL(m_nCmdID == CMD_ID_LOADRECORD);
	
	m_pResponseBuf[FLAG_VERSION_OFFSET] = 1;
	m_pResponseBuf[FLAG_OPTION_OFFSET] = ByteToCharBuffer(OPTION_RTN);
	m_pResponseBuf[FLAG_CMDID_OFFSET] = ByteToCharBuffer(CMD_ID_LOADRECORD);
	m_pResponseBuf[FLAG_PARAM_OFFSET] = ByteToCharBuffer(nTotalFieldCount);
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + FLAG_CMD_SIZE_OFFSET, FLAG_CMD_SIZE_LEN, &m_nResponseSize, sizeof(m_nResponseSize)));
	
	return true;
}

bool CmdProcessor::WriteIntFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID) {
	// Field Size
	int nFieldSize = FIELD_SIZE_LEN + FIELD_RECORD_ID_LEN + FIELD_TYPE_LEN + FIELD_VALUE_SIZE_LEN + FIELD_INT_VALUE_SIZE;
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_SIZE_LEN, &nFieldSize, sizeof(nFieldSize)));
	offset += FIELD_SIZE_LEN;
	// Record ID
	m_pResponseBuf[offset] = ByteToCharBuffer(nRecID);
	offset += FIELD_RECORD_ID_LEN;
	// Field Type				
	m_pResponseBuf[offset] = ByteToCharBuffer(FIELD_TYPE_INTEGER);
	offset += FIELD_TYPE_LEN;
	// Field Value size
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_VALUE_SIZE_LEN, &FIELD_INT_VALUE_SIZE, sizeof(FIELD_INT_VALUE_SIZE)));
	offset += FIELD_VALUE_SIZE_LEN;
	// Field Value
	BaseUtils::IntField* pIntField = dynamic_cast<BaseUtils::IntField*>(pField);
	RETURN_ON_FAIL(pIntField != NULL);
	int nValue = pIntField->GetValueInt();
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_INT_VALUE_SIZE, &nValue, sizeof(nValue)));
	offset += FIELD_INT_VALUE_SIZE;
	return true;
}

bool CmdProcessor::WriteRealFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID) {
	// Field Size
	int nFieldSize = FIELD_SIZE_LEN + FIELD_RECORD_ID_LEN + FIELD_TYPE_LEN + FIELD_VALUE_SIZE_LEN + FIELD_REAL_VALUE_SIZE;
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_SIZE_LEN, &nFieldSize, sizeof(nFieldSize)));
	offset += FIELD_SIZE_LEN;
	// Record ID
	m_pResponseBuf[offset] = ByteToCharBuffer(nRecID);
	offset += FIELD_RECORD_ID_LEN;
	// Field Type				
	m_pResponseBuf[offset] = ByteToCharBuffer(FIELD_TYPE_REAL);
	offset += FIELD_TYPE_LEN;
	// Field Value size
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_VALUE_SIZE_LEN, &FIELD_REAL_VALUE_SIZE, sizeof(FIELD_REAL_VALUE_SIZE)));
	offset += FIELD_VALUE_SIZE_LEN;
	// Field Value
	BaseUtils::RealField* pRealField = dynamic_cast<BaseUtils::RealField*>(pField);
	RETURN_ON_FAIL(pRealField != NULL);
	BaseUtils::Real rValue = pRealField->GetValueReal();
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_REAL_VALUE_SIZE, &rValue, sizeof(rValue)));
	offset += FIELD_INT_VALUE_SIZE;
	return true;
}

bool CmdProcessor::WriteStringFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID) {
	BaseUtils::StringField* pStringField = dynamic_cast<BaseUtils::StringField*>(pField);
	RETURN_ON_FAIL(pStringField != NULL);
	int nCharCount = _tcslen(pStringField->GetValueString()) + 1;	// size value includes the null-terminator
	// Field Size
	int nFieldSize = FIELD_SIZE_LEN + FIELD_RECORD_ID_LEN + FIELD_TYPE_LEN;
	nFieldSize += FIELD_VALUE_SIZE_LEN + CharCountToSize(nCharCount);
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_SIZE_LEN, &nFieldSize, sizeof(nFieldSize)));
	offset += FIELD_SIZE_LEN;
	// Record ID
	m_pResponseBuf[offset] = ByteToCharBuffer(nRecID);
	offset += FIELD_RECORD_ID_LEN;
	// Field Type				
	m_pResponseBuf[offset] = ByteToCharBuffer(FIELD_TYPE_STRING);
	offset += FIELD_TYPE_LEN;
	// Field Value size
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_VALUE_SIZE_LEN, &nCharCount, sizeof(nCharCount)));
	offset += FIELD_VALUE_SIZE_LEN;
	// Field Value
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, CharCountToSize(nCharCount), pStringField->GetValueString(),
		CharCountToSize(nCharCount)));
	offset += CharCountToSize(nCharCount);
	return true;
}

bool CmdProcessor::WriteTimeFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID) {
	BaseUtils::TimestampField* pTimeField = dynamic_cast<BaseUtils::TimestampField*>(pField);
	RETURN_ON_FAIL(pTimeField != NULL);
	// Field Size
	int nFieldSize = FIELD_SIZE_LEN + FIELD_RECORD_ID_LEN + FIELD_TYPE_LEN + FIELD_TIME_VALUE_SIZE;
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_SIZE_LEN, &nFieldSize, sizeof(nFieldSize)));
	offset += FIELD_SIZE_LEN;
	// Record ID
	m_pResponseBuf[offset] = ByteToCharBuffer(nRecID);
	offset += FIELD_RECORD_ID_LEN;
	// Field Type				
	m_pResponseBuf[offset] = ByteToCharBuffer(FIELD_TYPE_TIMESTAMP);
	offset += FIELD_TYPE_LEN;

	// Timestamp Field doesn't need a field value size descriptor in the buffer to send.
	const BaseUtils::Timestamp* pTimeValue = pTimeField->GetPointerTime();
	// Any field of time to be sent from LogServer to ProcessCore will always be of the Timestamp class type.
	// It's enough to use 2bytes for year, 1byte each for the month, day, hour, minute & second.
	// TimeRange class in BaseUtils namespace will never be sent by this function & this definition of TCP
	// packet.
	// Year
	short year = pTimeValue->GetYear();
	RETURN_ON_FAIL(0 == memcpy_s(m_pResponseBuf + offset, FIELD_TIME_YEAR_LEN, &year, sizeof(year)));
	offset += FIELD_TIME_YEAR_LEN;
	// Month
	m_pResponseBuf[offset] = ByteToCharBuffer(pTimeValue->GetMonth());
	offset += FIELD_TIME_MONTH_LEN;
	// Day
	m_pResponseBuf[offset] = ByteToCharBuffer(pTimeValue->GetDay());
	offset += FIELD_TIME_DAY_LEN;
	// Hour
	m_pResponseBuf[offset] = ByteToCharBuffer(pTimeValue->GetHour());
	offset += FIELD_TIME_HOUR_LEN;
	// Minute
	m_pResponseBuf[offset] = ByteToCharBuffer(pTimeValue->GetMinute());
	offset += FIELD_TIME_MINUTE_LEN;
	// Second
	m_pResponseBuf[offset] = ByteToCharBuffer(pTimeValue->GetSecond());
	offset += FIELD_TIME_SECOND_LEN;	

	return true;
}
