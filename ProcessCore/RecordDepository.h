// RecordDepository.h
// Declaration for RecordDespository class.
// As the name suggested, RecordDepository class is where we store records buffer fetched
// from the Log Server.
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"
#include <queue>

#include "CoreDeclaration.h"

namespace ProcessCore {
	class RecordDepository {
		// ===========================================================
		// Interfaces

		// Constructors and destructors
	public:
		RecordDepository(PRINT_FUNC pOutFunc = NULL);
		virtual ~RecordDepository();

		// Public interfaces
	public:
		bool Init(int log_id);

		BaseUtils::_Record_Ptr	GetARecord();
		void					ReleaseRecord(BaseUtils::_Record_Ptr);

		bool IsLogAlive();			// check for the log
		bool StartDepository();		// prepare the depository to fetch records from remote server
		bool HasAnyMoreRecords();	// true if the log has met the end
		bool ShutdownDepository();	// close the depository for this turn

		// Protectd interfaces
	protected:
		bool ReadRecords(int amount);	// read an amount of records to depository
		bool ConnectSocket();
		void CloseSocket();	// disconnect from the log server

		bool CheckBoolResponse(int cmd_id);

		bool AnalyzeFieldMessage(int nParamCount);
		BaseUtils::_Field_Ptr GetNextIntFieldFromTCPBuffer(int& offset, int& inner_offset);
		BaseUtils::_Field_Ptr GetNextRealFieldFromTCPBuffer(int& offset, int& inner_offset);
		BaseUtils::_Field_Ptr GetNextStringFieldFromTCPBuffer(int& offset, int& inner_offset);
		BaseUtils::_Field_Ptr GetNextTimestampFieldFromTCPBuffer(int& offset, int& inner_offset);

		void WriteLog(const TCHAR* str) { if (m_pPrintLogFunc != NULL) m_pPrintLogFunc(str); }

		// ===========================================================
		// Data members
	protected:
		int				m_nLogID;
		LPRECQUEUE		m_pDepoQueue;	// the depository container

		HANDLE			m_hReadingThread;
		TcpTranslator*	m_pTcpTranslator;
		TcpClientBuf*	m_pSendBuf;
		TcpClientBuf*	m_pRecvBuf;
		SOCKET			m_clientSocket;

		PRINT_FUNC		m_pPrintLogFunc;

		// ===========================================================
		// Static members
	public:
		// const members
		static const TCHAR* s_csConfSectionDepo;	// the section name containing configurations for this class
		static const TCHAR* s_csConfItemLogSvrName;
		static const TCHAR* s_csConfItemLogSvrPort;
		static const TCHAR* s_csConfItemDepoSize;
		static const int	s_cnMaxDestNameSize = 260;	// domain name length
		static const int	s_cnMaxDestPortSize = 11;	// 65535 is the max port number

		// non-const
		static int			s_nMaxQueueSize;
		static WSADATA		s_wsaData;
		static ADDRINFOT*	s_pAddrInfo;
		static TCHAR		s_szDestName[s_cnMaxDestNameSize];
		static TCHAR		s_szDestPort[s_cnMaxDestPortSize];

		// ===========================================================
		// Static functions
	protected:
		static bool InitTcpClient();
		static bool LoadConfig();
	
	public:
		static bool InitDepository();
		static bool Cleanup();

		static unsigned int ReadingThread(void*);

	};
};
