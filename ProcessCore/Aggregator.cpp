// Aggregator.cpp
// Implements the Aggregator class
//

#include "stdafx.h"
#include "Aggregator.h"
#include "AggregatorManager.h"
#include "RangeFuncParam.h"

#include "range.h"
#include "IntRange.h"
#include "RealRange.h"
#include "StringRange.h"
#include "TimestampRange.h"
#include "questiontree.h"

#include "IRangeContainer.h"
#include "RangeHashset.h"
#include "RangeSet.h"

using namespace ProcessCore;

Aggregator::Aggregator(const TCHAR*				szBasefield,
					   LPCONDLIST				pCondList,
					   RANGE_FUNC				pRangeFunc,
					   RangeParam*				pRangeParam,
					   AGGREGATE_FUNC			pAggFunc,
					   AGGREGATE_TYPE			aggType,
					   const QuestionTreeNode*	pQTNode,
					   AggregatorManager*		pAggMgr,
					   ACCUMULATION_DIRECTION	direction)
					   : m_szBaseField(szBasefield), m_pConditionList(pCondList), m_pFuncRange(pRangeFunc),
					   m_pRangeParam(pRangeParam), m_pFuncAgg(pAggFunc), m_aggType(aggType),
					   m_pQuestionNode(pQTNode), m_pMgr(pAggMgr), m_direction(direction)
{
	m_pRangeContainer = NULL;
	// The range parameter structure is used as the single parameter passed to the external range
	// functions. So assign these members related to aggregate with those in the aggregator itself.
	pRangeParam->agg_func = m_pFuncAgg;
	pRangeParam->agg_type = m_aggType;
}

Aggregator::~Aggregator() {
	// Because all the members that are delcared const are all allocated outside this aggregator.
	// This aggregator shall not release those pointers on its own.
	// However, the pointer to its Range Container is allocated on the Aggregator own by the Init
	// function, therefore it has the responsibility to release it here.
	DELETE_POINTER(m_pRangeContainer);
}

bool Aggregator::Init(const QuestionTreeNode* pQTNode, AggregatorManager* pAggMgr) {
	if (m_direction == accu_none) {
		// If accumulation option for this aggregator is not switched on, use the Hashset as the RangeContainer
		// which doen't support to move iterator left or right as its measurement indicates.
		//RETURN_ON_FAIL(NULL != (m_pRangeContainer = new RangeHashset));
		RETURN_ON_FAIL(NULL != (m_pRangeContainer = new RangeSet));	// test use
	} else {
		// If the aggregator needs left or right accumulation, then use the RangeSet as the RangeContainer,
		// whose range iterator can be used to get all other larger or less ranges.
		RETURN_ON_FAIL(NULL != (m_pRangeContainer = new RangeSet));
	}
	if (pQTNode != NULL)
		m_pQuestionNode = pQTNode;
	if (pAggMgr != NULL)
		m_pMgr = pAggMgr;
	return true;
}

bool Aggregator::ReceiveRecord(const BaseUtils::_Record_Ptr pRecord) {
	// After receiving a record, the aggregator first check for condition fields if any
	bool record_accepted = false;
	RETURN_ON_FAIL(CheckForCondition(pRecord, &record_accepted));	// errors occured when performing condition check
	if (!record_accepted) {	// record is not satisfied with all the conditions, so it was ignored
		return true;		// function completed while record is dismissed, but it doesn't mean an execution failure
	}

	// After condition check, extract the base field for that record.
	BaseUtils::Field*	pBaseField;
	RETURN_ON_FAIL(ExtractBaseField(pRecord, &pBaseField) && pBaseField != NULL);	// base field cannot be extracted successfully.
	
	// Get Range
	Range* pRange = NULL;
	RETURN_ON_FAIL(DistributeBaseField(pBaseField, &pRange));

	// Find or add range to the right place
	PROCESS_ERROR(m_pRangeContainer != NULL);
	bool inserted = m_pRangeContainer->FindOrInsertRange(pRange);
	if (m_pQuestionNode->m_pRangeAggNode != NULL && inserted) {
		// This range can be inserted successfully if and only if there's no same range, for example,
		// an integer range that has the same upper and lower bounds, already in the range container
		// of this aggregator.
		// Then, under this situation, a new object of range aggregator must be initialized if the
		// range claims one.
		Aggregator*	pNewAgg = NULL;
		PROCESS_ERROR(m_pMgr->CreateAggregator(&pNewAgg, m_pQuestionNode->m_pRangeAggNode));
		pRange->SetRangeAggregator(pNewAgg);
	}

	// Aggregate
	PROCESS_ERROR(pRange->DoAggregate(pBaseField));	// this will call the AGG_FUNC pointer, a data member of pRange
	if (m_direction == accu_left) {
		while (NULL != (pRange = m_pRangeContainer->FindLeft(pRange))) {
			pRange->DoAggregate(pBaseField);
		}
	} else if (m_direction == accu_right) {
		while (NULL != (pRange = m_pRangeContainer->FindRight(pRange))) {
			pRange->DoAggregate(pBaseField);
		}
	}

	return true;
Exit0:
	DELETE_POINTER(pRange);
	return false;
}

bool Aggregator::CheckForCondition(const BaseUtils::_Record_Ptr pRecord, bool* pIsAccepted) {
	using namespace BaseUtils;
	*pIsAccepted = false;

	RETURN_ON_FAIL(m_pConditionList != NULL);	// NULL pointer means error, while valid pointer with zero elements is acceptable.
	for (CONSTCONDITER iter = m_pConditionList->begin(); iter != m_pConditionList->end(); ++iter) {
		const FIELD_CONDITION* pCond = *iter;
		RETURN_ON_FAIL(pCond != NULL);	// condition shall not be NULL

		// Get field for this condition
		_Field_Ptr pField = NULL;
		pField = pRecord->GetField(pCond->field_id);
		RETURN_ON_FAIL(pField != NULL);	// condition may has error

		if (pCond->pCondFunc(pField, pCond->params) != pCond->rtn_val) {
			// False value is reserved for only this funcion has something wrong, e.g. NULL pointers.
			// In the Aggregator design, a record will be accepted only when it satisfies all the conditions.
			// When it didn't, it will be dismissed by the Aggregator, which, however, doesn't mean the function
			// has any error.
			*pIsAccepted = false;
			return true;	// return true, and then this record was not processed.
		}
	}

	*pIsAccepted = true;	// the record passes all the test
	return true;
}

bool Aggregator::ExtractBaseField(const BaseUtils::_Record_Ptr pRecord, BaseUtils::Field **ppBaseField) {
	RETURN_ON_FAIL(pRecord != NULL);
	*ppBaseField = pRecord->GetField(m_szBaseField);
	return (NULL != (*ppBaseField));
}

bool Aggregator::DistributeBaseField(const BaseUtils::_Field_Ptr pField, Range** ppRange) {
	using namespace BaseUtils;

	RETURN_ON_FAIL(pField != NULL && m_pRangeParam != NULL && ppRange != NULL);
	*ppRange = NULL;

	// Because currently we are not sure about whether this range is already exists in the Range Conainter,
	// it will be set NULL by default. And later after inserting this range to container and according its
	// returning value, we will be sure whether this range will be given a new range aggregator.
	Aggregator* pRangeAgg = NULL;

	if (m_pFuncRange != NULL) {	// range function was predefined already
		// When a range function was defined already, the parameters must exist.
		RETURN_ON_FAIL(m_pFuncRange(pField, m_pRangeParam, pRangeAgg, ppRange));
	} else {	// no range function defined, just allocate a default range
		switch (pField->GetType()) {
			case BaseUtils::Field::integer:
				*ppRange = new IntRange(0, 0, m_pRangeParam->agg_type, m_pRangeParam->agg_func, pRangeAgg);
				break;
			case BaseUtils::Field::real:
				*ppRange = new RealRange(0.0f, 0.0f, m_pRangeParam->agg_type, m_pRangeParam->agg_func, pRangeAgg);
				break;
			case BaseUtils::Field::string:
				*ppRange = new StringRange(false, false, m_pRangeParam->agg_type, 0, m_pRangeParam->agg_func, pRangeAgg);
				break;
			case BaseUtils::Field::timestamp:
				*ppRange = new TimestampRange(Timestamp(), Timestamp(), m_pRangeParam->agg_type, m_pRangeParam->agg_func, pRangeAgg);
				break;
			default:
				return false;
		}
		RETURN_ON_FAIL(*ppRange != NULL);
	}

	// do something
	return true;
}

bool Aggregator::ResetAggregateValue() {
	RETURN_ON_FAIL(NULL != m_pRangeContainer);
	
	RangeSet* pRangeSet = NULL;
	RangeHashset* pRangeHashset = NULL;
	pRangeSet = dynamic_cast<RangeSet*>(m_pRangeContainer);
	pRangeHashset = dynamic_cast<RangeHashset*>(m_pRangeContainer);
	if (pRangeSet != NULL) {	// the range container is a range set
		RangeSet::iterator iterRange = pRangeSet->begin();
		RangeSet::iterator iterLastRange = pRangeSet->end();
		for (; iterRange != iterLastRange; ++iterRange) {
			Range* pRange = (*iterRange);
			RETURN_ON_FAIL(pRange->ResetAggregateValue());
		}
	} else if (pRangeHashset != NULL) {	// the range container is a range hashset
		RangeHashset::iterator iterRange = pRangeHashset->begin();
		RangeHashset::iterator iterLastRange = pRangeHashset->end();
		for (; iterRange != iterLastRange; ++iterRange) {
			Range* pRange = (*iterRange);
			RETURN_ON_FAIL(pRange->ResetAggregateValue());
		}
	} else
		// the range container is neither a range set nor a hashset, currently it's impossible,
		// but when it occurs that the range container interface cannot be converted to any of
		// the two, it can leads this fault.
		RETURN_ON_FAIL(false);

	return true;
}

