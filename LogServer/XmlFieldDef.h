// XmlFieldDef.h
// Contains classes for record declaration in XML log files
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h" // for BaseUtils::Field::FieldType
#include "LogServerDef.h"
#include "FieldDef.h"

namespace LogServer {

	class XmlFieldDef : public IFieldDef {
		// ============================================
		// Interfaces

		// Constructors and destructors
	public:
		XmlFieldDef();
		virtual ~XmlFieldDef();
		
		// Public interfaces
	public:
		virtual	const TCHAR*	GetStartSymbol() const;
		virtual	const TCHAR*	GetEndSymbol() const;
		virtual const TCHAR*	GetAlias() const;

		// Get count of character
		virtual int GetStartCch() const;
		virtual int GetEndCch() const;
		virtual int GetAliasCch() const;

		virtual	BaseUtils::Field::FieldType GetFieldType() const;

		// ============================================
		// Data members
	protected:
	};

	class XmlFieldDefAnsi : public IFieldDefAnsi {
	public:
		virtual	const char*	GetStartSymbol() const;
		virtual	const char*	GetEndSymbol() const;
		virtual const char*	GetAlias() const;

		// Get count of character
		virtual int GetStartCb() const;
		virtual int GetEndCb() const;
		virtual int GetAliasCb() const;

		virtual	BaseUtils::Field::FieldType GetFieldType() const;
	};
};
