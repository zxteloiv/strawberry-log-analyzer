// Timestamp.cpp
// Implements Timestamp class
//

#include "stdafx.h"
#include "Timestamp.h"
#include "TimeRange.h"

using namespace BaseUtils;

#pragma warning(disable : 4244)	// disable warning for converting from Real to unsigned int

const int Timestamp::LeapYearMonthDays[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const int Timestamp::NormalYearMonthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define IsLeapYear(X)	(((0 == (X) % 4) && (0 != (X) % 100)) || (0 == (X) % 400))

Timestamp::Timestamp() : m_year(1900), m_month(1), m_day(1), m_hour(0), m_minute(0), m_second(0) {
	m_szOutputString = NULL;
}

Timestamp::Timestamp(int year, int mon, int day, int hour, int min, int sec)
: m_year(year), m_month(mon), m_day(day), m_hour(hour), m_minute(min), m_second(sec) {
	m_szOutputString = NULL;
}

Timestamp::~Timestamp() {
	DELETE_ARRAY_POINTER(m_szOutputString);
}

Timestamp::Timestamp(const Timestamp& another)
	: m_year(another.m_year), m_month(another.m_month), m_day(another.m_day),
	m_hour(another.m_hour), m_minute(another.m_minute), m_second(another.m_second) {
}

Timestamp& Timestamp::operator =(const Timestamp& other) {
	m_year = other.m_year;
	m_month = other.m_month;
	m_day = other.m_day;
	m_hour = other.m_hour;
	m_minute = other.m_minute;
	m_second = other.m_second;
	return (*this);
}

const TCHAR* Timestamp::ToString() const {
	// do something to assign value to the output string

	return m_szOutputString;
}

bool Timestamp::operator !=(const BaseUtils::Timestamp &other) const {
	return (m_year != other.m_year || m_month != other.m_month ||
		m_day != other.m_day || m_hour != other.m_hour ||
		m_minute != other.m_minute || m_second != other.m_second);
}

bool Timestamp::operator <(const BaseUtils::Timestamp &other) const {
	if (m_year < other.m_year)
		return true;
	else if (m_year > other.m_year)
		return false;
	else if (m_month < other.m_month) // when years are the same, continues checking remaining parts
		return true;
	else if (m_month > other.m_month)
		return false;
	else if (m_day < other.m_day) // when months are still the same, continues checking remaining parts
		return true;
	else if (m_day > other.m_day)
		return false;
	else if (m_hour < other.m_hour) // when days are still the same, continues checking remaining parts
		return true;
	else if (m_hour > other.m_hour)
		return false;
	else if (m_minute < other.m_minute) // when hours are still the same, continues checking remaining parts
		return true;
	else if (m_minute > other.m_minute)
		return false;
	else if (m_second < other.m_second) // when minutes are still the same, continues checking remaining parts
		return true;
	else
		return false;
}

bool Timestamp::operator <=(const BaseUtils::Timestamp &other) const {
	if (m_year < other.m_year)
		return true;
	else if (m_year > other.m_year)
		return false;
	else if (m_month < other.m_month) // when years are the same, continues checking remaining parts
		return true;
	else if (m_month > other.m_month)
		return false;
	else if (m_day < other.m_day) // when months are still the same, continues checking remaining parts
		return true;
	else if (m_day > other.m_day)
		return false;
	else if (m_hour < other.m_hour) // when days are still the same, continues checking remaining parts
		return true;
	else if (m_hour > other.m_hour)
		return false;
	else if (m_minute < other.m_minute) // when hours are still the same, continues checking remaining parts
		return true;
	else if (m_minute > other.m_minute)
		return false;
	else if (m_second <= other.m_second) // when minutes are still the same, continues checking remaining parts
		return true;
	else
		return false;
}

bool Timestamp::operator ==(const BaseUtils::Timestamp &other) const {
	return (m_year == other.m_year && m_month == other.m_month &&
		m_day == other.m_day && m_hour == other.m_hour &&
		m_minute == other.m_minute && m_second == other.m_second);
}

bool Timestamp::operator >(const BaseUtils::Timestamp &other) const {
	if (m_year < other.m_year)
		return false;
	else if (m_year > other.m_year)
		return true;
	else if (m_month < other.m_month) // when years are the same, continues checking remaining parts
		return false;
	else if (m_month > other.m_month)
		return true;
	else if (m_day < other.m_day) // when months are still the same, continues checking remaining parts
		return false;
	else if (m_day > other.m_day)
		return true;
	else if (m_hour < other.m_hour) // when days are still the same, continues checking remaining parts
		return false;
	else if (m_hour > other.m_hour)
		return true;
	else if (m_minute < other.m_minute) // when hours are still the same, continues checking remaining parts
		return false;
	else if (m_minute > other.m_minute)
		return true;
	else if (m_second > other.m_second)	// when minutes are still the same, continues checking remaining parts
		return true;
	else
		return false;
}

bool Timestamp::operator >=(const BaseUtils::Timestamp &other) const {
	if (m_year < other.m_year)
		return false;
	else if (m_year > other.m_year)
		return true;
	else if (m_month < other.m_month) // when years are the same, continues checking remaining parts
		return false;
	else if (m_month > other.m_month)
		return true;
	else if (m_day < other.m_day) // when months are still the same, continues checking remaining parts
		return false;
	else if (m_day > other.m_day)
		return true;
	else if (m_hour < other.m_hour) // when days are still the same, continues checking remaining parts
		return false;
	else if (m_hour > other.m_hour)
		return true;
	else if (m_minute < other.m_minute) // when hours are still the same, continues checking remaining parts
		return false;
	else if (m_minute > other.m_minute)
		return true;
	else if (m_second >= other.m_second)	// at last the second is larger
		return true;
	else
		return false;
}

Timestamp Timestamp::operator +(const BaseUtils::TimeRange &other) const {
	Timestamp tNew(*this);
	tNew.IncreaseYear(other.m_year);
	tNew.IncreaseMonth(other.m_month);
	tNew.IncreaseSecond(other.m_second);
	tNew.IncreaseMinute(other.m_minute);
	tNew.IncreaseHour(other.m_hour);
	tNew.IncreaseDay(other.m_day);	
	return tNew;
}

Timestamp& Timestamp::operator +=(const BaseUtils::TimeRange &other) {
	IncreaseYear(other.m_year);
	IncreaseMonth(other.m_month);
	IncreaseSecond(other.m_second);
	IncreaseMinute(other.m_minute);
	IncreaseHour(other.m_hour);
	IncreaseDay(other.m_day);
	return (*this);
}

Timestamp Timestamp::operator -(const BaseUtils::TimeRange &other) const {
	Timestamp tNew(*this);
	tNew.DecreaseYear(other.m_year);
	tNew.DecreaseMonth(other.m_month);
	tNew.DecreaseDay(other.m_day);
	tNew.DecreaseSecond(other.m_second);
	tNew.DecreaseMinute(other.m_minute);
	tNew.DecreaseHour(other.m_hour);
	return tNew;
}

Timestamp& Timestamp::operator -=(const BaseUtils::TimeRange &other) {
	DecreaseYear(other.m_year);
	DecreaseMonth(other.m_month);
	DecreaseDay(other.m_day);
	DecreaseSecond(other.m_second);
	DecreaseMinute(other.m_minute);
	DecreaseHour(other.m_hour);
	return (*this);
}

TimeRange Timestamp::operator-(const Timestamp& other) const {
	TimeRange trNew;
	trNew.m_year = m_year - other.m_year;
	trNew.m_month = m_month - other.m_month;
	trNew.m_day = m_day - other.m_day;
	trNew.m_hour = m_hour - other.m_hour;
	trNew.m_minute = m_minute - other.m_minute;
	trNew.m_second = m_second - other.m_second;
	return trNew;
}


void Timestamp::IncreaseYear(unsigned int addend) {
	m_year += addend;
}

void Timestamp::IncreaseMonth(unsigned int addend) {
	m_month += addend;
	while (m_month > 12) {
		IncreaseYear();
		m_month -= 12;
	}
}

void Timestamp::IncreaseDay(unsigned int addend) {
	m_day += addend;
	// never 
	int nCurrentMonthDays = IsLeapYear(m_year) ? LeapYearMonthDays[m_month - 1] : NormalYearMonthDays[m_month - 1];
	while (m_day > nCurrentMonthDays) {
		m_day -= nCurrentMonthDays;
		IncreaseMonth();
		nCurrentMonthDays = IsLeapYear(m_year) ? LeapYearMonthDays[m_month - 1] : NormalYearMonthDays[m_month - 1];
	}
}

void Timestamp::IncreaseHour(unsigned int addend) {
	m_hour += addend;
	while (m_hour >= 24) {
		m_hour -= 24;
		IncreaseDay();
	}
}
void Timestamp::IncreaseMinute(unsigned int addend) {
	m_minute += addend;
	while (m_minute >= 60) {
		m_minute -= 60;
		IncreaseHour();
	}
}

void Timestamp::IncreaseSecond(unsigned int addend) {
	m_second += addend;
	while (m_second >= 60) {
		m_second -= 60;
		IncreaseMinute();
	}
}

void Timestamp::DecreaseYear(unsigned int subtrahend) {
	m_year -= subtrahend;
}

void Timestamp::DecreaseMonth(unsigned int subtrahend) {
	m_month -= subtrahend;
	while (m_month < 1) {
		DecreaseYear();
		m_month += 12;
	}
}

void Timestamp::DecreaseDay(unsigned int subtrahend) {
	m_day -= subtrahend;
	while (m_day < 1) {
		int nCurrentMonthDays = IsLeapYear(m_year) ? LeapYearMonthDays[m_month - 1] : NormalYearMonthDays[m_month - 1];
		m_day += nCurrentMonthDays;
		DecreaseMonth();
	}
}

void Timestamp::DecreaseHour(unsigned int subtrahend) {
	m_hour -= subtrahend;
	while (m_hour < 0) {
		m_hour += 24;
		DecreaseDay();
	}
}

void Timestamp::DecreaseMinute(unsigned int subtrahend) {
	m_minute -= subtrahend;
	while (m_minute < 0) {
		m_minute += 60;
		DecreaseHour();
	}
}

void Timestamp::DecreaseSecond(unsigned int subtrahend) {
	m_second -= subtrahend;
	while (m_second < 0) {
		m_second += 60;
		DecreaseMinute();
	}
}

// ========================================================
// static function implementation

bool Timestamp::ParseTimestampFromString(const TCHAR* str, Timestamp* pTime) {
	return true;
}
