// TextFieldDef.cpp
// Implements the TextFieldDef class which is derived from the IFieldDef interface.
//

#include "stdafx.h"
#include "FieldDef.h"
#include "TextFieldDef.h"
#include "FixedLengthCompare.h"
#include <stdarg.h>		// for variable arguments of member function
#include <string>

// A UTF32 character is 4 bytes at most &  1 is for null terminator
#define WideCchToCharLen(X) ((X) * 4 + 1)

using namespace LogServer;

TextFieldDef::TextFieldDef(bool trim,
						   BaseUtils::Field::FieldType type)
						   : m_bTrim(trim), m_type(type) {
	m_szStartSymbol = NULL;
	m_szEndSymbol = NULL;
	m_szAlias = NULL;
}

TextFieldDef::~TextFieldDef() {
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
	DELETE_ARRAY_POINTER(m_szAlias);
}

bool TextFieldDef::Init(const TCHAR *start_symbol, const TCHAR *end_symbol, const TCHAR *alias) {
	int size = 0;
	if (start_symbol != NULL) {
		PROCESS_ERROR(LogServer::TranslatingCopy(&m_szStartSymbol, start_symbol));
		m_nStartCch = _tcslen(m_szStartSymbol);	// count of characters doesn't contain the null-terminator
	}
	if (end_symbol != NULL) {
		PROCESS_ERROR(LogServer::TranslatingCopy(&m_szEndSymbol, end_symbol));
		m_nEndCch = _tcslen(m_szEndSymbol);	// count of characters doesn't contain the null-terminator
	}
	if (alias != NULL) {
		PROCESS_ERROR(LogServer::TranslatingCopy(&m_szAlias, alias));
		m_nAliasCch = _tcslen(m_szAlias);	// count of characters doesn't contain the null-terminator
	}

	return true;
Exit0:
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
	DELETE_ARRAY_POINTER(m_szAlias);
	return false;
}

bool TextFieldDef::ToAnsiDef(TextFieldDefAnsi* pAnsiDef, int codepage_id) const {
	DELETE_ARRAY_POINTER(pAnsiDef->m_szStartSymbol);
	DELETE_ARRAY_POINTER(pAnsiDef->m_szEndSymbol);
	DELETE_ARRAY_POINTER(pAnsiDef->m_szAlias);

	// Convert start symbol together with the null-terminator
	int len = WideCchToCharLen(m_nStartCch);
	PROCESS_ERROR(NULL != (pAnsiDef->m_szStartSymbol = new char[len]));
	PROCESS_ERROR(m_nStartCch + 1 == WideCharToMultiByte(codepage_id, NULL, m_szStartSymbol, m_nStartCch + 1,
		pAnsiDef->m_szStartSymbol, len, NULL, NULL));
	pAnsiDef->m_nStartCb = strlen(pAnsiDef->m_szStartSymbol);
	
	// Convert end symbol together with the null-terminator
	len = WideCchToCharLen(m_nEndCch);
	PROCESS_ERROR(NULL != (pAnsiDef->m_szEndSymbol = new char[len]));
	PROCESS_ERROR(m_nEndCch + 1 == WideCharToMultiByte(codepage_id, NULL, m_szEndSymbol, m_nEndCch + 1,
		pAnsiDef->m_szEndSymbol, len, NULL, NULL));
	pAnsiDef->m_nEndCb = strlen(pAnsiDef->m_szEndSymbol);
	
	// Convert alias symbol together with the null-terminator
	len = WideCchToCharLen(m_nAliasCch);
	PROCESS_ERROR(NULL != (pAnsiDef->m_szAlias = new char[len]));
	PROCESS_ERROR(0 < WideCharToMultiByte(codepage_id, NULL, m_szAlias, -1,
		pAnsiDef->m_szAlias, len, NULL, NULL));
	pAnsiDef->m_nAliasCb = strlen(pAnsiDef->m_szAlias);
	
	// Assign the trim & type, too
	pAnsiDef->m_bTrim = this->m_bTrim;
	pAnsiDef->m_type = this->m_type;

	return true;
Exit0:
	DELETE_ARRAY_POINTER(pAnsiDef->m_szStartSymbol);
	DELETE_ARRAY_POINTER(pAnsiDef->m_szEndSymbol);
	DELETE_ARRAY_POINTER(pAnsiDef->m_szAlias);
	return false;
};

TextFieldDefAnsi::TextFieldDefAnsi(bool trim, BaseUtils::Field::FieldType type) : m_bTrim(trim), m_type(type) {
	m_szStartSymbol = NULL;
	m_szEndSymbol = NULL;
	m_szAlias = NULL;
}

TextFieldDefAnsi::~TextFieldDefAnsi() {
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
	DELETE_ARRAY_POINTER(m_szAlias);
}

bool TextFieldDefAnsi::Init(const char* start_symbol, const char* end_symbol, const char* alias) {
	int size = 0;
	if (start_symbol != NULL) {
		size = strlen(start_symbol) + 1;
		PROCESS_ERROR(NULL != (m_szStartSymbol = new char[size]));
		PROCESS_ERROR(0 == strcpy_s(m_szStartSymbol, size, start_symbol));
		m_nStartCb = size - 1;	// count of characters doesn't contain the null-terminator
	}
	if (end_symbol != NULL) {
		size = strlen(end_symbol) + 1;
		PROCESS_ERROR(NULL != (m_szEndSymbol = new char[size]));
		PROCESS_ERROR(0 == strcpy_s(m_szEndSymbol, size, end_symbol));
		m_nEndCb = size - 1;	// count of characters doesn't contain the null-terminator
	}
	if (alias != NULL) {
		size = strlen(alias) + 1;
		PROCESS_ERROR(NULL != (m_szAlias = new char[size]));
		PROCESS_ERROR(0 == strcpy_s(m_szAlias, size, alias));
		m_nAliasCb = size - 1;	// count of characters doesn't contain the null-terminator
	}

	return true;
Exit0:
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
	DELETE_ARRAY_POINTER(m_szAlias);
	return false;
}

