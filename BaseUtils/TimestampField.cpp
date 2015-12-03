// Timestamp.cpp
// Implements the TimestampField class
//

#include "stdafx.h"
#include "TimestampField.h"

using namespace BaseUtils;

TimestampField::TimestampField(const Timestamp& time) : BaseUtils::Field(), m_value(time) {
	this->m_fieldType = BaseUtils::Field::timestamp;
}

TimestampField::TimestampField(short year, char month, char day, char hour, char minute, char second)
: BaseUtils::Field(), m_value(year, month, day, hour, minute, second)
{
	m_fieldType = BaseUtils::Field::timestamp;
}

TimestampField::~TimestampField() {
}

const Timestamp* TimestampField::GetPointerTime() const {
	return &m_value;
}
