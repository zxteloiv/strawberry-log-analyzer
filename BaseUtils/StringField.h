// StringField.h
// Declarations for StringField class.
//

#pragma once

#include "field.h"

namespace BaseUtils {
	class StringField : public Field {
		// ===============================================
		// Interfaces

		// Constructors and destructors
	public:
		StringField();

		// Use an existing string and bind it
		StringField(TCHAR* str);
		~StringField();

	private:
		DISALLOW_COPY_AND_ASSIGN(StringField);

		// public interfaces
	public:
		const TCHAR*	GetValueString() const { return m_pContent; }
		bool	BindString(TCHAR* string);

		// ===============================================
		// Data members
	protected:
		TCHAR*	m_pContent;
	};
};
