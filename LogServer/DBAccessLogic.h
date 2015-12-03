// DBAccessLogic.h
// Definition for the DBAccessLogic class, which contains all the static methods to access the Database.
// DBConnector object shall be maintained on those callers' own.
// Other functions in this project must not use the DBConnector objects directly to execute SQL command.
//

#pragma once

#include "..\\Toolkit\\Toolkit.h"

namespace LogServer {
	class DBAccessLogic {
	public:
		static bool GetLogFilenameByLogID(int log_id, TCHAR** ppFilename, Toolkit::ADOConnector* pDBConn);
		static void FreeLogFilenameString(TCHAR** pFilename);
		static bool GetLogDefXMLByLogID(int log_id, TCHAR** ppXmlString, Toolkit::ADOConnector* pDBConn);
		static void FreeLogDefXML(TCHAR** ppXmlString);
	};
};
