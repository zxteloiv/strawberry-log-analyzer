// TcpTranslator.h
// TcpTranslator class is responsible for translating binary bytes in a TCP command into objects, and
// vice versa.
// 

#pragma once

#include "CoreDeclaration.h"

namespace ProcessCore {
	class TcpTranslator {
		// ================================================
		// Interfaces

		// Constructors & destructors
	public:
		TcpTranslator();
		virtual ~TcpTranslator();

		// Public interfaces
	public:
		bool	BuildCommand(int nVersion, int nFlag, int nCmdID, int nParamCount, ...);
		char*	GetBuffer() { return m_buf; }
		int		GetBufferLen() { return m_len; }
		void	ClearBuf();

		bool	ParseResponseHeader(const char* buf, int length, int* pVersion, char* pFlag,
			int* pCmdID, int* pParamCount, int* pSize);

		// ==================================================
		// Data members
	protected:
		char*	m_buf;
		int		m_len;
	};
};
