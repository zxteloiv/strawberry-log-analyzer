// StringRange.h
// Declaration for StringRange class.
//

#pragma once

#include "range.h"

namespace ProcessCore {
	class StringRange : public Range {
		// =======================================================
		// Interfaces

		// Constructors and destructors
	public:
		StringRange(bool case_sensitive,
			bool reversed_comp,
			AGGREGATE_TYPE agg_type,
			int char_to_comp = COMPARE_ENTIRE_STRING,
			AGGREGATE_FUNC pFunc = NULL,
			Aggregator* pAgg = NULL);
		virtual ~StringRange();

		// Public interfaces
	public:
		virtual CompRangeReturn IsLessThan(_Range_Ptr pOther) const;
		virtual CompRangeReturn EqualsTo(_Range_Ptr pOther) const;
		
		bool operator<(StringRange& other);
		bool operator==(StringRange& other);
		bool operator>(StringRange& other);

		virtual bool GetRangeAggValue(void** ppValue);
		virtual bool DoAggregate(BaseUtils::_Field_Ptr);

		bool SetString(const TCHAR*);
		const TCHAR* GetContentString() const { return m_szContent; }

		// ========================================================
		// Data members
	protected:
		int		m_nCompCharsCount;
		bool	m_bCaseSensitive;
		bool	m_bReversedComp;
		TCHAR*	m_szContent;	// content is the string that represents this StringRange

		// ========================================================
		// Static members
	public:
		// If the number of characters to compare in any StringRange object is set to
		// this value, then the whole string is taken into comparison.
		static const int COMPARE_ENTIRE_STRING = -1;

		// help functions
	public:
		// Compare two case-insensitive strings inversely
		static int _tcscmp_reversed(const TCHAR*, const TCHAR*);
		// Compare two case-insensitive strings inversely
		static int _tcsicmp_reversed(const TCHAR*, const TCHAR*);
		// Compare two case-sensitive strings for some charcters only inversely
		static int _tcsncmp_reversed(const TCHAR*, const TCHAR*, int);
		// Compare two cse-sinsensitive strings for parts of characters only inversely
		static int _tcsnicmp_reversed(const TCHAR*, const TCHAR*, int);

	};
};
