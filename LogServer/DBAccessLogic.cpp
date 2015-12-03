// DBAccessLogic.cpp
// Implementations for DBAccessLogic class.
//

#include "stdafx.h"
#include "DBAccessLogic.h"

// <string> is included before "DBAccessLogic.h" to avoid warning, because DBAccessLogic.h includes Toolkit first.
// But it is amazing that getting warnings just because Toolkit.h is included first.
// it turns out that <strsafe.h> is what incompatible with <string>, that's why <string> produce warning.
// So I removed <strsafe.h> and change StringCbCopy to _tcscpy_s
#include <string>

using namespace LogServer;


bool DBAccessLogic::GetLogFilenameByLogID(int log_id, TCHAR** ppFilename, Toolkit::ADOConnector* pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL && ppFilename != NULL);
	std::wstring s(_T("select [st_log].[fileName] AS fullname from st_log where id="));
	TCHAR szLogID[11];	// a string with length 11 can hold any number ranging from INT_MAX to INT_MIN
	RETURN_ON_FAIL(0 == _itot_s(log_id, szLogID, 11, 10));	// 10 is radix
	s.append(szLogID);

	ADODB::_RecordsetPtr pRecSet = pDBConn->ExecuteQuery(s.c_str());
	RETURN_ON_FAIL(pRecSet != NULL);
	RETURN_ON_FAIL(S_OK == pRecSet->MoveFirst());
	variant_t vtFilename(pRecSet->Fields->GetItem(_T("fullname"))->Value);

	// copy the name out
	LPCTSTR pFilename = vtFilename.bstrVal;
	int len = _tcslen(pFilename) + 1;
	*ppFilename = NULL;
	RETURN_ON_FAIL(NULL != (*ppFilename = new TCHAR[len]));
	if (0 != _tcscpy_s(*ppFilename, len, pFilename)) {	// copy failed
		DELETE_ARRAY_POINTER(*ppFilename);
		pRecSet->Close();
		return false;
	}

	pRecSet->Close();
	return true;
}

void DBAccessLogic::FreeLogFilenameString(TCHAR** ppFilename) {
	DELETE_ARRAY_POINTER(*ppFilename);
}

bool DBAccessLogic::GetLogDefXMLByLogID(int log_id, TCHAR** ppXmlString, Toolkit::ADOConnector* pDBConn) {
	RETURN_ON_FAIL(pDBConn != NULL && ppXmlString != NULL);
	std::wstring s(_T("select [st_log].[defXML] from st_log where id="));
	TCHAR szLogID[11];	// a string with length 11 can hold any number ranging from INT_MAX to INT_MIN
	RETURN_ON_FAIL(0 == _itot_s(log_id, szLogID, 11, 10));	// 10 is radix
	s.append(szLogID);
	
	ADODB::_RecordsetPtr pRecSet = pDBConn->ExecuteQuery(s.c_str());
	RETURN_ON_FAIL(pRecSet != NULL);
	RETURN_ON_FAIL(S_OK == pRecSet->MoveFirst());
	variant_t vtDefXML(pRecSet->Fields->GetItem(_T("defXML"))->Value);
	
	// copy the XML string out
	LPCTSTR pXmlString = vtDefXML.bstrVal;
	int len = _tcslen(pXmlString) + 1;
	*ppXmlString = NULL;
	RETURN_ON_FAIL(NULL != (*ppXmlString = new TCHAR[len]));
	if (0 != _tcscpy_s(*ppXmlString, len, pXmlString)) {	// copy failed
		DELETE_ARRAY_POINTER(*ppXmlString);
		pRecSet->Close();
		return false;
	}

	pRecSet->Close();
	return true;
}

void DBAccessLogic::FreeLogDefXML(TCHAR** ppXmlString) {
	DELETE_ARRAY_POINTER(*ppXmlString);
}
