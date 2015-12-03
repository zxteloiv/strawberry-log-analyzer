// RecordDepository.cpp
// Implements the RecordDepository class, along with a TCP client to connect the LogServer TCP server.
// 

#include "stdafx.h"

#include "RecordDepository.h"

#include "..\\BaseUtils\\BaseUtils.h"
#include "..\\LogServer\\LogExchangeProtocol.h"

#include <process.h>
#include "TcpTranslator.h"
#include "TcpClientBuf.h"

using namespace ProcessCore;

// =====================================================================================
// Const static variables initialization

const TCHAR* RecordDepository::s_csConfSectionDepo = _T("RecordDepository");
const TCHAR* RecordDepository::s_csConfItemLogSvrName = _T("log_svr_name");
const TCHAR* RecordDepository::s_csConfItemLogSvrPort = _T("log_svr_port");
const TCHAR* RecordDepository::s_csConfItemDepoSize = _T("depo_size");

ADDRINFOT*	RecordDepository::s_pAddrInfo = NULL;
int			RecordDepository::s_nMaxQueueSize = 0;
WSADATA		RecordDepository::s_wsaData;
TCHAR		RecordDepository::s_szDestName[s_cnMaxDestNameSize];
TCHAR		RecordDepository::s_szDestPort[s_cnMaxDestPortSize];

// =====================================================================================
// Class Function definition

RecordDepository::RecordDepository(PRINT_FUNC pOutFunc) {
	m_nLogID = 0;
	m_pPrintLogFunc = pOutFunc;
	m_pDepoQueue = NULL;
	m_pTcpTranslator = NULL;
	m_clientSocket = INVALID_SOCKET;
	m_hReadingThread = INVALID_HANDLE_VALUE;
	m_pSendBuf = NULL;
	m_pRecvBuf = NULL;
}

RecordDepository::~RecordDepository() {
	this->CloseSocket();
	DELETE_POINTER(m_pSendBuf);
	DELETE_POINTER(m_pRecvBuf);
	if (m_pDepoQueue != NULL) {
		while (!m_pDepoQueue->empty()) {
			BaseUtils::_Record_Ptr pRec = m_pDepoQueue->front();
			m_pDepoQueue->pop();
			delete pRec;
		}
		DELETE_POINTER(m_pDepoQueue);
	}
	DELETE_POINTER(m_pTcpTranslator);
}

bool RecordDepository::Init(int log_id) {
	PROCESS_ERROR(NULL != (m_pDepoQueue = new RECQUEUE));
	m_nLogID = log_id;
	RETURN_ON_FAIL(ConnectSocket());
	PROCESS_ERROR(NULL != (m_pTcpTranslator = new TcpTranslator));
	PROCESS_ERROR(NULL != (m_pSendBuf = new TcpClientBuf));
	PROCESS_ERROR(NULL != (m_pRecvBuf = new TcpClientBuf));
	
	return true;
Exit0:
	DELETE_POINTER(m_pDepoQueue);
	DELETE_POINTER(m_pTcpTranslator);
	DELETE_POINTER(m_pSendBuf);
	DELETE_POINTER(m_pRecvBuf);
	return true;
}

BaseUtils::_Record_Ptr RecordDepository::GetARecord() {
	using namespace BaseUtils;
	_Record_Ptr pRec = NULL;
	if (m_pDepoQueue->empty() && (!ReadRecords(s_nMaxQueueSize))) {
		return NULL;		
	}
	pRec = m_pDepoQueue->front();
	m_pDepoQueue->pop();

	return pRec;
}

void RecordDepository::ReleaseRecord(BaseUtils::_Record_Ptr pRecord) {
	if (pRecord != NULL) {
		delete pRecord;
	}
}

bool RecordDepository::CheckBoolResponse(int cmd_id) {
	// waiting to receive the response to VALIDATE command
	RETURN_ON_FAIL(m_pRecvBuf->Alloc(CMD_HEADER_SIZE));	// allocate space for holding the response header
	DWORD dwBytesReceived = 0;
	DWORD dwFlags = MSG_WAITALL;
	int res = WSARecv(m_clientSocket, m_pRecvBuf, 1, &dwBytesReceived, &dwFlags, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	// receive the response header
	int nVersion = 0;
	char flag = 0x00;
	int nCmdID = CMD_ID_INVALID;
	int nParamCount = 0;
	int nSize = 0;
	m_pTcpTranslator->ParseResponseHeader(m_pRecvBuf->buf, m_pRecvBuf->len,
		&nVersion, &flag, &nCmdID, &nParamCount, &nSize);
	
	// RETURN_ON_FAIL when these criteria are not satisfied.
	RETURN_ON_FAIL(1 == nVersion);			// Only Log Exchange Protocol Version 1 is allowed.
	RETURN_ON_FAIL(flag & CMD_MASK_RTN);	// the buffer is a response rather than a command
	RETURN_ON_FAIL(nCmdID == cmd_id);	// it's a response for the command just sent out
	RETURN_ON_FAIL(1 == nParamCount);	// response to CMD_ID_HASMORERECORD contains only 1 parameter
	
	// receive the response parameters
	RETURN_ON_FAIL(m_pRecvBuf->Alloc(nSize - CMD_HEADER_SIZE));	// allocate space for the parameter buffer
	dwFlags = MSG_WAITALL;
	res = WSARecv(m_clientSocket, m_pRecvBuf, 1, &dwBytesReceived, &dwFlags, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	// analyze the response
	int offset = 0;
	int nParamSize = 0;
	RETURN_ON_FAIL(0 == memcpy_s(&nParamSize, sizeof(nParamSize), m_pRecvBuf->buf + offset, CMD_PARAM_SIZE_LEN));
	offset += CMD_PARAM_SIZE_LEN;
	RETURN_ON_FAIL(sizeof(bool) == nParamSize);	// the only returned value is bool type
	bool rtn = false;

	RETURN_ON_FAIL(0 == memcpy_s(&rtn, sizeof(rtn), m_pRecvBuf->buf + offset, nParamSize));

	return rtn;
}	

bool RecordDepository::IsLogAlive() {
	RETURN_ON_FAIL(m_pTcpTranslator->BuildCommand(1, 0, CMD_ID_VALIDATE, 1, m_nLogID));
	RETURN_ON_FAIL(m_pSendBuf->AssignBuf(m_pTcpTranslator->GetBuffer(), m_pTcpTranslator->GetBufferLen()));

	// send out the command buffer
	int res = 0;
	DWORD dwBytesSent = 0;
	res = WSASend(m_clientSocket, m_pSendBuf, 1, &dwBytesSent, 0, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	return CheckBoolResponse(CMD_ID_VALIDATE);
}

bool RecordDepository::HasAnyMoreRecords() {
	if (!m_pDepoQueue->empty()) {
		return true;
	}

	// Build command buffer
	RETURN_ON_FAIL(m_pTcpTranslator->BuildCommand(1, 0, CMD_ID_HASMORERECORD, 0));
	RETURN_ON_FAIL(m_pSendBuf->AssignBuf(m_pTcpTranslator->GetBuffer(), m_pTcpTranslator->GetBufferLen()));

	// send out the command buffer
	int res = 0;
	DWORD dwBytesSent = 0;
	res = WSASend(m_clientSocket, m_pSendBuf, 1, &dwBytesSent, 0, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	return CheckBoolResponse(CMD_ID_HASMORERECORD);
}

bool RecordDepository::StartDepository() {
	RETURN_ON_FAIL(m_pTcpTranslator->BuildCommand(1, 0, CMD_ID_REQUESTLOG, 1, m_nLogID));
	RETURN_ON_FAIL(m_pSendBuf->AssignBuf(m_pTcpTranslator->GetBuffer(), m_pTcpTranslator->GetBufferLen()));

	// send out the command buffer
	int res = 0;
	DWORD dwBytesSent = 0;
	res = WSASend(m_clientSocket, m_pSendBuf, 1, &dwBytesSent, 0, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	return CheckBoolResponse(CMD_ID_REQUESTLOG);
}

bool RecordDepository::ShutdownDepository() {
	RETURN_ON_FAIL(m_pTcpTranslator->BuildCommand(1, 0, CMD_ID_CLOSEREQUEST, 0));
	RETURN_ON_FAIL(m_pSendBuf->AssignBuf(m_pTcpTranslator->GetBuffer(), m_pTcpTranslator->GetBufferLen()));

	// send out the command buffer
	int res = 0;
	DWORD dwBytesSent = 0;
	res = WSASend(m_clientSocket, m_pSendBuf, 1, &dwBytesSent, 0, NULL, NULL);
	RETURN_ON_FAIL(0 == res);
	
	return CheckBoolResponse(CMD_ID_CLOSEREQUEST);
}

bool RecordDepository::ReadRecords(int amount) {
	// Build command
	RETURN_ON_FAIL(m_pTcpTranslator->BuildCommand(1, 0, CMD_ID_LOADRECORD, 1, amount));
	RETURN_ON_FAIL(m_pSendBuf->AssignBuf(m_pTcpTranslator->GetBuffer(), m_pTcpTranslator->GetBufferLen()));

	// send out the command buffer
	int res = 0;
	DWORD dwBytesSent = 0;
	res = WSASend(m_clientSocket, m_pSendBuf, 1, &dwBytesSent, 0, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	// waiting to receive the response to VALIDATE command
	RETURN_ON_FAIL(m_pRecvBuf->Alloc(CMD_HEADER_SIZE));	// allocate space for holding the response header
	DWORD dwFlags = MSG_WAITALL;
	DWORD dwBytesReceived = 0;
	res = WSARecv(m_clientSocket, m_pRecvBuf, 1, &dwBytesReceived, &dwFlags, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	// receive the response header
	int nVersion = 0;
	char flag = 0x00;
	int nCmdID = CMD_ID_INVALID;
	int nParamCount = 0;
	int nSize = 0;
	m_pTcpTranslator->ParseResponseHeader(m_pRecvBuf->buf, m_pRecvBuf->len,
		&nVersion, &flag, &nCmdID, &nParamCount, &nSize);
	
	// RETURN_ON_FAIL when these criteria are not satisfied.
	RETURN_ON_FAIL(1 == nVersion);			// Only Log Exchange Protocol Version 1 is allowed.
	RETURN_ON_FAIL(flag & CMD_MASK_RTN);	// the buffer is a response rather than a command
	RETURN_ON_FAIL(nCmdID == CMD_ID_LOADRECORD);	// it's a response for the command just sent out
	RETURN_ON_FAIL(nParamCount <= amount);	// the number of the field returned is less than or equal to what is requested
	RETURN_ON_FAIL(0 < nParamCount && 0 < (nSize - CMD_HEADER_SIZE));

	// OK, allocate buffer for remaining parameters and then receive them
	RETURN_ON_FAIL(m_pRecvBuf->Alloc(nSize - CMD_HEADER_SIZE));
	dwFlags = MSG_WAITALL;
	res = WSARecv(m_clientSocket, m_pRecvBuf, 1, &dwBytesReceived, &dwFlags, NULL, NULL);
	RETURN_ON_FAIL(0 == res);

	// Extract records from the received binary buffer
	RETURN_ON_FAIL(AnalyzeFieldMessage(nParamCount));
	
	return true;
}

bool RecordDepository::AnalyzeFieldMessage(int nParamCount) {
	// analyze fields, extract records returned.
	int record_id = 0;
	int offset = 0;	// move cursor to the beginning of the first field of the first
	BaseUtils::_Record_Ptr pRec = NULL;
	RETURN_ON_FAIL(NULL != (pRec = new BaseUtils::Record));
	PROCESS_ERROR(pRec->Init());
	BaseUtils::_Field_Ptr pField = NULL;
	// In the Response to LoadRecord, Parameter Count means the field count rather than the record count.
	for (int nFieldBufID = 0; nFieldBufID < nParamCount; ++nFieldBufID) {
		int inner_offset = FIELD_SIZE_OFFSET;	// cursor position inside a field

		int nFieldSize = 0;	// size of the memory contains the content of this field
		memcpy_s(&nFieldSize, sizeof(nFieldSize), m_pRecvBuf->buf + offset + inner_offset, FIELD_SIZE_LEN);
		inner_offset += FIELD_SIZE_LEN;

		int new_record_id = (int)((BYTE)(m_pRecvBuf->buf[offset + inner_offset]));
		inner_offset += FIELD_RECORD_ID_LEN;
		if (record_id != new_record_id) {
			// following is a new record, just save the current record
			m_pDepoQueue->push(pRec);
			pRec = NULL;
			PROCESS_ERROR(NULL != (pRec = new BaseUtils::Record));
			PROCESS_ERROR(pRec->Init());
			record_id = new_record_id;
		}

		int field_type_id = (int)((BYTE)(m_pRecvBuf->buf[offset + inner_offset]));
		inner_offset += FIELD_TYPE_LEN;
		if (field_type_id == FIELD_TYPE_INTEGER) {	// integer field
			PROCESS_ERROR(NULL != (pField = GetNextIntFieldFromTCPBuffer(offset, inner_offset)));
		} else if (field_type_id == FIELD_TYPE_REAL) { // real field
			PROCESS_ERROR(NULL != (pField = GetNextRealFieldFromTCPBuffer(offset, inner_offset)));
		} else if (field_type_id == FIELD_TYPE_STRING) {	// string field
			PROCESS_ERROR(NULL != (pField = GetNextStringFieldFromTCPBuffer(offset, inner_offset)));
		} else if (field_type_id == FIELD_TYPE_TIMESTAMP) {	// timestamp field
			PROCESS_ERROR(NULL != (pField = GetNextTimestampFieldFromTCPBuffer(offset, inner_offset)));
		} else {	// error
			PROCESS_ERROR(false);
		}

		PROCESS_ERROR(nFieldSize == inner_offset);

		PROCESS_ERROR(pRec->AddField(pField));

		offset += nFieldSize;	// move to next field
	}
	if (pRec != NULL) {
		m_pDepoQueue->push(pRec);
		pRec = NULL;
	}
	return true;
Exit0:
	DELETE_POINTER(pField);
	DELETE_POINTER(pRec);
	return false;
}

bool RecordDepository::ConnectSocket() {
	if (m_clientSocket != INVALID_SOCKET)
		closesocket(m_clientSocket);

	ADDRINFOT* ptr = NULL;

	for(ptr = s_pAddrInfo; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		m_clientSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		RETURN_ON_FAIL(m_clientSocket != INVALID_SOCKET);

        // Connect to server.
		if (SOCKET_ERROR == connect(m_clientSocket, ptr->ai_addr, (int)ptr->ai_addrlen)) {
			closesocket(m_clientSocket);
			m_clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	RETURN_ON_FAIL(m_clientSocket != INVALID_SOCKET);
	return true;
}

void RecordDepository::CloseSocket() {
	if (m_clientSocket != INVALID_SOCKET) {
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}
}

BaseUtils::_Field_Ptr RecordDepository::GetNextIntFieldFromTCPBuffer(int& offset, int& inner_offset) {
	// get value size
	int nValueSize = 0;
	PROCESS_ERROR(0 == memcpy_s(&nValueSize, sizeof(nValueSize),
		m_pRecvBuf->buf + offset + inner_offset, FIELD_VALUE_SIZE_LEN));
	inner_offset += FIELD_VALUE_SIZE_LEN;

	// get value from buffer
	int nValue = 0;
	PROCESS_ERROR(0 == memcpy_s(&nValue, sizeof(nValue), m_pRecvBuf->buf + offset + inner_offset, nValueSize));
	inner_offset += nValueSize;

	BaseUtils::_Field_Ptr pField = NULL;
	pField = new BaseUtils::IntField(nValue);
	return pField;
Exit0:
	return NULL;
}

BaseUtils::_Field_Ptr RecordDepository::GetNextRealFieldFromTCPBuffer(int& offset, int& inner_offset) {
	// get value size
	int nValueSize = 0;
	PROCESS_ERROR(0 == memcpy_s(&nValueSize, sizeof(nValueSize), 
		m_pRecvBuf->buf + offset + inner_offset, FIELD_VALUE_SIZE_LEN));
	inner_offset += FIELD_VALUE_SIZE_LEN;

	// get value from buffer
	BaseUtils::Real rValue = 0;
	PROCESS_ERROR(0 == memcpy_s(&rValue, sizeof(rValue), m_pRecvBuf->buf + offset + inner_offset, nValueSize));
	inner_offset += nValueSize;

	BaseUtils::_Field_Ptr pField = NULL;
	pField = new BaseUtils::RealField(rValue);
	return pField;
Exit0:
	return NULL;
}

BaseUtils::_Field_Ptr RecordDepository::GetNextStringFieldFromTCPBuffer(int& offset, int& inner_offset) {
	int nStringCount = 0;
	PROCESS_ERROR(0 == memcpy_s(&nStringCount, sizeof(nStringCount),
		m_pRecvBuf->buf + offset + inner_offset, FIELD_VALUE_SIZE_LEN));
	inner_offset += FIELD_VALUE_SIZE_LEN;

	TCHAR* pStrValue = NULL;
	PROCESS_ERROR(NULL != (pStrValue = new TCHAR[nStringCount]));
	PROCESS_ERROR(0 == memcpy_s(pStrValue, CharCountToSize(nStringCount),
		m_pRecvBuf->buf + offset + inner_offset, CharCountToSize(nStringCount)));
	inner_offset += CharCountToSize(nStringCount);

	BaseUtils::_Field_Ptr pField = NULL;
	pField = new BaseUtils::StringField(pStrValue);
	return pField;
Exit0:
	DELETE_ARRAY_POINTER(pStrValue);
	return NULL;
}

BaseUtils::_Field_Ptr RecordDepository::GetNextTimestampFieldFromTCPBuffer(int& offset, int& inner_offset) {
	short year = 0;
	char month = 0, day = 0, hour = 0, minute = 0, second = 0;
	PROCESS_ERROR(0 == memcpy_s(&year, sizeof(year), m_pRecvBuf->buf + offset + inner_offset, FIELD_TIME_YEAR_LEN));
	inner_offset += FIELD_TIME_YEAR_LEN;
	PROCESS_ERROR(0 == memcpy_s(&month, sizeof(month), m_pRecvBuf->buf + offset + inner_offset, FIELD_TIME_MONTH_LEN));
	inner_offset += FIELD_TIME_MONTH_LEN;
	PROCESS_ERROR(0 == memcpy_s(&day, sizeof(day), m_pRecvBuf->buf + offset + inner_offset, FIELD_TIME_DAY_LEN));
	inner_offset += FIELD_TIME_DAY_LEN;
	PROCESS_ERROR(0 == memcpy_s(&hour, sizeof(hour), m_pRecvBuf->buf + offset + inner_offset, FIELD_TIME_HOUR_LEN));
	inner_offset += FIELD_TIME_HOUR_LEN;
	PROCESS_ERROR(0 == memcpy_s(&minute, sizeof(minute), m_pRecvBuf->buf + offset + inner_offset, FIELD_TIME_MINUTE_LEN));
	inner_offset += FIELD_TIME_MINUTE_LEN;
	PROCESS_ERROR(0 == memcpy_s(&second, sizeof(second), m_pRecvBuf->buf + offset + inner_offset, FIELD_TIME_SECOND_LEN));
	inner_offset += FIELD_TIME_SECOND_LEN;

	BaseUtils::_Field_Ptr pField = NULL;
	pField = new BaseUtils::TimestampField(year, month, day, hour, minute, second);
	return pField;
Exit0:
	return NULL;
}


// ====================================================================================
// Static function implementations

bool RecordDepository::InitDepository() {
	RETURN_ON_FAIL(InitTcpClient());
	RETURN_ON_FAIL(LoadConfig());

	return true;
}

bool RecordDepository::LoadConfig() {
	// default value is -1 which indicates an error has occured.
	s_nMaxQueueSize = ::GetPrivateProfileInt(s_csConfSectionDepo, s_csConfItemDepoSize, -1, g_szConfigFile);
	RETURN_ON_FAIL(0 < s_nMaxQueueSize);	// any negative number is not allowed
	return true;
}

bool RecordDepository::InitTcpClient() {
	int res = WSAStartup(MAKEWORD(2, 2), &s_wsaData);
	RETURN_ON_FAIL(0 == res);

	DWORD dwRes = ::GetPrivateProfileString(s_csConfSectionDepo, s_csConfItemLogSvrName, NULL,
		s_szDestName, s_cnMaxDestNameSize, g_szConfigFile);
	RETURN_ON_FAIL(dwRes > 0);
	dwRes = ::GetPrivateProfileString(s_csConfSectionDepo, s_csConfItemLogSvrPort, NULL,
		s_szDestPort, s_cnMaxDestPortSize, g_szConfigFile);
	RETURN_ON_FAIL(dwRes > 0);

	
	ADDRINFOT hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// resolve server name and port
	int nRes = ::GetAddrInfo(s_szDestName, s_szDestPort, &hints, &s_pAddrInfo);
	if (nRes != 0) {
		WSACleanup();
		return false;
	}

	return true;
}

bool RecordDepository::Cleanup() {
	if (s_pAddrInfo != NULL) {
		FreeAddrInfo(s_pAddrInfo);
	}
	WSACleanup();
	return true;
}

unsigned int RecordDepository::ReadingThread(void *pContext) {
	// here to send or receive message
	RecordDepository* pDepo = reinterpret_cast<RecordDepository*>(pContext);

	return 1;
}



