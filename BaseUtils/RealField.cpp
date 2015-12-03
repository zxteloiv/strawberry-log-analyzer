// RealField.cpp
// Implements the RealField class
//

#include "stdafx.h"
#include "RealField.h"

using namespace BaseUtils;

RealField::RealField() : BaseUtils::Field(), m_value(0) {
	m_fieldType = BaseUtils::Field::real;
}

RealField::RealField(Real &val) : BaseUtils::Field(), m_value(val) {
	m_fieldType = BaseUtils::Field::real;
}

RealField::~RealField() {
}