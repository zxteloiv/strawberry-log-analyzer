// Range.cpp
// Contains some partial implementation for Range class
//

#include "stdafx.h"
#include "range.h"

#include "Aggregator.h"

using namespace ProcessCore;

Range::Range(AGGREGATE_TYPE agg_type, AGGREGATE_FUNC pFunc, Aggregator* pAgg) {
	m_rangeType = reserved_range;
	memset(&m_val, 0, sizeof(m_val));
	m_aggType = agg_type;
	m_pFuncAgg = pFunc;
	m_pRangeAgg = NULL;
}

Range::~Range() {
	if (m_pRangeAgg != NULL)
		delete m_pRangeAgg;
}

bool Range::SetRangeAggregator(Aggregator* pAgg) {
	if (m_pRangeAgg == NULL && pAgg != NULL) {
		m_pRangeAgg = pAgg;
		return true;
	}
	return false;
}

bool Range::ResetAggregateValue() {
	memset(&m_val, 0, sizeof(m_val));
	if (m_pRangeAgg != NULL) {
		m_pRangeAgg->ResetAggregateValue();
	}
	return true;
}
