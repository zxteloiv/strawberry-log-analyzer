// DBAccess.h
// contains classes for data access layer
//

#pragma once

namespace Toolkit {
	struct ADOContext;

	class ADOConnector {
		// ==========================================
		// Interfaces

		// Constructors and Destructors
	public:
		ADOConnector();
		virtual ~ADOConnector();

		// Initializer: act as a two-phase initialization
		// Return:	true if successful, otherwise false is returned.
		bool Init();

		// Public interfaces
	public:
		// ExecuteNonQuery : used to execute sql commands other than select clause
		// Param:	SQL command
		// Return:	The number of rows affected
		int ExecuteNonQuery(const TCHAR*);	// SQL insert, update, delete

		// ExecuteQuery: execute sql select clause
		// Param:	SQL command
		// Return:	The table pointer for select clause
		ADODB::_RecordsetPtr	ExecuteQuery(const TCHAR*);		// SQL select

		// Open Connection
		bool Open();	// a UDL is used instead of the connection string

		// Close Connection
		bool Close();

		ADODB::_ConnectionPtr GetConnPtr() { return m_pConn; }

		// ==========================================
		// Data members
	protected:
		ADODB::_ConnectionPtr	m_pConn;
		ADODB::_RecordsetPtr	m_pRecordSet;
	};
};
