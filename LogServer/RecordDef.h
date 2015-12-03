// RecordDef.h
// Defines classes that work at the log server side, in charge of nomalization of
// log records, containing the declaration of a record
//

#pragma once

#include "LogServerDef.h"

namespace LogServer {

	enum LogTextType {
		text_log,
		xml_log,
		reserved_text_type
	};

	class RecordDef {
		// =============================================
		// Interfaces
		
		// Constructors and destructors
	protected:
		RecordDef();
	public:
		virtual ~RecordDef();
		bool Init(LogTextType	logType,
			const TCHAR* encoding = NULL,
			const TCHAR* start_symbol = NULL,
			const TCHAR* end_symbol = NULL);

		// Factory methods
	public:
		static	_RecordDef_Ptr	ParseDefFromXML(const TCHAR*);
		static	bool	ReleaseDef(_RecordDef_Ptr);
		static	bool	AddFieldToRecord(_RecordDef_Ptr, IFieldDef * const &);

		// Public interfaces
	public:
		int				GetFieldCount();
		LogTextType		GetLogTextType() const { return m_type; }
		const TCHAR*	GetStartSymbol() const { return m_szStartSymbol; }
		const TCHAR*	GetEndSymbol() const { return m_szEndSymbol; }
		const TCHAR*	GetEncoding() const { return m_szEncoding; }

		const IFieldDef* GetFieldDefAt(int) const;

		int		GetStartCch() const { return m_nStartCch; }
		int		GetEndCch() const { return m_nEndCch; }

		bool	ToAnsiDef(RecordDefAnsi** ppAnsi, UINT codepage_id);

		// =============================================
		// Data members
	protected:
		LPFIELDDEFVECTOR	m_pFieldDefs;
		LogTextType			m_type;

		TCHAR*		m_szStartSymbol;
		TCHAR*		m_szEndSymbol;
		TCHAR*		m_szEncoding;

		int			m_nStartCch;
		int			m_nEndCch;

		// =============================================
		// Static members
	public:
		static const TCHAR* s_szElemNameLogDef;
		static const TCHAR* s_szElemNameRecord;
		static const TCHAR* s_szElemNameField;
		static const TCHAR* s_szAttrNameLogEncoding;
		static const TCHAR* s_szAttrNameLogType;
		static const TCHAR* s_szAttrValueLogTypeXml;
		static const TCHAR* s_szAttrValueLogTypeText;
		static const TCHAR* s_szAttrNameFieldCount;
		static const TCHAR* s_szAttrNameRecStart;
		static const TCHAR* s_szAttrNameRecEnd;
		static const TCHAR* s_szAttrNameFieldStart;
		static const TCHAR* s_szAttrNameFieldEnd;
		static const TCHAR* s_szAttrNameFieldTrim;
		static const TCHAR* s_szAttrValueTrimTrue;
		static const TCHAR* s_szAttrValueTrimFalse;
		static const TCHAR* s_szAttrNameFieldType;
		static const TCHAR* s_szAttrValueFieldTypeInt;
		static const TCHAR* s_szAttrValueFieldTypeReal;
		static const TCHAR* s_szAttrValueFieldTypeTime;
		static const TCHAR* s_szAttrValueFieldTypeString;
		static const TCHAR* s_szAttrNameFieldAlias;
		static const TCHAR* s_szEmpty;

		// ==============================================
		// Static Functions
	public:
		static bool NameIsXmlComponent(const TCHAR*, const TCHAR*);
	};

	class RecordDefAnsi {
		friend class RecordDef;	// only the RecordDef class can produce a RecordDefAnsi
		// =============================================
		// Interfaces
		
		// Constructors and destructors
	public:
		RecordDefAnsi();
		virtual ~RecordDefAnsi();

		// Public interfaces
	public:
		int			GetFieldCount();
		LogTextType	GetLogTextType() const { return m_type; }
		const char*	GetStartSymbol() const { return m_szStartSymbol; }
		const char*	GetEndSymbol() const { return m_szEndSymbol; }

		const IFieldDefAnsi* GetFieldDefAt(int) const;

		int		GetStartCb() const { return m_nStartCb; }
		int		GetEndCb() const { return m_nEndCb; }

		// =============================================
		// Data members
	protected:
		LPANSIFIELDDEFVECTOR	m_pFieldDefs;

		LogTextType	m_type;

		char*		m_szStartSymbol;
		char*		m_szEndSymbol;

		int			m_nStartCb;
		int			m_nEndCb;
	};
};
