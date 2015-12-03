// IntRange.h
// Declaration for IntRange class.
//

#pragma once

#include "range.h"

namespace ProcessCore {
	class IntRange : public Range {
		// ====================================================
		// Interfaces
		
		// Constructors and destructors
	public:
		IntRange(int, int, AGGREGATE_TYPE agg_type,
			AGGREGATE_FUNC pFunc = NULL, Aggregator* pAgg = NULL);
		virtual ~IntRange();

		// Public interfaces
	public:
		virtual CompRangeReturn IsLessThan(_Range_Ptr pOther) const;
		virtual CompRangeReturn EqualsTo(_Range_Ptr pOther) const;
		
		bool operator<(IntRange& other) const;
		bool operator==(IntRange& other) const;
		bool operator>(IntRange& other) const;

		int	GetUpperBound() const { return m_nUpperBound; }
		int	GetLowerBound() const { return m_nLowerBound; }

		virtual bool GetRangeAggValue(void**);
		virtual bool DoAggregate(BaseUtils::_Field_Ptr);

		// ====================================================
		// Data members
	protected:
		int		m_nLowerBound;
		int		m_nUpperBound;
	};
};
