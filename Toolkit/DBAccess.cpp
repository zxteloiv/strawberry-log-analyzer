// DBAccess.cpp
// implements the connection class for database
//

#include "stdafx.h"
#include "dbaccess.h"

using namespace Toolkit;

ADOConnector::ADOConnector() : m_pConn(NULL), m_pRecordSet(NULL) {
	::CoInitialize(NULL);
}

ADOConnector::~ADOConnector() {
	if (m_pRecordSet != NULL) {
		if (m_pRecordSet->State == ADODB::adStateOpen)
			m_pRecordSet->Close();
	}
	if (m_pConn != NULL) {
		if (m_pConn->State == ADODB::adStateOpen)
			m_pConn->Close();
	}

	::CoUninitialize();
}

bool ADOConnector::Init() {
	m_pConn.CreateInstance(__uuidof(ADODB::Connection));
	m_pRecordSet.CreateInstance(__uuidof(ADODB::Recordset));

	RETURN_ON_FAIL(m_pConn != NULL && m_pRecordSet != NULL);

	return true;
}

bool ADOConnector::Open() {
	RETURN_ON_FAIL(m_pConn != NULL);
	try {
		HRESULT hr = m_pConn->Open(_T("File Name=.\\DBConn.udl;"), _T(""), _T(""), ADODB::adModeUnknown);
	} catch (_com_error&) {
		return false;
	}
	return true;
}

bool ADOConnector::Close() {
	try {
		if (m_pRecordSet != NULL) {
			if (m_pRecordSet->State == ADODB::adStateOpen)
				m_pRecordSet->Close();
		}
		if (m_pConn != NULL) {
			if (m_pConn->State == ADODB::adStateOpen)
				m_pConn->Close();
		}
	} catch (...) {
		return false;
	}
	return true;
}

int ADOConnector::ExecuteNonQuery(const TCHAR* szwSQLCmd) {
	RETURN_ON_FAIL(m_pConn != NULL && szwSQLCmd != NULL);
	variant_t rtn_var = -1;
	try {
		m_pConn->Execute(_bstr_t(szwSQLCmd), &rtn_var, ADODB::adCmdText);
		return true;
	} catch (_com_error&) {
		return rtn_var;
	}
	return rtn_var;
}

ADODB::_RecordsetPtr ADOConnector::ExecuteQuery(const TCHAR* szwSQLCmd) {
	RETURN_ON_FAIL(m_pRecordSet != NULL && m_pConn != NULL);
	try {
		if (m_pRecordSet->State == ADODB::adStateOpen) {
			m_pRecordSet->Close();
		}
		m_pRecordSet->Open(_bstr_t(szwSQLCmd),
			m_pConn.GetInterfacePtr(),
			ADODB::adOpenDynamic,
			ADODB::adLockOptimistic,
			ADODB::adCmdText
			);
	} catch (_com_error&) {
		return NULL;
	}
	return m_pRecordSet;
}

