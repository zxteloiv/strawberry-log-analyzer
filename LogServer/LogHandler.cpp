// LogHandler.cpp
// Implements LogHandler class.
// Take care of the Winsock headers for socket programming.
// Functions defined here are not allowed to be referenced outside this cpp file.
// So all these definitions are not included in a header file and are directly written here.
//

#include "stdafx.h"
#include "LogHandler.h"
#include "CmdProcessor.h"
#include "TcpCommBuf.h"

#include <process.h>	// for C functions of _beginthreadex & _endthreadex

using namespace LogServer;

const TCHAR* LogHandler::s_szConfigFilePath = _T(".\\logsvrconf.ini");
const TCHAR* LogHandler::s_szSection = _T("LogServer");
const TCHAR* LogHandler::s_szPortKey = _T("listen_port");
const TCHAR* LogHandler::s_szMaxConnKey = _T("max_connection");

WSADATA				LogHandler::s_wsaData;
SOCKET				LogHandler::s_listenSocket = INVALID_SOCKET;
ADDRINFOT*			LogHandler::s_pAddrInfo = NULL;
bool				LogHandler::s_bIsListening = false;
HANDLE				LogHandler::s_hListenThread = INVALID_HANDLE_VALUE;
int					LogHandler::s_nMaxConnection;
TCHAR				LogHandler::s_szPort[MAX_PORT_STRING_LEN] = _T("");

// ==========================================================
// Static function implementations

void LogHandler::Startup() {
}

bool LogHandler::Init() {
	RETURN_ON_FAIL(LoadConfig());
	ZeroMemory(&s_wsaData, sizeof(s_wsaData));
	RETURN_ON_FAIL(0 == WSAStartup(MAKEWORD(2, 2), &s_wsaData));
	RETURN_ON_FAIL(ResolveAddrInfo());
	return true;
}

bool LogHandler::LoadConfig() {	
	// retrieve TCP port for LogHandler to listen to
	// retrieve port number string from config file, and use 13000 as default if any error occurs.
	DWORD dw = ::GetPrivateProfileString(s_szSection, s_szPortKey, NULL,
		s_szPort, 10, s_szConfigFilePath);
	RETURN_ON_FAIL(0 < dw && dw < MAX_PORT_STRING_LEN);	// returned dw value does not include the null terminator.

	// retrieve maximum number of client connection allowed
	s_nMaxConnection = ::GetPrivateProfileInt(s_szSection, s_szMaxConnKey, 5, s_szConfigFilePath);
	return true;
}

bool LogHandler::ListenAsync() {
	RETURN_ON_FAIL(CreateSocket());
	RETURN_ON_FAIL(BindTcpSocket());
	RETURN_ON_FAIL(ListenOnSocket());

	s_bIsListening = true;
	s_hListenThread = (HANDLE)_beginthreadex(
		NULL,
		NULL,
		DoListeningStuff,
		NULL,
		NULL,
		NULL);

	// Here I assume this listening thread will never be failed to create.
	// Therefore no error detection is put here.
	// The newly created thread will continue executing, while this function will
	// return.

	return true;
}

void LogHandler::CleanUpWinsock() {
	if (s_pAddrInfo != NULL) {
		FreeAddrInfo(s_pAddrInfo);
		s_pAddrInfo = NULL;
	}
	if (s_listenSocket != INVALID_SOCKET) {
		closesocket(s_listenSocket);
		s_listenSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

void LogHandler::CleanUpHandler() {
	if (s_bIsListening) {
		StopListening();
	}
	CleanUpWinsock();
}

void LogHandler::StopListening() {
	// try to stop the listening thread
	s_bIsListening = false;
	closesocket(s_listenSocket);
	s_listenSocket = INVALID_SOCKET;
	// Block this thread until the listen thread is actually finished
	WaitForSingleObject(s_hListenThread, INFINITE);
	CloseHandle(s_hListenThread);
	s_hListenThread = INVALID_HANDLE_VALUE;
}

bool LogHandler::ResolveAddrInfo() {
	ADDRINFOT	hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	if (NULL != s_pAddrInfo) {
		FreeAddrInfo(s_pAddrInfo);
	}
	if (0 != ::GetAddrInfo(NULL, s_szPort, &hints, &s_pAddrInfo)) {
		WSACleanup();
		return false;
	}
	return true;
}

bool LogHandler::CreateSocket() {
	s_listenSocket = socket(s_pAddrInfo->ai_family, s_pAddrInfo->ai_socktype, s_pAddrInfo->ai_protocol);
	if (INVALID_SOCKET == s_listenSocket) {
		FreeAddrInfo(s_pAddrInfo);
		s_pAddrInfo = NULL;
		WSACleanup();
		return false;
	}
	return true;
}

bool LogHandler::BindTcpSocket() {
	int res = bind(s_listenSocket, s_pAddrInfo->ai_addr, (int)s_pAddrInfo->ai_addrlen);
	if (SOCKET_ERROR == res) {
		FreeAddrInfo(s_pAddrInfo);
		s_pAddrInfo = NULL;
		closesocket(s_listenSocket);
		s_listenSocket = INVALID_SOCKET;
		WSACleanup();
		return false;
	}
	return true;
}

bool LogHandler::ListenOnSocket() {
	// This allows program accepts the most acceptable connections on this socket
	int res = listen(s_listenSocket, SOMAXCONN);
	if (SOCKET_ERROR == res) {
		closesocket(s_listenSocket);
		s_listenSocket = INVALID_SOCKET;
		WSACleanup();
		return false;
	}
	return true;
}

VOID NTAPI LogHandler::HandleConnSimple(PTP_CALLBACK_INSTANCE pInst, PVOID pContext) {
	// do something
	SOCKET* pClientSocket = static_cast<SOCKET*>(pContext);
	SOCKET clientSo = *pClientSocket;
	//delete pClientSocket;

	DWORD byte_received = 0;
	CmdProcessor cp;
	if (!cp.Init()) {
		return;
	}
	int res = 0;
	DWORD flag = MSG_WAITALL;
	
	TcpCommBuf temp_buf;
	while (true) {		
		// read command buffer from client socket
		if (!temp_buf.Alloc(CMD_HEADER_SIZE)) {
			break;
		}

		flag = MSG_WAITALL;
		res = WSARecv(clientSo, &temp_buf, 1, &byte_received, &flag, NULL, NULL);
		if (res != 0 || temp_buf.len != byte_received) {
			DWORD dwError = WSAGetLastError();
			break;
		}

		// interpret command
		int nVersion = 0;
		int nCmdID = CMD_ID_INVALID;
		int nParamCount = 0;
		int nSize = 0;
		if (!cp.ParseCmdBuf(temp_buf.buf, temp_buf.len, &nVersion, &nCmdID, &nParamCount, &nSize)) {
			break;
		}
		if (nVersion != 1) {
			break;
		}

		// read parameter buffer for the previous command
		int nParamSize = nSize - CMD_HEADER_SIZE;
		if (nParamSize > 0) {
			if (!temp_buf.Alloc(nParamSize)) {
				break;
			}

			flag = MSG_WAITALL;
			res = WSARecv(clientSo, &temp_buf, 1, &byte_received, &flag, NULL, NULL);
			if (res != 0 || temp_buf.len != byte_received) {
				DWORD dwError = WSAGetLastError();
				break;
			}
		}

		// Parse parameters and execute the command
		if (!cp.ExecuteCommand(temp_buf.buf, temp_buf.len, NULL, NULL)) {
			break;
		}

		temp_buf.AssignBuf(cp.GetResponseBuf(), cp.GetResponseBufSize());

		DWORD dwBytesSent = 0;
		res = WSASend(clientSo, &temp_buf, 1, &dwBytesSent, 0, NULL, NULL);
		if (res != 0 || temp_buf.len != dwBytesSent) {
			DWORD dwError = WSAGetLastError();
			break;
		}
	}
}

unsigned int WINAPI LogHandler::DoListeningStuff(PVOID /*pContext*/) {
	SOCKET temp_client_socket = INVALID_SOCKET;
	SOCKADDR saClient;
	int len_saClient = sizeof(saClient);

	while (s_bIsListening) {
		temp_client_socket = WSAAccept(
			s_listenSocket,
			&saClient,
			&len_saClient,
			NULL,
			NULL);

		if (temp_client_socket == SOCKET_ERROR) {
			// client socket failed
			continue;
		}

		SOCKET*	pClientSocket = new SOCKET;
		*pClientSocket = temp_client_socket;
		if (!TrySubmitThreadpoolCallback(HandleConnSimple, (PVOID)pClientSocket, NULL)) {
			// submit failed
			continue;
		}
	}

	CleanUpWinsock();

	// Before thread ends, release a structrue that is allocated when
	// creating the thread by calling _beginthreadex
	_endthreadex(0);	// It seems that _beginthreadex has called this internally
						// So this line can be deleted.
	return 0;
}
