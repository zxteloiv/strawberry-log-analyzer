// StringRange.cpp
// Implements the StringRange class along its derived virtual functions.
//

#include "stdafx.h"
#include "StringRange.h"
#include "aggregator.h"	// for Aggregator::ReceiveRecord() in range aggregators

using namespace ProcessCore;

StringRange::StringRange(bool			case_sensitive,
						 bool			reversed_comp,
						 AGGREGATE_TYPE	agg_type,
						 int			char_to_comp,
						 AGGREGATE_FUNC	pFunc,
						 Aggregator*	pAgg)
						 : Range(agg_type, pFunc, pAgg)
{
	m_rangeType = string_range;
	m_bReversedComp = reversed_comp;
	m_bCaseSensitive = case_sensitive;
	m_nCompCharsCount = char_to_comp;
	m_szContent = NULL;

}

StringRange::~StringRange() {
	// For the following two Aggregate type, clean is needed.
	if (m_aggType == AGG_MAX || m_aggType == AGG_MIN) {
		DELETE_ARRAY_POINTER(m_val.string_ptr);
	}
	DELETE_ARRAY_POINTER(m_szContent);
}

CompRangeReturn StringRange::IsLessThan(ProcessCore::_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(string_range != pOther->GetRangeType());

	StringRange* pStringRange = dynamic_cast<StringRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pStringRange == NULL);
	
	RETURN_UNEXPECTEDERROR_WHEN(m_nCompCharsCount != pStringRange->m_nCompCharsCount);

	// If this string is less than pOther, that is the string < pOther, then the
	// functions shall all return negative value.
	// Three conditions in all yield 8 situations
	TCHAR* str1 = m_szContent;
	TCHAR* str2 = pStringRange->m_szContent;
	if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 > StringRange::_tcscmp_reversed(str1, str2));
	} else if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING){
		RETURN_YES_WHEN(0 > _tcsncmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 > StringRange::_tcsicmp_reversed(str1, str2));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 > _tcsnicmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 > _tcscmp(str1, str2));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 > _tcsncmp(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 > _tcsicmp(str1, str2));
	} else {
		RETURN_YES_WHEN(0 > _tcsnicmp(str1, str2, m_nCompCharsCount));
	}
	return CompNo;
}

CompRangeReturn StringRange::EqualsTo(_Range_Ptr pOther) const {
	RETURN_NULLPOINTER_WHEN(pOther == NULL);

	RETURN_TYPEMISMATCH_WHEN(string_range != pOther->GetRangeType());

	StringRange* pStringRange = dynamic_cast<StringRange*>(pOther);
	RETURN_POINTERCONVERSIONFAILED_WHEN(pStringRange == NULL);
	
	RETURN_UNEXPECTEDERROR_WHEN(m_nCompCharsCount != pStringRange->m_nCompCharsCount);

	// If this string is less than pOther, that is the string < pOther, then the
	// functions shall all return negative value.
	// Three conditions in all yield 8 situations
	TCHAR* str1 = m_szContent;
	TCHAR* str2 = pStringRange->m_szContent;
	if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 == StringRange::_tcscmp_reversed(str1, str2));
	} else if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING){
		RETURN_YES_WHEN(0 == _tcsncmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 == StringRange::_tcsicmp_reversed(str1, str2));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 == _tcsnicmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 == _tcscmp(str1, str2));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 == _tcsncmp(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		RETURN_YES_WHEN(0 == _tcsicmp(str1, str2));
	} else {
		RETURN_YES_WHEN(0 == _tcsnicmp(str1, str2, m_nCompCharsCount));
	}
	return CompNo;
}

bool StringRange::operator <(StringRange& other) {
	RETURN_ON_FAIL(m_nCompCharsCount != other.m_nCompCharsCount);
	TCHAR* str1 = m_szContent;
	TCHAR* str2 = other.m_szContent;
	if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 > StringRange::_tcscmp_reversed(str1, str2));
	} else if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING){
		return (0 > _tcsncmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 > StringRange::_tcsicmp_reversed(str1, str2));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		return (0 > _tcsnicmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 > _tcscmp(str1, str2));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		return (0 > _tcsncmp(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 > _tcsicmp(str1, str2));
	} else {
		return (0 > _tcsnicmp(str1, str2, m_nCompCharsCount));
	}
}

bool StringRange::operator ==(StringRange& other) {
	RETURN_ON_FAIL(m_nCompCharsCount != other.m_nCompCharsCount);
	TCHAR* str1 = m_szContent;
	TCHAR* str2 = other.m_szContent;
	if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 == StringRange::_tcscmp_reversed(str1, str2));
	} else if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING){
		return (0 == _tcsncmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 == StringRange::_tcsicmp_reversed(str1, str2));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		return (0 == _tcsnicmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 == _tcscmp(str1, str2));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		return (0 == _tcsncmp(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 == _tcsicmp(str1, str2));
	} else {
		return (0 == _tcsnicmp(str1, str2, m_nCompCharsCount));
	}
}

bool StringRange::operator >(StringRange& other) {
	RETURN_ON_FAIL(m_nCompCharsCount != other.m_nCompCharsCount);
	TCHAR* str1 = m_szContent;
	TCHAR* str2 = other.m_szContent;
	if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 < StringRange::_tcscmp_reversed(str1, str2));
	} else if (m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING){
		return (0 < _tcsncmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 < StringRange::_tcsicmp_reversed(str1, str2));
	} else if (!m_bCaseSensitive && m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		return (0 < _tcsnicmp_reversed(str1, str2, m_nCompCharsCount));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 < _tcscmp(str1, str2));
	} else if (m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount != COMPARE_ENTIRE_STRING) {
		return (0 < _tcsncmp(str1, str2, m_nCompCharsCount));
	} else if (!m_bCaseSensitive && !m_bReversedComp && m_nCompCharsCount == COMPARE_ENTIRE_STRING) {
		return (0 < _tcsicmp(str1, str2));
	} else {
		return (0 < _tcsnicmp(str1, str2, m_nCompCharsCount));
	}
}

bool StringRange::GetRangeAggValue(void** ppValue) {
	RETURN_ON_FAIL(ppValue != NULL && m_pFuncAgg != NULL);
	int** ppIntValue = NULL;
	TCHAR** pszString = NULL;
	switch (m_aggType) {
		case AGG_COUNT:
			ppIntValue = reinterpret_cast<int**>(ppValue);
			*ppIntValue = &(m_val.int_value);
			break;
		case AGG_MAX:
		case AGG_MIN:
			pszString = reinterpret_cast<TCHAR**>(ppValue);
			*pszString = m_val.string_ptr;
			break;
		default:
			return false;
	};
	return true;
}

bool StringRange::DoAggregate(BaseUtils::_Field_Ptr pBaseField) {
	RETURN_ON_FAIL(m_pFuncAgg != NULL);
	RETURN_ON_FAIL(m_pFuncAgg(pBaseField, &m_val));
	if (m_pRangeAgg != NULL) {
		RETURN_ON_FAIL(m_pRangeAgg->ReceiveRecord(pBaseField->GetRecordBelongedTo()));
	}
	return true;
}


bool StringRange::SetString(const TCHAR* str) {
	RETURN_ON_FAIL(m_szContent == NULL);
	int len = _tcslen(str);
	m_szContent = new TCHAR[len + 1];
	RETURN_ON_FAIL(m_szContent != NULL);
	return (0 == _tcscpy_s(m_szContent, len + 1, str));
}

int StringRange::_tcscmp_reversed(const TCHAR *str1, const TCHAR *str2) {
	int len1 = _tcslen(str1);
	int len2 = _tcslen(str2);
	while (len1 > 0 && len2 > 0) {
		--len1;
		--len2;
		wint_t minus = str1[len1] - str2[len2];
		if (minus < 0) {
			return -1;	// str1 - str2 = -1 < 0 means str1 < str2
		}
		if (minus > 0) {
			return 1;	// str1 - str2 = 1 > 0 means str1 > str2
		}
	}
	if (len1 > 0) {	// str2 is finished first, then str2 is less than str1
		return 1;
	} else if (len2 > 0) {	// str1 is finished first, then str1 is less than str2
		return -1;
	} else {	// they are the same until the last character
		return 0;
	}
}

int StringRange::_tcsicmp_reversed(const TCHAR *str1, const TCHAR *str2) {
	int len1 = _tcslen(str1);
	int len2 = _tcslen(str2);
	while (len1 > 0 && len2 > 0) {
		--len1;
		--len2;
		wint_t minus = _totlower(str1[len1]) - _totlower(str2[len2]);
		if (minus < 0) {
			return -1;	// str1 - str2 = -1 < 0 means str1 < str2
		}
		if (minus > 0) {
			return 1;	// str1 - str2 = 1 > 0 means str1 > str2
		}
	}
	if (len1 > 0) {	// str2 is finished first, then str2 is less than str1
		return 1;
	} else if (len2 > 0) {	// str1 is finished first, then str1 is less than str2
		return -1;
	} else {	// they are the same until the last character
		return 0;
	}
}

int StringRange::_tcsncmp_reversed(const TCHAR *str1, const TCHAR *str2, int max_count) {
	int len1 = _tcslen(str1);
	int len2 = _tcslen(str2);
	while (len1 > 0 && len2 > 0 && max_count > 0) {
		--len1;
		--len2;
		--max_count;
		wint_t minus = str1[len1] - str2[len2];
		if (minus < 0) {
			return -1;	// str1 - str2 = -1 < 0 means str1 < str2
		}
		if (minus > 0) {
			return 1;	// str1 - str2 = 1 > 0 means str1 > str2
		}
	}
	if (0 == max_count) {	// "max_count" characters are checked the same
		return 0;
	}
	if (len1 > 0) {	// str2 is finished first, then str2 is less than str1
		return 1;
	} else if (len2 > 0) {	// str1 is finished first, then str1 is less than str2
		return -1;
	} else {	// they are the same until the last character
		return 0;
	}
}

int StringRange::_tcsnicmp_reversed(const TCHAR * str1, const TCHAR *str2, int max_count) {
	int len1 = _tcslen(str1);
	int len2 = _tcslen(str2);
	while (len1 > 0 && len2 > 0 && max_count > 0) {
		--len1;
		--len2;
		--max_count;
		wint_t minus = _totlower(str1[len1]) - _totlower(str2[len2]);
		if (minus < 0) {
			return -1;	// str1 - str2 = -1 < 0 means str1 < str2
		}
		if (minus > 0) {
			return 1;	// str1 - str2 = 1 > 0 means str1 > str2
		}
	}
	if (0 == max_count) {	// "max_count" characters are checked the same
		return 0;
	}
	if (len1 > 0) {	// str2 is finished first, then str2 is less than str1
		return 1;
	} else if (len2 > 0) {	// str1 is finished first, then str1 is less than str2
		return -1;
	} else {	// they are the same until the last character
		return 0;
	}
}
