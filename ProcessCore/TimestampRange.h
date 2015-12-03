// TimestampRange.h
// Declaration for TimestampRange class.
//

#pragma once

#include "range.h"
#include "..\\BaseUtils\\BaseUtils.h"

namespace ProcessCore {
	class TimestampRange : public Range {
		// =======================================================
		// Interfaces

		// Constructors and destructors
	public:
		TimestampRange(const BaseUtils::Timestamp&,
			const BaseUtils::Timestamp&,
			AGGREGATE_TYPE agg_type,
			AGGREGATE_FUNC pFunc = NULL,
			Aggregator* pAgg = NULL);

		virtual ~TimestampRange();

		// Public interfaces
	public:
		virtual CompRangeReturn IsLessThan(_Range_Ptr pOther) const;
		virtual CompRangeReturn EqualsTo(_Range_Ptr pOther) const;
		
		bool operator<(TimestampRange& other);
		bool operator==(TimestampRange& other);
		bool operator>(TimestampRange& other);

		bool GetUpperBound(BaseUtils::Timestamp*);
		bool GetLowerBound(BaseUtils::Timestamp*);

		virtual bool GetRangeAggValue(void**);
		virtual bool DoAggregate(BaseUtils::_Field_Ptr);

		// ====================================================
		// Data members
	protected:
		BaseUtils::Timestamp	m_tLowerBound;
		BaseUtils::Timestamp	m_tUpperBound;
	};
};
