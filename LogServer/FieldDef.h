// FieldDef.h
// Contains the interface declaration for a single field.
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h" // for BaseUtils::Field::FieldType

#include "LogServerDef.h"

namespace LogServer {	
	class IFieldDef {
	public:
		virtual	const TCHAR*	GetStartSymbol() const = NULL;
		virtual	const TCHAR*	GetEndSymbol() const = NULL;
		virtual const TCHAR*	GetAlias() const = NULL;

		// Get count of character
		virtual int GetStartCch() const = NULL;
		virtual int GetEndCch() const = NULL;
		virtual int GetAliasCch() const = NULL;

		virtual	BaseUtils::Field::FieldType GetFieldType() const = NULL;
	};

	class IFieldDefAnsi {
	public:
		virtual	const char*	GetStartSymbol() const = NULL;
		virtual	const char*	GetEndSymbol() const = NULL;
		virtual const char*	GetAlias() const = NULL;

		// Get count of character
		virtual int GetStartCb() const = NULL;
		virtual int GetEndCb() const = NULL;
		virtual int GetAliasCb() const = NULL;

		virtual	BaseUtils::Field::FieldType GetFieldType() const = NULL;
	};
};
