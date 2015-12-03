// FixedLengthCompare.h
// Declare both ANSI and Unicode version of the FixedLengthCompare class, which is used to
// compare a fixed length string and a template string in CmdProcessor class. The fixed
// length string has a continous input of character stream.
//

#pragma once

namespace LogServer {	
	class FixedLengthCompareW {
		// ===========================================================
		// Interfaces
	public:
		FixedLengthCompareW();
		~FixedLengthCompareW();
		bool Init(int len, const TCHAR* szTemplate);
		void Reset();
		bool AppendAndTest(TCHAR ch);

		int	 GetTemplateLength() const { return m_nLen; }
		const TCHAR* GetTemplateBuf() const { return m_pTemplate; }

	protected:
		TCHAR*	m_pCompBuf;
		TCHAR*	m_pTemplate;
		int		m_nLen;
		int		m_nPos;
		bool	m_bFull;	// true if the number of input characters has exceeded the length once
	};

	class FixedLengthCompareA {
		// ===========================================================
		// Interfaces
	public:
		FixedLengthCompareA();
		~FixedLengthCompareA();
		bool Init(int len, const char* szTemplate);
		void Reset();
		bool AppendAndTest(char ch);

		int	 GetTemplateLength() const { return m_nLen; }
		const char* GetTemplateBuf() const { return m_pTemplate; }

	protected:
		char*	m_pCompBuf;
		char*	m_pTemplate;
		int		m_nLen;
		int		m_nPos;
		bool	m_bFull;	// true if the number of input characters has exceeded the length once
	};

	bool TranslatingCopy(TCHAR** ppDest, const TCHAR* source);
};
