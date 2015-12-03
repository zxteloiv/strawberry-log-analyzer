// RealRange.cpp
// Implements the RealRange class, as long as some pure virtual functions in its
// base class.
//

#include "stdafx.h"
#include "RealRange.h"
#include "aggregator.h"

using namespace ProcessCore;
using namespace BaseUtils;

RealRange::RealRange(Real lower_bound,
					 Real upper_bound,
					 AGGREGATE_TYPE	agg_type,
					 AGGREGATE_FUNC	pFunc,
					 Aggregator*	pAgg)
					 : Range(agg_type, pFunc, pAgg) {
	m_rLowerBound = lower_bound;
	m_rUpperBound = upper_bound;
	m_rangeType = real_range;
}

RealRange::~RealRange() {
}

CompRangeReturn RealRange::IsLessThan(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(real_range != pOther->GetRangeType());

	RealRange* pRealRange = dynamic_cast<RealRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pRealRange == NULL);

	RETURN_YES_WHEN(m_rUpperBound <= pRealRange->m_rLowerBound);

	return CompNo;
}

CompRangeReturn RealRange::EqualsTo(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(real_range != pOther->GetRangeType());

	RealRange* pRealRange = dynamic_cast<RealRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pRealRange == NULL);

	RETURN_YES_WHEN(m_rLowerBound == pRealRange->m_rLowerBound &&
		m_rUpperBound == pRealRange->m_rUpperBound);

	return CompNo;
}

bool RealRange::operator <(ProcessCore::RealRange &other) {
	return (m_rUpperBound <= other.m_rLowerBound);
}

bool RealRange::operator ==(ProcessCore::RealRange &other) {
	return (m_rUpperBound == other.m_rUpperBound &&
		m_rLowerBound == other.m_rLowerBound);
}

bool RealRange::operator >(ProcessCore::RealRange &other) {
	return (other.m_rUpperBound <= m_rLowerBound);
}

bool RealRange::GetRangeAggValue(void** ppValue) {
	RETURN_ON_FAIL(ppValue != NULL && m_pFuncAgg != NULL);
	int** ppIntValue = NULL;
	Real** ppRealValue = NULL;
	switch (m_aggType) {
		case AGG_COUNT:
			ppIntValue = reinterpret_cast<int**>(ppValue);
			*ppIntValue = &(m_val.int_value);
			break;
		case AGG_MAX:
		case AGG_MIN:
			ppRealValue = reinterpret_cast<Real**>(ppValue);
			*ppRealValue = &(m_val.real_value);
			break;
		default:
			return false;
	};
	return true;
}

bool RealRange::DoAggregate(BaseUtils::_Field_Ptr pBaseField) {
	RETURN_ON_FAIL(m_pFuncAgg != NULL);
	RETURN_ON_FAIL(m_pFuncAgg(pBaseField, &m_val));
	if (m_pRangeAgg != NULL) {
		RETURN_ON_FAIL(m_pRangeAgg->ReceiveRecord(pBaseField->GetRecordBelongedTo()));
	}
	return true;
}


