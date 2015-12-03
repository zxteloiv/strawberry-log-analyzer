// RealRange.h
// Declaration for RealRange class
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"
#include "range.h"

namespace ProcessCore {
	class RealRange : public Range {
		// =======================================================
		// Interfaces

		// Constructors and destructors
	public:
		RealRange(BaseUtils::Real, BaseUtils::Real, AGGREGATE_TYPE agg_type,
			AGGREGATE_FUNC pFunc = NULL, Aggregator* pAgg = NULL);
		virtual ~RealRange();

		// Public interfaces
	public:
		virtual CompRangeReturn IsLessThan(_Range_Ptr pOther) const;
		virtual CompRangeReturn EqualsTo(_Range_Ptr pOther) const;
		
		bool operator<(RealRange& other);
		bool operator==(RealRange& other);
		bool operator>(RealRange& other);

		BaseUtils::Real GetUpperBound() { return m_rUpperBound; }
		BaseUtils::Real GetLowerBound() { return m_rLowerBound; }

		virtual bool GetRangeAggValue(void**);
		virtual bool DoAggregate(BaseUtils::_Field_Ptr);

		// ====================================================
		// Data members
	protected:
		BaseUtils::Real	m_rLowerBound;
		BaseUtils::Real	m_rUpperBound;
	};
};
