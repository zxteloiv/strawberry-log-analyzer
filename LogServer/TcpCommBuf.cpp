// TcpCommBuf.cpp
// Implement TcpCommBuf class
//

#include "stdafx.h"
#include "TcpCommBuf.h"

using namespace LogServer;

TcpCommBuf::TcpCommBuf() {
	buf = NULL;
	len = 0;
	m_pBuf = NULL;
	m_nLen = 0;
}

TcpCommBuf::~TcpCommBuf() {
	DELETE_ARRAY_POINTER(m_pBuf);
}

bool TcpCommBuf::AssignBuf(const char *buf, int len) {
	RETURN_ON_FAIL(Alloc(len));
	RETURN_ON_FAIL(0 == memcpy_s(m_pBuf, m_nLen, buf, len));

	SetWSABuf();
	return true;
}

bool TcpCommBuf::BindBuf(char *buf, int len) {
	DELETE_ARRAY_POINTER(m_pBuf);

	m_pBuf = buf;
	m_nLen = len;

	SetWSABuf();
	return true;
}

bool TcpCommBuf::Alloc(int len) {
	DELETE_ARRAY_POINTER(m_pBuf);

	m_pBuf = new char[len];
	RETURN_ON_FAIL(m_pBuf != NULL);
	m_nLen = len;
	SetWSABuf();
	return true;
}

void TcpCommBuf::SetWSABuf() {
	this->buf = m_pBuf;
	this->len = m_nLen;
}
