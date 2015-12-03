// StringField.cpp
// Implements the StringField class.
//

#include "stdafx.h"
#include "StringField.h"

using namespace BaseUtils;

StringField::StringField() : BaseUtils::Field(), m_pContent(NULL) {
	m_fieldType = BaseUtils::Field::string;
}

StringField::StringField(TCHAR* str) : BaseUtils::Field(), m_pContent(str) {
	m_fieldType = BaseUtils::Field::string;
}

StringField::~StringField() {
	if (m_pContent != NULL)
		delete[] m_pContent;
}

bool StringField::BindString(TCHAR *string) {
	if (m_pContent) {
		delete[] m_pContent;
		m_pContent = NULL;
	}
	m_pContent = string;
	return true;
}
