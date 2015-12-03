// ExternalRoutes.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#ifndef EXTERNALROUTES_EXPORTS
#define EXTERNALROUTES_EXPORTS
#endif

#include "ExternalRoutes.h"

#include <math.h>
#include <errno.h>
#include <ctype.h>

using namespace BaseUtils;
using namespace ProcessCore;

#pragma warning(disable: 4244)	// disable warning for converting double to float

// --------------------------------------------------------------------------------------------------
// Implements exported C functions
extern "C" {
	EXTERNALROUTES_API bool WINAPI AggFuncCount(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue) {
		return AggregateFunctions::Count(pField, pValue);
	}
	EXTERNALROUTES_API bool WINAPI AggFuncMax(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue) {
		return AggregateFunctions::Max(pField, pValue);
	}
	EXTERNALROUTES_API bool WINAPI AggFuncMin(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue) {
		return AggregateFunctions::Min(pField, pValue);
	}
};

extern "C" {
	EXTERNALROUTES_API bool WINAPI RangeFuncInt(const BaseUtils::_Field_Ptr		pField,
												const ProcessCore::RangeParam*	pRangeParam,
												ProcessCore::Aggregator*		pAgg,
												ProcessCore::Range**			ppRange)
	{
		return RangeFunctions::RangeForInt(pField, pRangeParam, pAgg, ppRange);
	}

	EXTERNALROUTES_API bool WINAPI RangeFuncReal(const BaseUtils::_Field_Ptr	pField,
												const ProcessCore::RangeParam*	pRangeParam,
												ProcessCore::Aggregator*		pAgg,
												ProcessCore::Range**			ppRange)
	{
		return RangeFunctions::RangeForReal(pField, pRangeParam, pAgg, ppRange);
	}

	EXTERNALROUTES_API bool WINAPI RangeFuncTime(const BaseUtils::_Field_Ptr	pField,
												const ProcessCore::RangeParam*	pRangeParam,
												ProcessCore::Aggregator*		pAgg,
												ProcessCore::Range**			ppRange)
	{
		return RangeFunctions::RangeForTime(pField, pRangeParam, pAgg, ppRange);
	}

	EXTERNALROUTES_API bool WINAPI RangeFuncString(const BaseUtils::_Field_Ptr	pField,
												const ProcessCore::RangeParam*	pRangeParam,
												ProcessCore::Aggregator*		pAgg,
												ProcessCore::Range**			ppRange)
	{
		return RangeFunctions::RangeForString(pField, pRangeParam, pAgg, ppRange);
	}
};

extern "C" {
	EXTERNALROUTES_API bool WINAPI IntLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::IntLessThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI IntEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::IntEqualTo(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI IntLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::IntLargerThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI RealLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::RealLessThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI RealEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::RealEqualTo(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI RealLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::RealLargerThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI TimeLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::TimeLessThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI TimeEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::TimeEqualTo(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI TimeLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::TimeLargerThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI StringIsEmpty(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::StringIsEmpty(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI StringLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::StringLessThan(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI StringEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::StringEqualTo(pField, param);
	}

	EXTERNALROUTES_API bool WINAPI StringLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
		return ConditionFunctions::StringLargerThan(pField, param);
	}

};

// -------------------------------------------------------------------------------------------------
// Implement where exported functions are really implemented
bool WINAPI AggregateFunctions::Count(_Field_Ptr pField, AggregateValue *pValue) {
	RETURN_ON_FAIL(pValue != NULL);
	RETURN_ON_FAIL(pField != NULL);
	
	pValue->int_value++;
	return true;
}

bool WINAPI AggregateFunctions::Min(_Field_Ptr pField, AggregateValue *pValue) {
	switch (pField->GetType()) {
		case BaseUtils::Field::integer: {
			IntField* pIntField = dynamic_cast<IntField*>(pField);
			RETURN_ON_FAIL(pIntField != NULL);
			int nIntVal = pIntField->GetValueInt();
			if (nIntVal < pValue->int_value)
				pValue->int_value = nIntVal;
										}
										break;
		case BaseUtils::Field::real: {
			RealField* pRealField = dynamic_cast<RealField*>(pField);
			RETURN_ON_FAIL(pRealField != NULL);
			Real rRealVal = pRealField->GetValueReal();
			if (rRealVal < pValue->real_value)
				pValue->real_value = rRealVal;
									 }
									 break;
		case BaseUtils::Field::string: {
			return false;	// currently string
									   }
									   break;
		case BaseUtils::Field::timestamp: {
			TimestampField* pTimeField = dynamic_cast<TimestampField*>(pField);
			RETURN_ON_FAIL(pTimeField != NULL);
			const Timestamp* pTime = pTimeField->GetPointerTime();
			if ((*pTime) < (*(pValue->timestamp_ptr)))
				pValue->timestamp_ptr->operator =(*pTime);
										  }
										  break;
		default:
			return false;
	}
	return true;
}

bool WINAPI AggregateFunctions::Max(_Field_Ptr pField, AggregateValue *pValue) {
	switch (pField->GetType()) {
		case BaseUtils::Field::integer: {
			IntField* pIntField = dynamic_cast<IntField*>(pField);
			RETURN_ON_FAIL(pIntField != NULL);
			int nIntVal = pIntField->GetValueInt();
			if (nIntVal > pValue->int_value)
				pValue->int_value = nIntVal;
										}
										break;
		case BaseUtils::Field::real: {
			RealField* pRealField = dynamic_cast<RealField*>(pField);
			RETURN_ON_FAIL(pRealField != NULL);
			Real rRealVal = pRealField->GetValueReal();
			if (rRealVal > pValue->real_value)
				pValue->real_value = rRealVal;
									 }
									 break;
		case BaseUtils::Field::string: {
			return false;	// currently string compare is not supported
									   }
									   break;
		case BaseUtils::Field::timestamp: {
			TimestampField* pTimeField = dynamic_cast<TimestampField*>(pField);
			RETURN_ON_FAIL(pTimeField != NULL);
			const Timestamp* pTime = pTimeField->GetPointerTime();
			if ((*pTime) > (*(pValue->timestamp_ptr)))
				pValue->timestamp_ptr->operator =(*pTime);
										  }
										  break;
		default:
			return false;
	}
	return true;
}

bool WINAPI RangeFunctions::RangeForInt(const _Field_Ptr pBaseField,
										const RangeParam* pRangeParam,
										ProcessCore::Aggregator* pRangeAgg,
										Range** ppRange)
{
	RETURN_ON_FAIL(pBaseField != NULL && pBaseField->GetType() == BaseUtils::Field::integer && pRangeParam != NULL
		&& ppRange != NULL);

	const IntRangeParam* pIntRangeParam = static_cast<const IntRangeParam*>(pRangeParam);
	RETURN_ON_FAIL(pIntRangeParam != NULL);

	const IntField* pIntField = dynamic_cast<const IntField*>(pBaseField);
	RETURN_ON_FAIL(pIntField != NULL);

	int nVal = pIntField->GetValueInt();

	int nAnchor = pIntRangeParam->anchor;
	int nWidth = pIntRangeParam->width;
	RETURN_ON_FAIL(nWidth > 0);
	int nLowerBound = 0;
	int nUpperBound = 0;

	// Give the number value, anchor value and the range width value, calculate out the upperbound and lower-bound
	if (nVal >= nAnchor) {
		int offset = (nVal - nAnchor) / nWidth;
		nLowerBound = nAnchor + offset * nWidth;
		nUpperBound = nLowerBound + nWidth;
	} else {
		int offset = (nAnchor - nVal) / nWidth;
		nLowerBound = nAnchor - offset * nWidth;
		nUpperBound = nLowerBound - nWidth;
	}

	*ppRange = new IntRange(nLowerBound, nUpperBound, pIntRangeParam->agg_type, pIntRangeParam->agg_func, pRangeAgg);
	RETURN_ON_FAIL(ppRange != NULL);
	return true;
}

bool WINAPI RangeFunctions::RangeForReal(const _Field_Ptr pBaseField,
										 const RangeParam* pRangeParam,
										 ProcessCore::Aggregator* pRangeAgg,
										 Range** ppRange)
{
	RETURN_ON_FAIL(pBaseField != NULL && pBaseField->GetType() == BaseUtils::Field::real && pRangeParam != NULL
		&& ppRange != NULL);

	const RealRangeParam* pRealRangeParam = static_cast<const RealRangeParam*>(pRangeParam);
	RETURN_ON_FAIL(pRealRangeParam != NULL);

	const RealField* pRealField = dynamic_cast<const RealField*>(pBaseField);
	RETURN_ON_FAIL(pRealField != NULL);

	Real rVal = pRealField->GetValueReal();

	Real rAnchor = pRealRangeParam->anchor;
	Real rWidth = pRealRangeParam->width;
	RETURN_ON_FAIL(rWidth > 0);
	Real rLowerBound = 0;
	Real rUpperBound = 0;

	if (rVal >= rAnchor) {
		Real offset = (rVal - rAnchor) / rWidth;
		int nOffset = (int)offset;
		rLowerBound = rAnchor + nOffset * rWidth;
		rUpperBound = rLowerBound + rWidth;
	} else {
		Real offset = (rAnchor - rVal) / rWidth;
		int nOffset = (int)offset;
		rLowerBound = rAnchor - offset * rWidth;
		rUpperBound = rLowerBound - rWidth;
	}

	*ppRange = new RealRange(rLowerBound, rUpperBound, pRealRangeParam->agg_type, pRealRangeParam->agg_func, pRangeAgg);
	return true;
}

bool WINAPI RangeFunctions::RangeForTime(const _Field_Ptr pBaseField,
										 const RangeParam* pRangeParam,
										 ProcessCore::Aggregator* pRangeAgg,
										 Range** ppRange)
{
	RETURN_ON_FAIL(pBaseField != NULL && pBaseField->GetType() == BaseUtils::Field::timestamp && pRangeParam != NULL
		&& ppRange != NULL);

	const TimestampRangeParam* pTimeRangeParam = static_cast<const TimestampRangeParam*>(pRangeParam);
	RETURN_ON_FAIL(pTimeRangeParam != NULL);

	const TimestampField* pTimeField = dynamic_cast<const TimestampField*>(pBaseField);
	RETURN_ON_FAIL(pTimeField != NULL);

	const Timestamp* pTimeVal = pTimeField->GetPointerTime();
	const Timestamp* pAnchor = &(pTimeRangeParam->anchor);
	const TimeRange* pWidth = &(pTimeRangeParam->width);
	Timestamp tUpperBound;
	Timestamp tLowerBound;
	if (*pTimeVal >= *pAnchor) {
		Real offset = ((*pAnchor) - (*pTimeVal)) / (*pWidth);
		int nOffset = (int)offset;
		tLowerBound = (*pAnchor) + ((*pWidth) * nOffset);
		tUpperBound = tLowerBound + *pWidth;
	} else {
		Real offset = ((*pTimeVal) - (*pAnchor)) / (*pWidth);
		int nOffset = (int)offset;
		tLowerBound = (*pAnchor) - ((*pWidth) * nOffset);
		tUpperBound = tLowerBound - (*pWidth);
	}

	*ppRange = new TimestampRange(tLowerBound, tUpperBound, pTimeRangeParam->agg_type, pTimeRangeParam->agg_func, pRangeAgg);
	return true;
}

bool WINAPI RangeFunctions::RangeForString(const _Field_Ptr pBaseField,
										   const RangeParam* pRangeParam,
										   ProcessCore::Aggregator* pRangeAgg,
										   Range** ppRange)
{
	RETURN_ON_FAIL(pBaseField != NULL && pBaseField->GetType() == BaseUtils::Field::string && pRangeParam != NULL
		&& ppRange != NULL);

	const StringRangeParam* pStringRangeParam = static_cast<const StringRangeParam*>(pRangeParam);
	RETURN_ON_FAIL(pStringRangeParam != NULL);

	const StringField* pStringField = dynamic_cast<const StringField*>(pBaseField);
	RETURN_ON_FAIL(pStringField != NULL);

	StringRange* pStringRange = new StringRange(pStringRangeParam->case_sensitive, pStringRangeParam->reversed_comp,
		pStringRangeParam->agg_type, pStringRangeParam->char_num_to_comp, pStringRangeParam->agg_func, pRangeAgg);
	RETURN_ON_FAIL(pStringRange != NULL);

	if (!pStringRange->SetString(pStringField->GetValueString())) {
		delete pStringRange;
		return false;
	}

	*ppRange = pStringRange;

	return true;
}

bool WINAPI ConditionFunctions::IntLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::integer);
	const IntField* pIntField = NULL;
	pIntField = dynamic_cast<const IntField*>(pField);
	RETURN_ON_FAIL(NULL != pIntField);
	int nFieldValue = pIntField->GetValueInt();

	// a wrong string will be converted to an integer of 0 without any failure prompt, but we will not detect this type of error
	int nOther = _ttoi(param);
	RETURN_ON_FAIL(errno != EINVAL && errno != ERANGE);

	return (nFieldValue < nOther);
}

bool WINAPI ConditionFunctions::IntEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::integer);
	const IntField* pIntField = NULL;
	pIntField = dynamic_cast<const IntField*>(pField);
	RETURN_ON_FAIL(NULL != pIntField);
	int nFieldValue = pIntField->GetValueInt();

	// a wrong string will be converted to an integer of 0 without any failure prompt, but we will not detect this type of error
	int nOther = _ttoi(param);
	RETURN_ON_FAIL(errno != EINVAL && errno != ERANGE);

	return (nFieldValue == nOther);
}

bool WINAPI ConditionFunctions::IntLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::integer);
	const IntField* pIntField = NULL;
	pIntField = dynamic_cast<const IntField*>(pField);
	RETURN_ON_FAIL(NULL != pIntField);
	int nFieldValue = pIntField->GetValueInt();

	// a wrong string will be converted to an integer of 0 without any failure prompt, but we will not detect this type of error
	int nOther = _ttoi(param);
	RETURN_ON_FAIL(errno != EINVAL && errno != ERANGE);

	return (nFieldValue > nOther);
}

bool WINAPI ConditionFunctions::RealLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::real);
	const RealField* pRealField = NULL;
	pRealField = dynamic_cast<const RealField*>(pField);
	RETURN_ON_FAIL(NULL != pRealField);
	Real rFieldValue = pRealField->GetValueReal();

	// a wrong string will be converted to an integer of 0 without any failure prompt, but we will not detect this type of error
	Real rOther = _tstof(param);
	RETURN_ON_FAIL(errno != EINVAL && errno != ERANGE);

	return (rFieldValue < rOther);
}

bool WINAPI ConditionFunctions::RealEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::real);
	const RealField* pRealField = NULL;
	pRealField = dynamic_cast<const RealField*>(pField);
	RETURN_ON_FAIL(NULL != pRealField);
	Real rFieldValue = pRealField->GetValueReal();

	// a wrong string will be converted to an integer of 0 without any failure prompt, but we will not detect this type of error
	Real rOther = _tstof(param);
	RETURN_ON_FAIL(errno != EINVAL && errno != ERANGE);

	return (rFieldValue == rOther);
}

bool WINAPI ConditionFunctions::RealLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::real);
	const RealField* pRealField = NULL;
	pRealField = dynamic_cast<const RealField*>(pField);
	RETURN_ON_FAIL(NULL != pRealField);
	Real rFieldValue = pRealField->GetValueReal();

	// a wrong string will be converted to an integer of 0 without any failure prompt, but we will not detect this type of error
	Real rOther = _tstof(param);
	RETURN_ON_FAIL(errno != EINVAL && errno != ERANGE);

	return (rFieldValue > rOther);
}

bool WINAPI ConditionFunctions::TimeLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::timestamp);
	const TimestampField* pTimeField = NULL;
	pTimeField = dynamic_cast<const TimestampField*>(pField);
	RETURN_ON_FAIL(NULL != pTimeField);
	
	const Timestamp* pFieldValue = pTimeField->GetPointerTime();

	Timestamp other;
	RETURN_ON_FAIL(Timestamp::ParseTimestampFromString(param, &other));

	return (pFieldValue->operator <(other));
}

bool WINAPI ConditionFunctions::TimeEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::timestamp);
	const TimestampField* pTimeField = NULL;
	pTimeField = dynamic_cast<const TimestampField*>(pField);
	RETURN_ON_FAIL(NULL != pTimeField);
	
	const Timestamp* pFieldValue = pTimeField->GetPointerTime();

	Timestamp other;
	RETURN_ON_FAIL(Timestamp::ParseTimestampFromString(param, &other));

	return (pFieldValue->operator ==(other));
}

bool WINAPI ConditionFunctions::TimeLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::timestamp);
	const TimestampField* pTimeField = NULL;
	pTimeField = dynamic_cast<const TimestampField*>(pField);
	RETURN_ON_FAIL(NULL != pTimeField);
	
	const Timestamp* pFieldValue = pTimeField->GetPointerTime();

	Timestamp other;
	RETURN_ON_FAIL(Timestamp::ParseTimestampFromString(param, &other));

	return (pFieldValue->operator >(other));
}

bool WINAPI ConditionFunctions::StringIsEmpty(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	// This function will do not care about the second parameter, the parameter list is
	// defined so just to be exported to a specific function pointer.
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::string);
	const StringField* pStringField = dynamic_cast<const StringField*>(pField);
	RETURN_ON_FAIL(pStringField != NULL);

	const TCHAR* szValue = pStringField->GetValueString();
	for (int pos = 0; szValue[pos] != _T('\0'); ++pos) {
		if (_istgraph(szValue[pos]))	// contains any charater that can be printed out
			return false;
	}

	return true;
}

bool WINAPI ConditionFunctions::StringLessThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::string);
	const StringField* pStringField = dynamic_cast<const StringField*>(pField);
	RETURN_ON_FAIL(pStringField != NULL);

	const TCHAR* szValue = pStringField->GetValueString();
	return (0 < _tcscmp(szValue, param));	// use normal string comparison convension
}

bool WINAPI ConditionFunctions::StringEqualTo(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::string);
	const StringField* pStringField = dynamic_cast<const StringField*>(pField);
	RETURN_ON_FAIL(pStringField != NULL);

	const TCHAR* szValue = pStringField->GetValueString();
	return (0 == _tcscmp(szValue, param));	// use normal string comparison convension
}

bool WINAPI ConditionFunctions::StringLargerThan(const BaseUtils::_Field_Ptr pField, const TCHAR* param) {
	RETURN_ON_FAIL(pField->GetType() == BaseUtils::Field::string);
	const StringField* pStringField = dynamic_cast<const StringField*>(pField);
	RETURN_ON_FAIL(pStringField != NULL);

	const TCHAR* szValue = pStringField->GetValueString();
	return (0 > _tcscmp(szValue, param));	// use normal string comparison convension
}

