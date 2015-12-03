// DBAccessLogic.cpp
// Implements the global functions declared in DBAccessLogic.h
//

#include "stdafx.h"
#include "DBAccessLogic.h"
//#include <atlstr.h>
#include <string>

using namespace Toolkit;
using namespace ProcessCore;

TCHAR* DBAccessLogic::STR_DBNULL = _T("");

bool DBAccessLogic::GetPendingRequestID(int pending_count, int* pContainer, Toolkit::ADOConnector* pDBConn, int* pItemGot) {
	RETURN_ON_FAIL(pDBConn != NULL && pContainer != NULL);
	
	std::wstring s(L"select top(");
	TCHAR count_str[12];	// 11 is enough for max of unsigned int of 4294967295
	RETURN_ON_FAIL(0 == _itot_s(pending_count, count_str, 12, 10));	// 10 is Radix, 11 is array length
	s.append(count_str);
	s += L") st_request.id from st_request inner join st_requestState on st_request.stateID = st_requestState.id where st_requestState.detail = 'Pending' ";
	
	ADODB::_RecordsetPtr pRecSet = pDBConn->ExecuteQuery(s.c_str());
	RETURN_ON_FAIL(pRecSet != NULL);

	// read each record in the returned recordset
	//pRecSet->RecordCount
	try {
		pRecSet->MoveFirst();
		int rowID = 0;
		while (!pRecSet->EndOfFile && pending_count--) {
			pContainer[rowID] = pRecSet->Fields->Item["id"]->Value.intVal;
			rowID++;
			pRecSet->MoveNext();
		}

		if (pItemGot != NULL)
			*pItemGot = rowID;

		pRecSet->Close();
	} catch (...) {
		return false;
	}

	return true;
}

bool DBAccessLogic::GetLogIDByRequestID(int request_id, int* pLogID, Toolkit::ADOConnector* pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL && pLogID != NULL);
	std::wstring s(L"select logID from st_request where id = ");
	TCHAR reqID_str[12] = _T("");
	RETURN_ON_FAIL(0 == _itot_s(request_id, reqID_str, 12, 10));
	s.append(reqID_str);

	ADODB::_RecordsetPtr pRecSet = pDBConn->ExecuteQuery(s.c_str());
	RETURN_ON_FAIL(pRecSet != NULL);
	RETURN_ON_FAIL(S_OK == pRecSet->MoveFirst());

	try {
		pRecSet->MoveFirst();
		*pLogID = pRecSet->Fields->Item[_T("logID")]->Value.intVal;

		pRecSet->Close();
	} catch (...) {
		pRecSet->Close();
		return false;
	}
	
	return true;
}

bool DBAccessLogic::GetRequestXMLByRequestID(int request_id, TCHAR** ppXMLString, Toolkit::ADOConnector* pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL && ppXMLString != NULL);
	std::wstring s(L"select st_request.requestXML AS Xml from st_request where st_request.id = ");
	TCHAR reqID_str[12] = _T("");
	RETURN_ON_FAIL(0 == _itot_s(request_id, reqID_str, 12, 10));
	s.append(reqID_str);

	ADODB::_RecordsetPtr pRecSet = pDBConn->ExecuteQuery(s.c_str());
	RETURN_ON_FAIL(pRecSet != NULL);
	RETURN_ON_FAIL(S_OK == pRecSet->MoveFirst());

	try {
		pRecSet->MoveFirst();

		variant_t vtXml(pRecSet->GetFields()->GetItem(_T("Xml"))->GetValue());
		int len;
		// make a deep copy for returned xml value.
		if (vtXml.vt == VT_NULL) {
			len = _tcslen(STR_DBNULL) + 1;
			RETURN_ON_FAIL(NULL != (*ppXMLString = new TCHAR[len]));
			PROCESS_ERROR(0 == _tcscpy_s(*ppXMLString, len, STR_DBNULL));
		} else {
			len = _tcslen((PCTSTR)vtXml.bstrVal) + 1;
			RETURN_ON_FAIL(NULL != (*ppXMLString = new TCHAR[len]));
			PROCESS_ERROR(0 == _tcscpy_s(*ppXMLString, len, (PCTSTR)vtXml.bstrVal));
		}

		pRecSet->Close();
	} catch (...) {
		pRecSet->Close();
		return false;
	}
	
	return true;
Exit0:
	DELETE_POINTER(ppXMLString);
	return false;
}

void DBAccessLogic::FreeRequestXML(TCHAR* pXMLString) {
	delete pXMLString;
}

bool DBAccessLogic::UpdateRequestToPreparing(int request_id, Toolkit::ADOConnector *pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL);
	std::wstring s(L"update st_request set stateID=(select id from st_requestState where locale='en-US' and detail='Preparing') where id=");
	TCHAR reqID_str[12] = _T("");
	RETURN_ON_FAIL(0 == _itot_s(request_id, reqID_str, 12, 10));
	s.append(reqID_str);
	int nAffectedRows = pDBConn->ExecuteNonQuery(s.c_str());
	return (1 == nAffectedRows);
}

bool DBAccessLogic::UpdateRequestToProcessing(int request_id, Toolkit::ADOConnector *pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL);
	std::wstring s(L"update st_request set stateID=(select id from st_requestState where locale='en-US' and detail='Processing') where id=");
	TCHAR reqID_str[12] = _T("");
	RETURN_ON_FAIL(0 == _itot_s(request_id, reqID_str, 12, 10));
	s.append(reqID_str);
	int nAffectedRows = pDBConn->ExecuteNonQuery(s.c_str());
	return (1 == nAffectedRows);
}

bool DBAccessLogic::UpdateRequestToSuccess(int request_id, Toolkit::ADOConnector *pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL);
	std::wstring s(L"update st_request set stateID=(select id from st_requestState where locale='en-US' and detail='Success') where id=");
	TCHAR reqID_str[12] = _T("");
	RETURN_ON_FAIL(0 == _itot_s(request_id, reqID_str, 12, 10));
	s.append(reqID_str);
	int nAffectedRows = pDBConn->ExecuteNonQuery(s.c_str());
	return (1 == nAffectedRows);
}

bool DBAccessLogic::UpdateRequestToFailed(int request_id, Toolkit::ADOConnector *pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL);
	std::wstring s(L"update st_request set stateID=(select id from st_requestState where locale='en-US' and detail='Fail') where id=");
	TCHAR reqID_str[12] = _T("");
	RETURN_ON_FAIL(0 == _itot_s(request_id, reqID_str, 12, 10));
	s.append(reqID_str);
	int nAffectedRows = pDBConn->ExecuteNonQuery(s.c_str());
	return (1 == nAffectedRows);
}

bool DBAccessLogic::SaveResultForRequest(int request_id, int log_id, TCHAR* pResultXML, Toolkit::ADOConnector* pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL && pResultXML != NULL);

	// 1. Create a command object
	ADODB::_CommandPtr spInsertCmd;
	RETURN_ON_FAIL(S_OK == spInsertCmd.CreateInstance(__uuidof(ADODB::Command)));	
	// 2. Assign the connection to the command
	spInsertCmd->ActiveConnection = pDBConn->GetConnPtr();
	// 3. Set the command text
	// SQL statement or the name of the stored procedure 
	_bstr_t bstrInsertCmd(L"INSERT INTO st_result VALUES (?)");
	spInsertCmd->CommandText = bstrInsertCmd;
	// 4. Set the command type
	// ADODB::adCmdText for oridinary SQL statements; 
	// ADODB::adCmdStoredProc for stored procedures.
	spInsertCmd->CommandType = ADODB::adCmdText;
	// 5. Append the parameters
	// Append the parameter for result XML
	ADODB::_ParameterPtr spResultParam;
	spResultParam = spInsertCmd->CreateParameter(
		_bstr_t("ResultXML"),		// Parameter name
		ADODB::adVarWChar,			// Parameter type (NVarChar)
		ADODB::adParamInput,		// Parameter direction
		-1,							// Max size (ignored for XML) ? // for test
		variant_t(pResultXML));		// Parameter value
	RETURN_ON_FAIL(S_OK == spInsertCmd->Parameters->Append(spResultParam));
	// 6. Execute the command
	spInsertCmd->Execute(NULL, NULL, ADODB::adExecuteNoRecords);

	// get the identity id inserted
	try {
		// get the result id of the row just inserted, which is a identity column
		ADODB::_RecordsetPtr pRecSet = pDBConn->ExecuteQuery(L"select IDENT_CURRENT('st_result') as [the_id]");
		RETURN_ON_FAIL(pRecSet != NULL);
		RETURN_ON_FAIL(S_OK == pRecSet->MoveFirst());
		int nResultID = pRecSet->Fields->Item[_T("the_id")]->Value.intVal;

		//update st_request set resultID=1 where id=1
		std::wstring s(L"update st_request set resultID=");
		TCHAR szNumBuf[12] = L"";
		RETURN_ON_FAIL(0 == _itot_s(nResultID, szNumBuf, 12, 10));
		s.append(szNumBuf);
		s.append(L" where id=");
		RETURN_ON_FAIL(0 == _itot_s(request_id, szNumBuf, 12, 10));
		s.append(szNumBuf);
		RETURN_ON_FAIL(1 == pDBConn->ExecuteNonQuery(s.c_str()));
	} catch (...) {
		return false;
	}
	return true;
}