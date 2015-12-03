// TcpClientBuf.cpp
// implements the TcpClientBuf class.
//

#include "stdafx.h"
#include "TcpClientBuf.h"

using namespace ProcessCore;

TcpClientBuf::TcpClientBuf() {
	buf = NULL;
	len = 0;
	m_pBuf = NULL;
	m_nLen = 0;
}

TcpClientBuf::~TcpClientBuf() {
	DELETE_ARRAY_POINTER(m_pBuf);
}

bool TcpClientBuf::AssignBuf(const char *buf, int len) {
	RETURN_ON_FAIL(Alloc(len));
	RETURN_ON_FAIL(0 == memcpy_s(m_pBuf, m_nLen, buf, len));

	SetWSABuf();
	return true;
}

bool TcpClientBuf::BindBuf(char *buf, int len) {
	DELETE_ARRAY_POINTER(m_pBuf);

	m_pBuf = buf;
	m_nLen = len;

	SetWSABuf();
	return true;
}

bool TcpClientBuf::Alloc(int len) {
	DELETE_ARRAY_POINTER(m_pBuf);

	m_pBuf = new char[len];
	RETURN_ON_FAIL(m_pBuf != NULL);
	m_nLen = len;

	SetWSABuf();
	return true;
}

void TcpClientBuf::SetWSABuf() {
	this->buf = m_pBuf;
	this->len = m_nLen;
}
