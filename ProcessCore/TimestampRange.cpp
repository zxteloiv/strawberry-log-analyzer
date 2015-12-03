// TimestampRange.cpp
// Implements the TimestampRange class, together with other functions derived from the Range base class.
//

#include "stdafx.h"
#include "TimestampRange.h"
#include "aggregator.h"

using namespace ProcessCore;
using namespace BaseUtils;

TimestampRange::TimestampRange(const Timestamp& lower_bound,
							   const Timestamp& upper_bound,
							   AGGREGATE_TYPE	agg_type,
							   AGGREGATE_FUNC	pFunc,
							   Aggregator*		pAgg)
							   : Range(agg_type, pFunc, pAgg)
{
	m_rangeType = timestamp_range;
	m_tLowerBound = lower_bound;
	m_tUpperBound = upper_bound;
}

TimestampRange::~TimestampRange() {
	// For the following two Aggregate type, clean is needed.
	if (m_aggType == AGG_MAX || m_aggType == AGG_MIN) {
		if (m_val.timestamp_ptr != NULL)
			delete m_val.timestamp_ptr;
	}
}

CompRangeReturn TimestampRange::IsLessThan(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(m_rangeType != pOther->GetRangeType());

	TimestampRange* pTimeRange = dynamic_cast<TimestampRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pTimeRange == NULL);

	RETURN_YES_WHEN(m_tUpperBound <= pTimeRange->m_tLowerBound);

	return CompNo;
}

CompRangeReturn TimestampRange::EqualsTo(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(m_rangeType != pOther->GetRangeType());

	TimestampRange* pTimeRange = dynamic_cast<TimestampRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pTimeRange == NULL);

	RETURN_YES_WHEN(m_tLowerBound == pTimeRange->m_tLowerBound &&
		m_tUpperBound == pTimeRange->m_tUpperBound);

	return CompNo;
}

bool TimestampRange::operator <(ProcessCore::TimestampRange &other) {
	return (m_tUpperBound <= other.m_tLowerBound);
}

bool TimestampRange::operator ==(ProcessCore::TimestampRange &other) {
	return (m_tLowerBound == other.m_tLowerBound &&
		m_tUpperBound == other.m_tUpperBound);
}

bool TimestampRange::operator >(ProcessCore::TimestampRange &other) {
	return (other.m_tUpperBound <= m_tLowerBound);
}

bool TimestampRange::GetUpperBound(BaseUtils::Timestamp* pTime) {
	*pTime = m_tLowerBound;
	return true;
}

bool TimestampRange::GetLowerBound(BaseUtils::Timestamp* pTime) {
	*pTime = m_tUpperBound;
	return true;
}

bool TimestampRange::DoAggregate(BaseUtils::_Field_Ptr pBaseField) {
	RETURN_ON_FAIL(m_pFuncAgg != NULL);
	RETURN_ON_FAIL(m_pFuncAgg(pBaseField, &m_val));
	if (m_pRangeAgg != NULL) {
		RETURN_ON_FAIL(m_pRangeAgg->ReceiveRecord(pBaseField->GetRecordBelongedTo()));
	}
	return true;
}

bool TimestampRange::GetRangeAggValue(void** ppValue) {
	RETURN_ON_FAIL(ppValue != NULL && m_pFuncAgg != NULL);
	int** ppIntValue = NULL;
	Timestamp**	ppTimeValue = NULL;
	switch (m_aggType) {
		case AGG_COUNT:
			ppIntValue = reinterpret_cast<int**>(ppValue);
			*ppIntValue = &(m_val.int_value);
			break;
		case AGG_MAX:
		case AGG_MIN:
			ppTimeValue = reinterpret_cast<Timestamp**>(ppValue);
			*ppTimeValue = m_val.timestamp_ptr;
			break;
	}
	return true;
}


