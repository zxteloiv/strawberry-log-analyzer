// TcpCommBuf.h
// A wrapper for the WSABuf Struct in WinSock.
//

#pragma once

namespace LogServer {
	class TcpCommBuf : public WSABUF {
		// ==================================================
		// Interfaces

		// constructors & desturctor
	public:
		TcpCommBuf();
		virtual ~TcpCommBuf();

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
