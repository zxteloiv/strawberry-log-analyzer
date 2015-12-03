// IntField.cpp
// Implements the IntField class
//

#include "stdafx.h"
#include "IntField.h"

using namespace BaseUtils;

IntField::IntField() : BaseUtils::Field(), m_value(0) {
	this->m_fieldType = BaseUtils::Field::integer;
}

IntField::IntField(int val) : BaseUtils::Field(), m_value(val) {
	this->m_fieldType = BaseUtils::Field::integer;
}

IntField::~IntField() {
}
