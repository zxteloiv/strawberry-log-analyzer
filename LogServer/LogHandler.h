// LogHandler.h
// Definition for LogHandler class which is responsible to read log file,
// extract each record and send it to the RecordDepository class.
//

#pragma once

#include "LogServerDef.h"

namespace LogServer {
	class LogHandler {
		// =======================================================
		// Static funtions

		// Thread functions
	public:
		static unsigned int WINAPI DoListeningStuff(PVOID pContext);

		static VOID NTAPI HandleConnSimple(
			PTP_CALLBACK_INSTANCE	pInst,
			PVOID					pContext
			);

		// Public Interfaces
	public:
		// Set the environment variables
		static void Startup();
		// Initilize some of the variables
		static bool Init();

		static bool ListenAsync();
		static void CleanUpHandler();

		static bool IsListening() { return s_bIsListening; }
		static void StopListening();

		// Help functions
	protected:
		static bool LoadConfig();
		static bool ResolveAddrInfo();
		static bool CreateSocket();
		static bool BindTcpSocket();
		static bool ListenOnSocket();
		static void CleanUpWinsock();

		// =======================================================
		// Static members

		// Const variables
	public:
		static const TCHAR*	s_szConfigFilePath;
		static const TCHAR*	s_szSection;
		static const TCHAR*	s_szPortKey;
		static const TCHAR*	s_szMaxConnKey;
		static const int MAX_PORT_STRING_LEN = 10;	// A port number is less than 65535, therefore 10 is enough.

		// non-const
	protected:
		// for winsock initialization
		static WSADATA		s_wsaData;
		static ADDRINFOT*	s_pAddrInfo;
		static TCHAR		s_szPort[MAX_PORT_STRING_LEN];
		static SOCKET		s_listenSocket;
		
		// properties for listening
		static bool		s_bIsListening;

		// LogHandler implementation details
		static HANDLE		s_hListenThread;

		static int			s_nMaxConnection;	// current not used

	};
};
