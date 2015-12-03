// IntRange.cpp
// contains implementations for the IntRange class, which is derived from the Range abstract class.
//

#include "stdafx.h"
#include "IntRange.h"
#include "aggregator.h"

using namespace ProcessCore;

IntRange::IntRange(int nLowerBound,
				   int nUpperBound,
				   AGGREGATE_TYPE	agg_type,
				   AGGREGATE_FUNC	pFunc,
				   Aggregator*		pAgg)
				   : Range(agg_type, pFunc, pAgg) {
	m_rangeType = Range::integer_range;
	m_nLowerBound = nLowerBound;
	m_nUpperBound = nUpperBound;
}

IntRange::~IntRange() {
}

CompRangeReturn IntRange::IsLessThan(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(integer_range != pOther->GetRangeType());
	
	IntRange* pIntRange = dynamic_cast<IntRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pIntRange == NULL);

	RETURN_YES_WHEN(m_nUpperBound <= pIntRange->m_nLowerBound);

	return CompNo;
}

CompRangeReturn IntRange::EqualsTo(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(integer_range != pOther->GetRangeType());
	
	IntRange* pIntRange = dynamic_cast<IntRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pIntRange == NULL);

	RETURN_YES_WHEN(m_nLowerBound == pIntRange->m_nLowerBound &&
		m_nUpperBound == pIntRange->m_nUpperBound);

	return CompNo;
}

bool IntRange::GetRangeAggValue(void** ppValue) {
	RETURN_ON_FAIL(m_pFuncAgg != NULL && ppValue != NULL);		// no aggregate value when no aggregate function designated
	int **ppIntValue = NULL;
	switch (m_aggType) {
		// for an integer range, the max/min value and the count number are all integers
		case AGG_COUNT:
		case AGG_MAX:
		case AGG_MIN:
			ppIntValue = reinterpret_cast<int**>(ppValue);
			*ppIntValue = &(m_val.int_value);
			break;
		default:
			return false;
	}
	return true;
}

bool IntRange::DoAggregate(BaseUtils::_Field_Ptr pBaseField) {
	RETURN_ON_FAIL(m_pFuncAgg != NULL);
	RETURN_ON_FAIL(m_pFuncAgg(pBaseField, &m_val));
	if (m_pRangeAgg != NULL) {
		RETURN_ON_FAIL(m_pRangeAgg->ReceiveRecord(pBaseField->GetRecordBelongedTo()));
	}
	return true;
};

bool IntRange::operator <(ProcessCore::IntRange &other) const {
	return (m_nUpperBound <= other.m_nLowerBound);
}

bool IntRange::operator ==(ProcessCore::IntRange &other) const {
	return (m_nUpperBound == other.m_nUpperBound &&
		m_nLowerBound == other.m_nLowerBound);
}

bool IntRange::operator >(ProcessCore::IntRange &other) const {
	return (other.m_nUpperBound <= m_nLowerBound);
}
