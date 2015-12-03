// FixedLengthCompare.cpp
// Implement both the ANSI and Unicode version of FixedLengthCompare class.
//

#include "stdafx.h"
#include "FixedLengthCompare.h"
#include "LogServerDef.h"
#include <string>

using namespace LogServer;

FixedLengthCompareW::FixedLengthCompareW() {
	m_pCompBuf = NULL;
	m_pTemplate = NULL;
	m_nLen = 0;
	m_nPos = 0;
	m_bFull = false;
}

FixedLengthCompareW::~FixedLengthCompareW() {
	DELETE_ARRAY_POINTER(m_pCompBuf);
	DELETE_ARRAY_POINTER(m_pTemplate);
}

bool FixedLengthCompareW::Init(int len, const TCHAR *szTemplate) {
	if (len > 0) {
		DELETE_ARRAY_POINTER(m_pCompBuf);
		DELETE_ARRAY_POINTER(m_pTemplate);
		PROCESS_ERROR(NULL != (m_pCompBuf = new TCHAR[len]) && NULL != (m_pTemplate = new TCHAR[len]));
		memcpy(m_pTemplate, szTemplate, sizeof(TCHAR) * len);
	}
	m_nLen = len;
	m_bFull = (0 == m_nLen);
	m_nPos = 0;

	return true;

Exit0:
	DELETE_ARRAY_POINTER(m_pCompBuf);
	DELETE_ARRAY_POINTER(m_pTemplate);
	return false;	
}

void FixedLengthCompareW::Reset() {
	m_nPos = 0;
	m_bFull = (0 == m_nLen);
}

bool FixedLengthCompareW::AppendAndTest(TCHAR ch) {
	if (0 == m_nLen)
		return true;

	m_pCompBuf[m_nPos] = ch;
	m_nPos++;
	if (m_nPos == m_nLen) {
		m_bFull = true;
		m_nPos = 0;
	}
	if (!m_bFull)
		return false;

	for (int comp_ch_counter = 0; comp_ch_counter < m_nLen; ++comp_ch_counter) {
		int comp_pos = (m_nPos + comp_ch_counter) % m_nLen;
		if (m_pCompBuf[comp_pos] != m_pTemplate[comp_ch_counter])
			return false;
	}

	return true;
}

FixedLengthCompareA::FixedLengthCompareA() {
	m_pCompBuf = NULL;
	m_pTemplate = NULL;
	m_nLen = 0;
	m_nPos = 0;
	m_bFull = false;
}

FixedLengthCompareA::~FixedLengthCompareA() {
	DELETE_ARRAY_POINTER(m_pCompBuf);
	DELETE_ARRAY_POINTER(m_pTemplate);
}

bool FixedLengthCompareA::Init(int len, const char *szTemplate) {
	if (len > 0) {
		DELETE_ARRAY_POINTER(m_pCompBuf);
		DELETE_ARRAY_POINTER(m_pTemplate);
		PROCESS_ERROR(NULL != (m_pCompBuf = new char[len]) && NULL != (m_pTemplate = new char[len]));
		memcpy(m_pTemplate, szTemplate, len);
	}
	m_nLen = len;
	m_bFull = (0 == len);
	m_nPos = 0;
	return true;

Exit0:
	DELETE_ARRAY_POINTER(m_pCompBuf);
	DELETE_ARRAY_POINTER(m_pTemplate);
	return false;	
}

void FixedLengthCompareA::Reset() {
	m_nPos = 0;
	m_bFull = (0 == m_nLen);	// if the length is zero, the buffer is always taken as full
}

bool FixedLengthCompareA::AppendAndTest(char ch) {
	if (0 == m_nLen)
		return true;

	m_pCompBuf[m_nPos] = ch;
	m_nPos++;
	if (m_nPos == m_nLen) {
		m_bFull = true;
		m_nPos = 0;
	}
	if (!m_bFull)
		return false;

	for (int comp_ch_counter = 0; comp_ch_counter < m_nLen; ++comp_ch_counter) {
		int comp_pos = (m_nPos + comp_ch_counter) % m_nLen;
		if (m_pCompBuf[comp_pos] != m_pTemplate[comp_ch_counter])
			return false;
	}

	return true;
}

// TranslatingCopy
// A string-copy function that can translate plain escape characters into their correct form.
bool LogServer::TranslatingCopy(TCHAR** ppDest, const TCHAR* source) {
	FixedLengthCompare flcDS;	// double slash
	FixedLengthCompare flcCR;	// carriage return
	FixedLengthCompare flcLF;	// line feed
	FixedLengthCompare flcTab;	// tab
	RETURN_ON_FAIL(flcCR.Init(2, _T("\\r")) && flcLF.Init(2, _T("\\n")) &&
		flcDS.Init(2, _T("\\\\")) && flcTab.Init(2, _T("\\t")));
	const TCHAR* CR = _T("\r");
	const TCHAR* LF = _T("\n");
	const TCHAR* TAB = _T("\t");
	const TCHAR* DS = _T("\\");

#ifdef UNICODE
	std::wstring destbuf;
#else
	std::string destbuf;
#endif

	TCHAR appendbuf[2] = _T("");
	for (int nTchPos = 0; source[nTchPos] != _T(''); ++nTchPos) {
		appendbuf[0] = source[nTchPos];
		destbuf.append(appendbuf);
		if (flcDS.AppendAndTest(source[nTchPos])) {	// double slash has the highest priority
			destbuf.erase(destbuf.size() - flcDS.GetTemplateLength(), flcDS.GetTemplateLength());
			destbuf.append(DS);	// when "\\" is specified in XML, it means a "\" should be appended
		} else if (flcCR.AppendAndTest(source[nTchPos])) {
			destbuf.erase(destbuf.size() - flcCR.GetTemplateLength(), flcCR.GetTemplateLength());
			destbuf.append(CR);
		} else if (flcLF.AppendAndTest(source[nTchPos])) {
			destbuf.erase(destbuf.size() - flcLF.GetTemplateLength(), flcLF.GetTemplateLength());
			destbuf.append(LF);
		} else if (flcTab.AppendAndTest(source[nTchPos])) {
			destbuf.erase(destbuf.size() - flcTab.GetTemplateLength(), flcTab.GetTemplateLength());
			destbuf.append(TAB);
		} else
			continue;

		// Although it's not necessary to call Reset() when only one flc is used, it is possible that a flc
		// test is passed while others are not. But here after a flc test is passed, all FLCs have to be
		// reset to prevent them use part of characters passed in the previous FLC test as their own.
		// For example when string "aab" comes in, FLC with a template of "aa" must take effect eariler than
		// the FLC with a template "ab".
		flcDS.Reset();
		flcCR.Reset();
		flcLF.Reset();
		flcTab.Reset();
	}

	*ppDest = NULL;
	int nDestSize = destbuf.size();;
	RETURN_ON_FAIL(NULL != (*ppDest = new TCHAR[nDestSize + 1]) && (nDestSize == destbuf.copy(*ppDest, nDestSize)));
	(*ppDest)[destbuf.size()] = _T('\0');
	
	return true;
}
