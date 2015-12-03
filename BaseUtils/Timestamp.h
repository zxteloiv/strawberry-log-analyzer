// Timestamp.h
// Declaration for the timestamp class, whose instance stands for a fixed date and time
//

#pragma once

#include "UtilsDef.h"

namespace BaseUtils {

	class Timestamp {
		friend class TimeRange;
		// =======================================================
		// Interfaces

		// Constructors and destructors
	public:
		Timestamp();
		Timestamp(int year, int mon, int day, int hour, int min, int sec);
		Timestamp(const Timestamp&);
		virtual ~Timestamp();
		Timestamp& operator=(const Timestamp&);

		// Public interfaces
	public:
		bool operator<(const Timestamp&) const;
		bool operator==(const Timestamp&) const;
		bool operator>(const Timestamp&) const;
		bool operator<=(const Timestamp&) const;
		bool operator>=(const Timestamp&) const;
		bool operator!=(const Timestamp&) const;

		Timestamp operator+(const TimeRange&) const;
		Timestamp& operator+=(const TimeRange&);
		Timestamp operator-(const TimeRange&) const;
		Timestamp& operator-=(const TimeRange&);
		TimeRange operator-(const Timestamp&) const;

		int		GetYear() const { return m_year; }
		int		GetMonth() const { return m_month; }
		int		GetDay() const { return m_day; }
		int		GetHour() const { return m_hour; }
		int		GetMinute() const { return m_minute; }
		int 	GetSecond() const { return m_second; }

		TimeRange	ToTimeRange();
		bool		ToTimeRange(TimeRange*);
		const TCHAR*	ToString() const;

		// help functions
	protected:
		void	IncreaseYear(unsigned int addend = 1);
		void	IncreaseMonth(unsigned int addend = 1);
		void	IncreaseDay(unsigned int addend = 1);
		void	IncreaseHour(unsigned int addend = 1);
		void	IncreaseMinute(unsigned int addend = 1);
		void	IncreaseSecond(unsigned int addend = 1);

		void	DecreaseYear(unsigned int subtrahend = 1);
		void	DecreaseMonth(unsigned int subtrahend = 1);
		void	DecreaseDay(unsigned int subtrahend = 1);
		void	DecreaseHour(unsigned int subtrahend = 1);
		void	DecreaseMinute(unsigned int subtrahend = 1);
		void	DecreaseSecond(unsigned int subtrahend = 1);

		// =======================================================
		// Data members
	protected:
		// although int is too large for date and time here, it is important to ensure the value
		// overflow when we're doing arithmatic operations on this class.
		int		m_year;
		int		m_month;
		int		m_day;		
		int		m_hour;	
		int		m_minute;	
		int		m_second;

		TCHAR*	m_szOutputString;

		// =======================================================
		// Static function
	public:
		static bool ParseTimestampFromString(const TCHAR* str, Timestamp* pTime);

		// =======================================================
		// Static Members
	protected:
		static const int LeapYearMonthDays[12];
		static const int NormalYearMonthDays[12];

	};
};
