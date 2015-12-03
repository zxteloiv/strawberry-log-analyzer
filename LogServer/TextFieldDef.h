// TextFieldDef.h
// Contains field declaration class designed for pure text log file
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h" // for BaseUtils::Field::FieldType
#include "LogServerDef.h"
#include "FieldDef.h"

namespace LogServer {
	
	class TextFieldDef : public IFieldDef {
		// ============================================
		// Interfaces

		// Constructor and destructor and initializer
	public:
		TextFieldDef(bool trim, BaseUtils::Field::FieldType type);
		virtual ~TextFieldDef();
		bool Init(const TCHAR* start_symbol, const TCHAR* end_symbol, const TCHAR* alias);
		
		// Public interfaces
	public:
		virtual	const TCHAR*	GetStartSymbol() const { return m_szStartSymbol; }
		virtual	const TCHAR*	GetEndSymbol() const { return m_szEndSymbol; }
		virtual const TCHAR*	GetAlias() const { return m_szAlias; }

		// Get count of character
		virtual int GetStartCch() const { return m_nStartCch; }
		virtual int GetEndCch() const { return m_nEndCch; }
		virtual int GetAliasCch() const { return m_nAliasCch; }

		virtual	BaseUtils::Field::FieldType GetFieldType() const { return m_type; }

		bool ToAnsiDef(TextFieldDefAnsi*, int) const;
		// ============================================
		// Data members
	protected:
		TCHAR*	m_szStartSymbol;	// indicates where a record starts
		TCHAR*	m_szEndSymbol;		// indicates where a record ends
		TCHAR*	m_szAlias;			// an alias for friendly indentify the field

		int		m_nStartCch;		// the number of characters in a Start Symbol
		int		m_nEndCch;			// the number of characters in an End Symbol
		int		m_nAliasCch;		// the number of characters in an Alias
		
		BaseUtils::Field::FieldType	m_type;
		bool	m_bTrim;
	};

	class TextFieldDefAnsi : public IFieldDefAnsi {
		friend class TextFieldDef;
		// ============================================
		// Interfaces

		// Constructor and destructor and initializer
	public:
		TextFieldDefAnsi() : m_szStartSymbol(NULL), m_szEndSymbol(NULL), m_szAlias(NULL) {}
		TextFieldDefAnsi(bool trim, BaseUtils::Field::FieldType type);
		virtual ~TextFieldDefAnsi();
		bool Init(const char* start_symbol, const char* end_symbol, const char* alias);
		
		// Public interfaces
	public:
		virtual	const char*	GetStartSymbol() const { return m_szStartSymbol; }
		virtual	const char*	GetEndSymbol() const { return m_szEndSymbol; }
		virtual const char*	GetAlias() const { return m_szAlias; }
		// Get count of character
		virtual int GetStartCb() const { return m_nStartCb; }
		virtual int GetEndCb() const { return m_nEndCb; }
		virtual int GetAliasCb() const { return m_nAliasCb; }

		virtual	BaseUtils::Field::FieldType GetFieldType() const { return m_type; }

		// ============================================
		// Data members
	protected:
		char*	m_szStartSymbol;	// indicates where a record starts
		char*	m_szEndSymbol;		// indicates where a record ends
		char*	m_szAlias;			// an alias for friendly indentify the field

		int		m_nStartCb;		// the number of characters in a Start Symbol
		int		m_nEndCb;			// the number of characters in an End Symbol
		int		m_nAliasCb;		// the number of characters in an Alias
		
		BaseUtils::Field::FieldType	m_type;
		bool	m_bTrim;
	};
};
