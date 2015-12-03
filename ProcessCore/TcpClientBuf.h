// TcpClientBuf.h
// A buffer class inherited from the WSABuf, handle buffer allocating & releasing work.
//

#pragma once

#include "CoreDeclaration.h"

namespace ProcessCore {
	class TcpClientBuf : public WSABUF {
		// ==================================================
		// Interfaces

		// constructors & desturctor
	public:
		TcpClientBuf();
		virtual ~TcpClientBuf();

		// public interfaces
	public:
		bool	AssignBuf(const char* buf, int len);
		bool	BindBuf(char* buf, int len);
		bool	Alloc(int len);

		// help functions
	protected:
		void	SetWSABuf();

		// ===================================================
		// Data members
	protected:
		char*	m_pBuf;
		int		m_nLen;

	};
};
