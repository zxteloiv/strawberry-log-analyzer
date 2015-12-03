// field.cpp
// Implement the Field abstract class
//

#include "stdafx.h"
#include "field.h"

#pragma warning( disable : 4018 )	// disable waring for '<' signed/unsigned mismatch

using namespace BaseUtils;

Field::Field() : m_fieldType(any_field_type), m_pChildFields(NULL), m_pRecordBelongedTo(NULL) {
}

Field::~Field() {
	RELEASE_STD_CONTAINER_CONTENT(std::vector<Field*>, m_pChildFields);
	DELETE_POINTER(m_pChildFields);
}

_Field_Ptr Field::GetChildField(int subindex) const {
	if (m_pChildFields != NULL && subindex < m_pChildFields->size()) {
		return m_pChildFields->at(subindex);
	}
	return NULL;
}
