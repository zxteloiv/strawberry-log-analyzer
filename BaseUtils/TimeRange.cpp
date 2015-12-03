// TimeRange.cpp
// Implementation of the TimeRange class.
//

#include "stdafx.h"
#include "TimeRange.h"
#include "Timestamp.h"

using namespace BaseUtils;

bool TimeRange::ParseTimeRangeFromString(const TCHAR* str, TimeRange* pTimeRange) {
	return true;
};

TimeRange::TimeRange() : m_year(0), m_month(0), m_day(0), m_hour(0), m_minute(0), m_second(0) {
	m_szOutputString = NULL;
}

TimeRange::TimeRange(Real year, Real mon, Real day, Real hour, Real min, Real sec)
: m_year(year), m_month(mon), m_day(day), m_hour(hour), m_minute(min), m_second(sec){
}

TimeRange::TimeRange(const TimeRange& other) : m_year(other.m_year), m_month(other.m_month), m_day(other.m_day),
m_hour(other.m_hour), m_minute(other.m_minute), m_second(other.m_second) {
}

TimeRange::~TimeRange() {
}

TimeRange& TimeRange::operator =(const BaseUtils::TimeRange &other) {
	m_year = other.m_year;
	m_month = other.m_month;
	m_day = other.m_day;
	m_hour = other.m_hour;
	m_minute = other.m_minute;
	m_second = other.m_second;
	return (*this);
}

const TCHAR* TimeRange::ToString() const {
	return m_szOutputString;
}

TimeRange TimeRange::operator +(const BaseUtils::TimeRange &other) const {
	TimeRange trNew(*this);
	trNew.m_year += other.m_year;
	trNew.m_month += other.m_month;
	trNew.m_day += other.m_day;
	trNew.m_hour += other.m_hour;
	trNew.m_minute += other.m_minute;
	trNew.m_second += other.m_second;

	return trNew;
}

TimeRange& TimeRange::operator +=(const BaseUtils::TimeRange &other) {
	m_year += other.m_year;
	m_month += other.m_month;
	m_day += other.m_day;
	m_hour += other.m_hour;
	m_minute += other.m_minute;
	m_second += other.m_second;
	return (*this);
}

TimeRange TimeRange::operator -(const BaseUtils::TimeRange &other) const {
	TimeRange trNew(*this);
	trNew.m_year -= other.m_year;
	trNew.m_month -= other.m_month;
	trNew.m_day -= other.m_day;
	trNew.m_hour -= other.m_hour;
	trNew.m_minute -= other.m_minute;
	trNew.m_second -= other.m_second;

	return trNew;
}

TimeRange& TimeRange::operator -=(const BaseUtils::TimeRange &other) {
	m_year -= other.m_year;
	m_month -= other.m_month;
	m_day -= other.m_day;
	m_hour -= other.m_hour;
	m_minute -= other.m_minute;
	m_second -= other.m_second;
	return (*this);
}

TimeRange TimeRange::operator *(int multiplier) const {
	TimeRange trNew(*this);
	trNew.m_year *= multiplier;
	trNew.m_month *= multiplier;
	trNew.m_day *= multiplier;
	trNew.m_hour *= multiplier;
	trNew.m_minute *= multiplier;
	trNew.m_second *= multiplier;

	return trNew;
}

TimeRange& TimeRange::operator *=(int multiplier) {
	m_year *= multiplier;
	m_month *= multiplier;
	m_day *= multiplier;
	m_hour *= multiplier;
	m_minute *= multiplier;
	m_second *= multiplier;
	return (*this);
}

TimeRange TimeRange::operator *(BaseUtils::Real multiplier) const {
	TimeRange trNew(*this);
	trNew.m_year *= multiplier;
	trNew.m_month *= multiplier;
	trNew.m_day *= multiplier;
	trNew.m_hour *= multiplier;
	trNew.m_minute *= multiplier;
	trNew.m_second *= multiplier;

	return trNew;
}

TimeRange& TimeRange::operator *=(BaseUtils::Real multiplier) {
	m_year *= multiplier;
	m_month *= multiplier;
	m_day *= multiplier;
	m_hour *= multiplier;
	m_minute *= multiplier;
	m_second *= multiplier;
	return (*this);
}

int TimeRange::operator/(const TimeRange& other) const {
	return 0;
}
