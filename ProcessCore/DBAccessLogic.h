// DBAccessLogic.h
// Declare C functions for reading & writing database without directly access towards returned recordset.
// Wrapper the SQL statement for SELECT, INSERT, UPDATE & DELETE clauses for all usage in ProcessCore.
// Every object shall maintain their own Database Connector Object but shall avoid accessing Database by
// using those objects.
//

#pragma once

#include "CoreDeclaration.h"
#include "..\\Toolkit\\Toolkit.h"

namespace ProcessCore {
	class DBAccessLogic {
	public:
		static bool GetPendingRequestID(int pending_count, int* pContainer, Toolkit::ADOConnector* pDBConn, int* pItemGot);
		static bool GetLogIDByRequestID(int request_id, int* pLogID, Toolkit::ADOConnector* pDBConn);
		static bool GetRequestXMLByRequestID(int request_id, TCHAR** ppXMLString, Toolkit::ADOConnector* pDBConn);
		static void FreeRequestXML(TCHAR* pXMLString);

		static bool UpdateRequestToPreparing(int request_id, Toolkit::ADOConnector* pDBConn);
		static bool UpdateRequestToProcessing(int request_id, Toolkit::ADOConnector* pDBConn);
		static bool UpdateRequestToSuccess(int request_id, Toolkit::ADOConnector* pDBConn);
		static bool UpdateRequestToFailed(int request_id, Toolkit::ADOConnector* pDBConn);

		static bool SaveResultForRequest(int request_id, int log_id, TCHAR* pResultXML, Toolkit::ADOConnector* pDBConn);


		// variables
	public:
		static TCHAR* STR_DBNULL;
	};
};
