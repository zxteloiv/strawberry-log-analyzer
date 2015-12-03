// TimeRange.h
// Definition of the TimeRange class that is used to represent a range of time and
// supports the manipulation of add, subtract, multiplication and division.
// 

#pragma once

#include "UtilsDef.h"

namespace BaseUtils {
	class TimeRange {
		friend class Timestamp;
		// =======================================================
		// Interfaces

		// Constructors and destructors and assignment
	public:
		TimeRange();
		TimeRange(Real, Real, Real, Real, Real, Real);
		TimeRange(const TimeRange&);
		~TimeRange();
		
		TimeRange& operator=(const TimeRange& other);
		
		// Public interfaces
	public:
		Real GetYear() const { return m_year; }
		Real GetMonth() const { return m_month; }
		Real GetDay() const { return m_day; }
		Real GetHour() const { return m_hour; }
		Real GetMinute() const { return m_minute; }
		Real GetSecond() const { return m_second; }

		TimeRange operator+(const TimeRange& other) const;
		TimeRange& operator+=(const TimeRange& other);
		TimeRange operator-(const TimeRange& other) const;
		TimeRange& operator-=(const TimeRange& other);

		Timestamp operator+(const Timestamp& other);
		Timestamp operator-(const Timestamp& other);

		TimeRange operator*(int) const;
		TimeRange& operator*=(int);
		TimeRange operator*(Real) const;
		TimeRange& operator*=(Real);

		int operator/(const TimeRange& other) const;
		TimeRange operator%(const TimeRange& other) const;
		TimeRange& operator%=(const TimeRange& other);

		int ToDaysCount();
		int ToHoursCount();
		int ToMinutesCount();
		int ToSecondsCount();

		Timestamp	ToTimestamp();
		bool		ToTimestamp(Timestamp*);

		const TCHAR* ToString() const;

		// ======================================================
		// Data members
	public:
		Real	m_year;
		Real	m_month;
		Real	m_day;
		Real	m_hour;
		Real	m_minute;
		Real	m_second;

		TCHAR*	m_szOutputString;

		// ======================================================
		// Static members
	public:
		static bool ParseTimeRangeFromString(const TCHAR* str, TimeRange* pTimeRange);
	};
};
