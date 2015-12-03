// Translator.cpp
// Implements TcpTranslator class.
// 

#include "stdafx.h"
#include "TcpTranslator.h"

#include <stdarg.h>		// for variable arguments of member function
#include "..\\LogServer\\LogExchangeProtocol.h"

using namespace ProcessCore;

TcpTranslator::TcpTranslator() : m_buf(NULL), m_len(0) {
}

TcpTranslator::~TcpTranslator() {
	ClearBuf();
}

void TcpTranslator::ClearBuf() {
	DELETE_ARRAY_POINTER(m_buf);
	m_len = 0;
}

bool TcpTranslator::BuildCommand(int nVersion, int nFlag, int nCmdID, int nParamCount, ...) {
	// Currently only the Log Exchange Protocol Version 1 is supported, in which only 1 byte is used
	// for parameter counter.
	RETURN_ON_FAIL(1 == nVersion && nParamCount <= 255);

	ClearBuf();
	const int param_size = sizeof(int) * 2;	// assume all the parameters are integers, a parameter has a 4-bit size & 4-bit content
	RETURN_ON_FAIL(NULL != (m_buf = new char[CMD_HEADER_SIZE + nParamCount * param_size]));
	m_len = CMD_HEADER_SIZE + nParamCount * param_size;

	m_buf[FLAG_VERSION_OFFSET] = (char)((BYTE)nVersion);
	m_buf[FLAG_OPTION_OFFSET] = (char)(nFlag & (~OPTION_RTN));	// this tcp message is a command rather than a response
	m_buf[FLAG_CMDID_OFFSET] = (char)((BYTE)nCmdID);
	m_buf[FLAG_PARAM_OFFSET] = (char)((BYTE)nParamCount);	// make sure parameter counter supports a 255 maximum
	
	// The command buffer size. Because m_len is an integer, no leak will occur.
	PROCESS_ERROR(0 == memcpy_s(m_buf + FLAG_CMD_SIZE_OFFSET, sizeof(int), &m_len, sizeof(m_len)));

	// then copy all the command parameters to buffer
	va_list params;
	va_start(params, nParamCount);
	for (int paramID = 0, offset = CMD_HEADER_SIZE; paramID < nParamCount; ++paramID) {
		// For a command parameter, a length descriptor & the parameter itself are needed.
		int nParam = va_arg(params, int);
		int nParamSize = sizeof(nParam);

		// Write the parameter size into buffer
		// LEPv1 uses a 4-bit integer to store parameter size
		PROCESS_ERROR(0 == memcpy_s(m_buf + offset, sizeof(int), &nParamSize, sizeof(nParamSize)));
		offset += sizeof(nParamSize);

		// Write the parameter itself into buffer
		PROCESS_ERROR(0 == memcpy_s(m_buf + offset, nParamSize, &nParam, nParamSize));
		offset += nParamSize;
	}
	va_end(params);


	return true;
Exit0:
	ClearBuf();

	// Variable parameters are placed on the thread stack, it will be cleaned after the function returns.
	// Therefore no manual clean is needed.
	return false;
}

bool TcpTranslator::ParseResponseHeader(const char *buf, int length, int *pVersion,
										char *pFlag, int *pCmdID, int *pParamCount, int *pSize)
{
	RETURN_ON_FAIL(CMD_HEADER_SIZE <= length);

	// All of these flags are 1 byte long, therefore it's possible to use assignment directly.
	int version = (int)((BYTE)buf[FLAG_VERSION_OFFSET]);
	char flag = (char)((BYTE)buf[FLAG_OPTION_OFFSET]);
	int cmd_id = (int)((BYTE)buf[FLAG_CMDID_OFFSET]);
	int param_count = (int)((BYTE)buf[FLAG_PARAM_OFFSET]);
	int size = 0;
	RETURN_ON_FAIL(0 == memcpy_s(&size, sizeof(size), buf + FLAG_CMD_SIZE_OFFSET, FLAG_CMD_SIZE_LEN));

	if (pVersion)
		*pVersion = version;
	if (pFlag)
		*pFlag = flag;
	if (pCmdID)
		*pCmdID = cmd_id;
	if (pParamCount)
		*pParamCount = param_count;
	if (pSize)
		*pSize = size;
		
	return true;
}


