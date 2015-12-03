// AggreagatorManager.cpp
// Implements the AggregatorManager class.
//

#include "stdafx.h"
#include "aggregator.h"
#include "AggregatorManager.h"
#include "RangeFuncParam.h"
#include "RangeSet.h"		// for saving result of an aggregator that uses RangeSet
#include "RangeHashset.h"	// for saving result of an aggregator that uses RangeHashset

#include "QuestionTree.h"
#include <string>


#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")
#include "..\\Toolkit\\Toolkit.h"

#include "..\\BaseUtils\\BaseUtils.h"

#include "..\\ExternalRoutes\\ExternalRoutes.h"

#pragma warning(disable : 4018)	// disable waring for '<' signed/unsigned mismatch
#pragma warning(disable : 4244)	// disable warning for converting double to float

#define SAME_TSTR(X,Y)	(0 == _tcscmp((X), (Y)))

using namespace ProcessCore;

// ----------------------------------------------------------------------------------------------
// Static members of AggregatorManager class


#pragma region Strings For XML Parsing

const TCHAR* AggregatorManager::s_szElemAggregator = _T("agg");
const TCHAR* AggregatorManager::s_szAttrID = _T("id");

const TCHAR* AggregatorManager::s_szElemBaseField = _T("base_field");

const TCHAR* AggregatorManager::s_szElemRangeFunc = _T("range_func");
const TCHAR* AggregatorManager::s_szAttrRangeFuncAuto = _T("auto_range");
const TCHAR* AggregatorManager::s_szAttrFuncName = _T("func_name");
const TCHAR* AggregatorManager::s_szAttrImageName = _T("image_name");
const TCHAR* AggregatorManager::s_szAttrValDefaultImageName = _T("ExternalRoutes.dll");
const TCHAR* AggregatorManager::s_szAttrRangeFuncType = _T("type");
const TCHAR* AggregatorManager::s_szAttrValRangeFuncTypeInt = _T("int");
const TCHAR* AggregatorManager::s_szAttrValRangeFuncTypeReal = _T("real");
const TCHAR* AggregatorManager::s_szAttrValRangeFuncTypeTime = _T("time");
const TCHAR* AggregatorManager::s_szAttrValRangeFuncTypeString = _T("string");
const TCHAR* AggregatorManager::s_szRangeFuncDefaultNameInt = _T("_RangeFuncInt@16");
const TCHAR* AggregatorManager::s_szRangeFuncDefaultNameReal = _T("_RangeFuncReal@16");
const TCHAR* AggregatorManager::s_szRangeFuncDefaultNameTime = _T("_RangeFuncTime@16");
const TCHAR* AggregatorManager::s_szRangeFuncDefaultNameString = _T("_RangeFuncString@16");

const TCHAR* AggregatorManager::s_szElemRangeFuncInt = _T("int");
const TCHAR* AggregatorManager::s_szElemRangeFuncReal = _T("real");
const TCHAR* AggregatorManager::s_szElemRangeFuncTimestamp = _T("timestamp");
const TCHAR* AggregatorManager::s_szAttrRangeAutoAnchor = _T("anchor");
const TCHAR* AggregatorManager::s_szAttrRangeAutoWidth = _T("width");

const TCHAR* AggregatorManager::s_szElemRangeFuncString = _T("string");
const TCHAR* AggregatorManager::s_szAttrRangeFuncStringCaseSensitive = _T("case_sensitive");
const TCHAR* AggregatorManager::s_szAttrRangeFuncStringNumberOfCompChar = _T("number_of_char_to_comp");
const TCHAR* AggregatorManager::s_szAttrRangeFuncStringReversedComp = _T("reversed_comp");

const TCHAR* AggregatorManager::s_szElemAggFunc = _T("aggregate_func");
const TCHAR* AggregatorManager::s_szAttrAggFuncType = _T("type");
const TCHAR* AggregatorManager::s_szAttrValAggFuncTypeCount = _T("count");
const TCHAR* AggregatorManager::s_szAttrValAggFuncTypeMin = _T("min");
const TCHAR* AggregatorManager::s_szAttrValAggFuncTypeMax = _T("max");
const TCHAR* AggregatorManager::s_szAggFuncNameCount = _T("_AggFuncCount@8");
const TCHAR* AggregatorManager::s_szAggFuncNameMin = _T("_AggFuncMic@8");
const TCHAR* AggregatorManager::s_szAggFuncNameMax = _T("_AggFuncMax@8");

const TCHAR* AggregatorManager::s_szElemAccumulate = _T("accumulate");
const TCHAR* AggregatorManager::s_szAttrAccumulateDirection = _T("direction");
const TCHAR* AggregatorManager::s_szAttrValDirectionLeft = _T("left");
const TCHAR* AggregatorManager::s_szAttrValDirectionRight = _T("right");
const TCHAR* AggregatorManager::s_szAttrValDirectionNone = _T("none");

const TCHAR* AggregatorManager::s_szElemConditionFields = _T("condition_fields");
const TCHAR* AggregatorManager::s_szElemConditionField = _T("field");
const TCHAR* AggregatorManager::s_szAttrFuncParam = _T("params");
const TCHAR* AggregatorManager::s_szAttrCFPreferedValue = _T("prefered_value");

const TCHAR* AggregatorManager::s_szAttrValTrue = _T("true");
const TCHAR* AggregatorManager::s_szAttrValFalse = _T("false");

#pragma endregion

#pragma region Strings For Writing XML Result

const TCHAR* AggregatorManager::s_szElemAggregatorOutput = _T("agg");
const TCHAR* AggregatorManager::s_szAttrAggIDOutput = _T("agg_id");
const TCHAR* AggregatorManager::s_szAttrRangeCountOutput = _T("range_count");
const TCHAR* AggregatorManager::s_szElemRangeOutput = _T("range");
const TCHAR* AggregatorManager::s_szAttrContentOutput = _T("content");
const TCHAR* AggregatorManager::s_szAttrValueOutput = _T("value");

#pragma endregion

// -----------------------------------------------------------------------------------------------
// Global C help Functions for reading each element in the Aggregator Definition XML.
// Help functions are declared here in this CPP file(not in any header), therefore they will not
// be referenced in any other CPP files.

namespace ProcessCore {
	// Store the information retrieved from the Aggregators definition XML
	struct AggregatorContext {
		TCHAR*			szBaseField;
		LPCONDLIST		pCondList;
		RANGE_FUNC		pRangeFunc;
		RangeParam*		pRangeParam;
		AGGREGATE_FUNC	pAggFunc;
		AGGREGATE_TYPE	aggType;

		Aggregator::ACCUMULATION_DIRECTION	direction;

		void reset() {
			szBaseField = NULL;
			pCondList = NULL;
			pRangeFunc = NULL;
			pRangeParam = NULL;
			pAggFunc = NULL;
			aggType = AGG_COUNT;
			direction = Aggregator::accu_none;
		}

		bool validate() {
			bool is_initial = (NULL == szBaseField && NULL == pCondList &&
				NULL == pRangeFunc && NULL == pRangeParam && NULL == pAggFunc);
			return (!is_initial);
		}
	};

	class AggregatorManagerHelper {
	public:
		static bool ReadElementAgg(IXmlReader* pReader, AggregatorContext* pAgg);
		static bool ReadElementBaseField(IXmlReader* pReader, AggregatorContext* pAgg);
		static bool ReadElementRangeFunc(IXmlReader* pReader, AggregatorContext* pAgg, const TCHAR** ppszDefaultAggFuncType,
			AggregatorManager*);
		static bool ReadElementIntRangeFunc(IXmlReader* pReader, AggregatorContext* pAgg, AggregatorManager* pAggMgr);
		static bool ReadElementRealRangeFunc(IXmlReader* pReader, AggregatorContext* pAgg, AggregatorManager* pAggMgr);
		static bool ReadElementTimeRangeFunc(IXmlReader* pReader, AggregatorContext* pAgg, AggregatorManager* pAggMgr);
		static bool ReadElementStringRangeFunc(IXmlReader* pReader, AggregatorContext* pAgg, AggregatorManager* pAggMgr);
		static bool ReadElementAggFunc(IXmlReader* pReader, AggregatorContext* pAgg, AggregatorManager* pAggMgr);
		static bool ReadElementAccumulate(IXmlReader* pReader, AggregatorContext* pAgg);
		static bool ReadElementConditionFields(IXmlReader* pReader, AggregatorContext* pAgg);
		static bool ReadElementField(IXmlReader* pReader, AggregatorContext* pAgg, AggregatorManager* pAggMgr);
	};
}
// ------------------------------------------------------------------------------------------------
// AggregatorManager class implementation

AggregatorManager::AggregatorManager() : m_pAggHolder(NULL), m_pDerivedAggs(NULL), m_pDllModules(NULL) {
}

AggregatorManager::~AggregatorManager() {
	RELEASE_STD_CONTAINER_CONTENT(AGGLIST, m_pDerivedAggs);
	DELETE_POINTER(m_pDerivedAggs);
	RELEASE_STD_CONTAINER_CONTENT(AGGVECTOR, m_pAggHolder);
	DELETE_POINTER(m_pAggHolder);
	if (m_pDllModules != NULL) {
		for (MODULEITER iter = m_pDllModules->begin(); iter != m_pDllModules->end(); ++iter) {
			::FreeLibrary(*iter);
		}
		DELETE_POINTER(m_pDllModules);
	}
}

bool AggregatorManager::Init() {
	RETURN_ON_FAIL(NULL == m_pAggHolder && NULL == m_pDerivedAggs && NULL == m_pDllModules);
	PROCESS_ERROR(NULL != (m_pAggHolder = new AGGVECTOR));
	PROCESS_ERROR(NULL != (m_pDerivedAggs = new AGGLIST));
	PROCESS_ERROR(NULL != (m_pDllModules = new MODULELIST));

	return true;
Exit0:
	DELETE_POINTER(m_pAggHolder);
	DELETE_POINTER(m_pDerivedAggs);
	DELETE_POINTER(m_pDllModules);
	return false;
}

Aggregator* AggregatorManager::GetAggregatorByID(int agg_id) const {
	if (m_pAggHolder != NULL && agg_id < m_pAggHolder->size())
		return m_pAggHolder->at(agg_id);
	return NULL;
}

bool AggregatorManager::CreateAggregator(ProcessCore::Aggregator** ppAgg, const ProcessCore::QuestionTreeNode* pNode) {
	RETURN_ON_FAIL(pNode != NULL && ppAgg != NULL);
	if (pNode->m_nAggID < m_pAggHolder->size()) {
		RETURN_ON_FAIL(_CreateAggregator(ppAgg, m_pAggHolder->at(pNode->m_nAggID)));
		(*ppAgg)->Init(pNode, this);
		return true;
	}
	return false;
}

bool AggregatorManager::_CreateAggregator(ProcessCore::Aggregator** ppAgg, const ProcessCore::Aggregator* agg_template) {
	Aggregator* pNewAgg = NULL;
	pNewAgg = new Aggregator(agg_template->m_szBaseField, agg_template->m_pConditionList,
		agg_template->m_pFuncRange, agg_template->m_pRangeParam, agg_template->m_pFuncAgg,
		agg_template->m_aggType, agg_template->m_pQuestionNode, this, agg_template->m_direction);
	RETURN_ON_FAIL(pNewAgg != NULL);
	m_pDerivedAggs->push_back(pNewAgg);
	*ppAgg = pNewAgg;
	return true;
}

bool AggregatorManager::LoadExternalFunction(const TCHAR* szImageName, const TCHAR* szFuncName, RANGE_FUNC* ppFunc) {
	HMODULE hDll = NULL;
	RETURN_ON_FAIL(NULL != (hDll = ::LoadLibrary(szImageName)));
	
#ifdef UNICODE
	// Because function name in a dll must be English characters & underline, both the ANSI version & the UNICODE version
	// of this function name must be of the SAME length.
	int len = _tcslen(szFuncName) + 1;
	char* szAnsiFuncName = NULL;
	RETURN_ON_FAIL(NULL != (szAnsiFuncName = new char[len]));
	if (0 == WideCharToMultiByte(CP_ACP, 0, szFuncName, -1, szAnsiFuncName, len, NULL, NULL)) {	// convert error
		DELETE_ARRAY_POINTER(szAnsiFuncName);
		::FreeLibrary(hDll);
		return false;
	}
	RANGE_FUNC pFunc = (DLL_RANGE_FUNC)::GetProcAddress(hDll, szAnsiFuncName);
	if (pFunc == NULL) {	// error
		DELETE_ARRAY_POINTER(szAnsiFuncName);
		::FreeLibrary(hDll);
		return false;
	}
#else
	RANGE_FUNC pFunc = (DLL_RANGE_FUNC)::GetProcAddress(hDll, szFuncName);
	if (pFunc == NULL) {
		::FreeLibrary(hDll);
		return false;
	}
#endif

	*ppFunc = pFunc;
	m_pDllModules->push_back(hDll);	
	return true;
}

bool AggregatorManager::LoadExternalFunction(const TCHAR* szImageName, const TCHAR* szFuncName, AGGREGATE_FUNC* ppFunc) {
	HMODULE hDll = NULL;
	RETURN_ON_FAIL(NULL != (hDll = ::LoadLibrary(szImageName)));
	
#ifdef UNICODE
	// Because function name in a dll must be English characters & underline, both the ANSI version & the UNICODE version
	// of this function name must be of the SAME length.
	int len = _tcslen(szFuncName) + 1;
	char* szAnsiFuncName = NULL;
	RETURN_ON_FAIL(NULL != (szAnsiFuncName = new char[len]));
	if (0 == WideCharToMultiByte(CP_ACP, 0, szFuncName, -1, szAnsiFuncName, len, NULL, NULL)) {	// convert error
		DELETE_ARRAY_POINTER(szAnsiFuncName);
		::FreeLibrary(hDll);
		return false;
	}
	AGGREGATE_FUNC pFunc = (DLL_AGG_FUNC)::GetProcAddress(hDll, szAnsiFuncName);
	if (pFunc == NULL) {	// error
		DELETE_ARRAY_POINTER(szAnsiFuncName);
		::FreeLibrary(hDll);
		return false;
	}
#else
	AGGREGATE_FUNC pFunc = (DLL_AGG_FUNC)::GetProcAddress(hDll, szFuncName);
	if (pFunc == NULL) {
		::FreeLibrary(hDll);
		return false;
	}
#endif

	*ppFunc = pFunc;
	m_pDllModules->push_back(hDll);
	return true;
}

bool AggregatorManager::LoadExternalFunction(const TCHAR* szImageName, const TCHAR *szFuncName, FIELD_CONDITION_FUNC* ppFunc) {
	HMODULE hDll = NULL;
	RETURN_ON_FAIL(NULL != (hDll = ::LoadLibrary(szImageName)));
	
#ifdef UNICODE
	// Because function name in a dll must be English characters & underline, both the ANSI version & the UNICODE version
	// of this function name must be of the SAME length.
	int len = _tcslen(szFuncName) + 1;
	char* szAnsiFuncName = NULL;
	RETURN_ON_FAIL(NULL != (szAnsiFuncName = new char[len]));
	if (0 == WideCharToMultiByte(CP_ACP, 0, szFuncName, -1, szAnsiFuncName, len, NULL, NULL)) {	// convert error
		DELETE_ARRAY_POINTER(szAnsiFuncName);
		::FreeLibrary(hDll);
		return false;
	}
	FIELD_CONDITION_FUNC pFunc = (DLL_FILTER_FUNC)::GetProcAddress(hDll, szAnsiFuncName);
	if (pFunc == NULL) {	// error
		DELETE_ARRAY_POINTER(szAnsiFuncName);
		::FreeLibrary(hDll);
		return false;
	}
#else
	FIELD_CONDITION_FUNC pFunc = (DLL_FILTER_FUNC)::GetProcAddress(hDll, szFuncName);
	if (pFunc == NULL) {
		::FreeLibrary(hDll);
		return false;
	}
#endif

	*ppFunc = pFunc;
	m_pDllModules->push_back(hDll);
	return true;
}

void AggregatorManager::ReleaseDerivedAggs() {
	RELEASE_STD_CONTAINER_CONTENT(AGGLIST, m_pDerivedAggs);
	if (m_pDerivedAggs != NULL)
		m_pDerivedAggs->clear();
}

void AggregatorManager::ReleaseTemplateAggs() {
	if (m_pAggHolder != NULL) {
		for (AGGVECTITER iter = m_pAggHolder->begin(); iter != m_pAggHolder->end(); ++iter) {
			Aggregator* pAgg = *iter;
			RELEASE_STD_CONTAINER_CONST_CONTENT(CONDLIST, pAgg->m_pConditionList);
			DELETE_POINTER(pAgg->m_pConditionList);
			DELETE_POINTER(pAgg->m_pRangeParam);
			DELETE_ARRAY_POINTER(pAgg->m_szBaseField);
		}
		m_pAggHolder->clear();
	}
}

bool AggregatorManager::SaveQuestionResult(void* pXmlWriter) {
	RETURN_ON_FAIL(pXmlWriter != NULL);
	IXmlWriter* pWriter = reinterpret_cast<IXmlWriter*>(pXmlWriter);
	TCHAR szNumBuf[12] = _T(""); // 12 is enough for holding both the maximum and the minimum integer

	// Currently the derived aggregators are all related to this question.
	// Write <agg> for each aggregator in the derived agg list.
	for (AGGLISTITER iter = m_pDerivedAggs->begin(); iter != m_pDerivedAggs->end(); ++iter) {
		Aggregator* pCurrentAgg = *iter;
		RETURN_ON_FAIL(pCurrentAgg != NULL && pCurrentAgg->m_pQuestionNode != NULL);
		if (!pCurrentAgg->m_pQuestionNode->m_bSaveRangeCount && !pCurrentAgg->m_pQuestionNode->m_bSaveRangeValue) {
			continue;	// if result of this agg is not needed to save, do with the next aggregator
		}

		RETURN_ON_FAIL(S_OK == pWriter->WriteStartElement(NULL, s_szElemAggregatorOutput, NULL));	// <agg>

		if (pCurrentAgg->m_pQuestionNode->m_bSaveRangeCount) {
			// ATTRIBUTE agg_id=""
			RETURN_ON_FAIL((*iter) != NULL && (*iter)->m_pQuestionNode != NULL);
			RETURN_ON_FAIL(0 == _itot_s((*iter)->m_pQuestionNode->m_nAggID, szNumBuf, 12, 10));
			RETURN_ON_FAIL(S_OK == pWriter->WriteAttributeString(NULL, s_szAttrAggIDOutput, NULL, szNumBuf));

			// ATTRIBUTE range_count=""
			RETURN_ON_FAIL((*iter) != NULL && (*iter)->m_pRangeContainer != NULL);
			int nRangeCount = (*iter)->m_pRangeContainer->GetContainerCount();
			RETURN_ON_FAIL(0 == _itot_s(nRangeCount, szNumBuf, 12, 10));
			RETURN_ON_FAIL(S_OK == pWriter->WriteAttributeString(NULL, s_szAttrRangeCountOutput, NULL, szNumBuf));
		}

		if (pCurrentAgg->m_pQuestionNode->m_bSaveRangeValue) {
			RETURN_ON_FAIL(SaveSingleAggregator(*iter, pXmlWriter));
		}

		RETURN_ON_FAIL(S_OK == pWriter->WriteEndElement());	// </agg>
	}
	return true;
}

bool AggregatorManager::SaveSingleAggregator(Aggregator* pAgg, void* pXmlWriter) {
	RETURN_ON_FAIL(pXmlWriter != NULL && pAgg != NULL && pAgg->m_pRangeContainer != NULL);
	IXmlWriter* pWriter = reinterpret_cast<IXmlWriter*>(pXmlWriter);

	RangeSet* pRangeSet = dynamic_cast<RangeSet*>(pAgg->m_pRangeContainer);
	RangeHashset* pRangeHashset = dynamic_cast<RangeHashset*>(pAgg->m_pRangeContainer);
	if (pRangeSet != NULL) {
		for (RangeSet::iterator iter = pRangeSet->begin(); iter != pRangeSet->end(); ++iter) {
			_Range_Ptr pRange = *iter;
			RETURN_ON_FAIL(S_OK == pWriter->WriteStartElement(NULL, s_szElemRangeOutput, NULL));	// <range>

			RETURN_ON_FAIL(SaveRangeContentAttr(pRange, pXmlWriter));
			RETURN_ON_FAIL(SaveRangeValueAttr(pRange, pXmlWriter));

			RETURN_ON_FAIL(S_OK == pWriter->WriteEndElement());	// </range>			
		}
	} else if (pRangeHashset != NULL) {
		for (RangeHashset::iterator iter = pRangeHashset->begin(); iter != pRangeHashset->end(); ++iter) {
			_Range_Ptr pRange = *iter;
			RETURN_ON_FAIL(S_OK == pWriter->WriteStartElement(NULL, s_szElemRangeOutput, NULL));	// <range>

			RETURN_ON_FAIL(SaveRangeContentAttr(pRange, pXmlWriter));
			RETURN_ON_FAIL(SaveRangeValueAttr(pRange, pXmlWriter));

			RETURN_ON_FAIL(S_OK == pWriter->WriteEndElement());	// </range>
		}
	} else
		RETURN_ON_FAIL(false);	// error
	
	return true;
}

bool AggregatorManager::SaveRangeContentAttr(Range* pRange, void* pXmlWriter) {
	RETURN_ON_FAIL(pXmlWriter != NULL && pRange != NULL);
	IXmlWriter* pWriter = reinterpret_cast<IXmlWriter*>(pXmlWriter);
	TCHAR szNumBuf[50] = _T("");	
	std::wstring sAttrVal;

	// check range type and write content attribute
	Range::RangeType range_type = pRange->GetRangeType();
	if (range_type == Range::integer_range) {
		IntRange* pIntRange = dynamic_cast<IntRange*>(pRange);
		RETURN_ON_FAIL(pIntRange != NULL);
		
		sAttrVal.append(L"(");
		RETURN_ON_FAIL(0 == _itot_s(pIntRange->GetLowerBound(), szNumBuf, 50, 10));
		sAttrVal.append(szNumBuf);
		sAttrVal.append(L",");
		RETURN_ON_FAIL(0 == _itot_s(pIntRange->GetUpperBound(), szNumBuf, 50, 10));
		sAttrVal.append(szNumBuf);
		sAttrVal.append(L")");
	} else if (range_type == Range::real_range) {
		RealRange* pRealRange = dynamic_cast<RealRange*>(pRange);
		RETURN_ON_FAIL(pRealRange != NULL);
		
		sAttrVal.append(L"(");
		RETURN_ON_FAIL(0 == _stprintf_s(szNumBuf, 50, _T("%lf"), pRealRange->GetLowerBound()));
		sAttrVal.append(szNumBuf);
		sAttrVal.append(L",");
		RETURN_ON_FAIL(0 == _stprintf_s(szNumBuf, 50, _T("%lf"), pRealRange->GetUpperBound()));
		sAttrVal.append(szNumBuf);
		sAttrVal.append(L")");
	} else if (range_type == Range::string_range) {
		StringRange* pStringRange = dynamic_cast<StringRange*>(pRange);
		sAttrVal.append(pStringRange->GetContentString());				
	} else if (range_type == Range::timestamp_range) {
		TimestampRange* pTimeRange = dynamic_cast<TimestampRange*>(pRange);
		BaseUtils::Timestamp boundary;
		RETURN_ON_FAIL(pTimeRange->GetLowerBound(&boundary));
		sAttrVal.append(L"(");
		sAttrVal.append(boundary.ToString());
		sAttrVal.append(L",");
		RETURN_ON_FAIL(pTimeRange->GetUpperBound(&boundary));
		sAttrVal.append(boundary.ToString());
		sAttrVal.append(L")");
	} else if (range_type == Range::customized_range) {	// user defined range, not yet supported
		RETURN_ON_FAIL(false);
	}

	RETURN_ON_FAIL(S_OK == pWriter->WriteAttributeString(NULL, s_szAttrContentOutput, NULL, sAttrVal.c_str()));
	return true;
}

bool AggregatorManager::SaveRangeValueAttr(Range* pRange, void* pXmlWriter) {
	RETURN_ON_FAIL(pXmlWriter != NULL && pRange != NULL);
	IXmlWriter* pWriter = reinterpret_cast<IXmlWriter*>(pXmlWriter);
	TCHAR szNumBuf[50] = _T("");
	std::wstring sAttrVal;

	AGGREGATE_TYPE agg_type = pRange->GetAggType();
	if (agg_type == AGG_COUNT) {
		int* pnValue = NULL;
		RETURN_ON_FAIL(pRange->GetRangeAggValue((void**)&pnValue));
		RETURN_ON_FAIL(0 == _itot_s(*pnValue, szNumBuf, 50, 10));
		sAttrVal.append(szNumBuf);
	} else if (agg_type == AGG_MAX || agg_type == AGG_MIN) {	// not supported yet
		RETURN_ON_FAIL(false);
	} else
		RETURN_ON_FAIL(false);

	RETURN_ON_FAIL(S_OK == pWriter->WriteAttributeString(NULL, s_szAttrValueOutput, NULL, sAttrVal.c_str()));
	return true;
}

bool AggregatorManager::BuildAggListFromXML(const TCHAR* xml_string) {
	bool bSuccess = false;
	IXmlReader* pReader = NULL;
	ISequentialStream* pStream = NULL;

	PROCESS_ERROR(S_OK == (Toolkit::CStringStream::Create(xml_string, &pStream)));
	PROCESS_ERROR(S_OK == ::CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL));
	PROCESS_ERROR(S_OK == pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit));
	PROCESS_ERROR(S_OK == pReader->SetInput(pStream));

	XmlNodeType nodeType;
	const TCHAR* local_name;
	AggregatorContext aggContext;
	const TCHAR* szRangeType = NULL;
	while (S_OK == pReader->Read(&nodeType)) {
		if (nodeType == XmlNodeType_Element) {
			PROCESS_ERROR(S_OK == pReader->GetLocalName(&local_name, NULL));
			if (SAME_TSTR(local_name, s_szElemAggregator)) {				// <agg>
				aggContext.reset();
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementAgg(pReader, &aggContext));
			} else if (SAME_TSTR(local_name, s_szElemBaseField)) {			// <base_field>
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementBaseField(pReader, &aggContext));
			} else if (SAME_TSTR(local_name, s_szElemRangeFunc)) {			// <range_func>
				// If the returned value of szRangeType is still NULL, it means this aggregator will use some user-defined
				// function rather than the default range functions. This parameter cannot be given NULL.
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementRangeFunc(pReader, &aggContext, &szRangeType, this));
			} else if (SAME_TSTR(local_name, s_szElemRangeFuncInt)) {		// <int>, a default range function for int field
				PROCESS_ERROR(szRangeType != NULL && SAME_TSTR(szRangeType, s_szAttrValRangeFuncTypeInt));
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementIntRangeFunc(pReader, &aggContext, this));
			} else if (SAME_TSTR(local_name, s_szElemRangeFuncReal)) {		// <real>, a default range function for real field
				PROCESS_ERROR(szRangeType != NULL && SAME_TSTR(szRangeType, s_szAttrValRangeFuncTypeReal));
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementRealRangeFunc(pReader, &aggContext, this));
			} else if (SAME_TSTR(local_name, s_szElemRangeFuncTimestamp)) {	// <timestamp>, a default range function for time field
				PROCESS_ERROR(szRangeType != NULL && SAME_TSTR(szRangeType, s_szAttrValRangeFuncTypeTime));
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementTimeRangeFunc(pReader, &aggContext, this));
			} else if (SAME_TSTR(local_name, s_szElemRangeFuncString)) {	// <string>, a default range function for string field
				PROCESS_ERROR(szRangeType != NULL && SAME_TSTR(szRangeType, s_szAttrValRangeFuncTypeString));
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementStringRangeFunc(pReader, &aggContext, this));
			} else if (SAME_TSTR(local_name, s_szElemAggFunc)) {			// <aggregate_func>
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementAggFunc(pReader, &aggContext, this));
			} else if (SAME_TSTR(local_name, s_szElemAccumulate)) {			// <accumulate>
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementAccumulate(pReader, &aggContext));
			} else if (SAME_TSTR(local_name, s_szElemConditionFields)) {	// <condition_fields>, in this way of parsing, this element is trivial
				PROCESS_ERROR((NULL == aggContext.pCondList) && (NULL != (aggContext.pCondList = new CONDLIST)));
			} else if (SAME_TSTR(local_name, s_szElemConditionField)) {		// <field>
				PROCESS_ERROR(NULL != aggContext.pCondList);
				PROCESS_ERROR(AggregatorManagerHelper::ReadElementField(pReader, &aggContext, this));
			}
		} else if (nodeType == XmlNodeType_EndElement) {
			PROCESS_ERROR(S_OK == pReader->GetLocalName(&local_name, NULL));
			if (SAME_TSTR(local_name, s_szElemAggregator)) {	// </agg> agg definition is finished
				PROCESS_ERROR(aggContext.validate());
				Aggregator* pNewAgg = NULL;
				// After initializing, the AggFunc & AggType data members are all assigned, but the member of
				// the RangeParameter structure is not yet ready here. The constructor function will assign
				// the members of this structure with AggFunc & AggType members of this Aggregator object.
				pNewAgg = new Aggregator(aggContext.szBaseField, aggContext.pCondList, aggContext.pRangeFunc,
					aggContext.pRangeParam, aggContext.pAggFunc, aggContext.aggType, NULL, this, aggContext.direction
					);
				m_pAggHolder->push_back(pNewAgg);
			}
		}
	}	

	bSuccess = true;
Exit0:
	RELEASE_COM_OBJECT(pReader);
	RELEASE_COM_OBJECT(pStream);
	if (!bSuccess) {
		ReleaseTemplateAggs();
		RELEASE_STD_CONTAINER_CONTENT(CONDLIST, aggContext.pCondList);
		DELETE_POINTER(aggContext.pCondList);
		DELETE_POINTER(aggContext.pRangeParam);
		DELETE_ARRAY_POINTER(aggContext.szBaseField);
	}
	return bSuccess;
}

// ---------------------------------------------------------------------------------------------------------
// Implement global help functions declared inside this cpp file

bool AggregatorManagerHelper::ReadElementAgg(IXmlReader* pReader, AggregatorContext* pAggContext) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrID, NULL));
	const TCHAR* value = NULL;
	RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));
	// The ID attributes of all <agg> elements are assumed sorted, starting from 0.
	// Then as the sorted sequence suggested, these aggregators will be inserted into the template vector
	// in the order in which they are places in the xml string.
	// So there's no need to process the ID value here.

	pReader->MoveToElement();
	return true;
}

bool AggregatorManagerHelper::ReadElementBaseField(IXmlReader* pReader, AggregatorContext* pAggContext) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrID, NULL));
	const TCHAR* value = NULL;
	RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));
	// deep-copy value attribute
	DELETE_ARRAY_POINTER(pAggContext->szBaseField);
	int id_len = _tcslen(value) + 1;
	RETURN_ON_FAIL(NULL != (pAggContext->szBaseField = new TCHAR[id_len]));
	if (0 != _tcscpy_s(pAggContext->szBaseField, id_len, value)) {	// that returning value isn't 0 means error occurs
		DELETE_ARRAY_POINTER(pAggContext->szBaseField);
	}

	pReader->MoveToElement();
	return true;
}

bool AggregatorManagerHelper::ReadElementRangeFunc(IXmlReader*			pReader,
												   AggregatorContext*	pAggContext,
												   const TCHAR**		ppszDefaultAggFuncType,
												   AggregatorManager*	pAggMgr)
{
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL && ppszDefaultAggFuncType != NULL);
	bool bAutoRange = true;
	if (S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeFuncAuto, NULL)) {
		const TCHAR* value = NULL;
		RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));
		bAutoRange = SAME_TSTR(value, AggregatorManager::s_szAttrValTrue);
	}

	if (bAutoRange) {	// when AutoRange is applied, it is needed to explicitly indicate the base field type
		RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrAggFuncType, NULL));
		RETURN_ON_FAIL(S_OK == pReader->GetValue(ppszDefaultAggFuncType, NULL));
	} else {	// external func_name & image_name(*.dll, *.exe) shall be designated
		const TCHAR* image_name = NULL;
		const TCHAR* func_name = NULL;
		RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrImageName, NULL));
		RETURN_ON_FAIL(S_OK == pReader->GetValue(&image_name, NULL));
		RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrFuncName, NULL));
		RETURN_ON_FAIL(S_OK == pReader->GetValue(&func_name, NULL));

		if (SAME_TSTR(func_name, _T("null"))) {
			pAggContext->pRangeFunc = NULL;
		} else {
			// Load functions from this dll module
			RETURN_ON_FAIL(pAggMgr->LoadExternalFunction(image_name, func_name, &(pAggContext->pRangeFunc)));
		}
		*ppszDefaultAggFuncType = NULL;	// not use default function
	}
	
	return true;
}

bool AggregatorManagerHelper::ReadElementIntRangeFunc(IXmlReader* pReader, AggregatorContext* pAggContext, AggregatorManager* pAggMgr) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	bool bSuccess = false;
	IntRangeParam* pRangeParam = NULL;
	RETURN_ON_FAIL(NULL != (pRangeParam = new IntRangeParam));
	
	// extract anchor & width attribute values.
	const TCHAR* value;
	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeAutoAnchor, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	int anchor = _ttoi(value);
	PROCESS_ERROR(errno != EINVAL && errno != ERANGE);

	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeAutoWidth, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	int width = _ttoi(value);
	PROCESS_ERROR(errno != EINVAL && errno != ERANGE);

	PROCESS_ERROR(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName, \
		AggregatorManager::s_szRangeFuncDefaultNameInt, &(pAggContext->pRangeFunc)));

	pRangeParam->anchor = anchor;
	pRangeParam->width = width;

	pAggContext->pRangeParam = pRangeParam;	// only part of this structure is assigned with value

	bSuccess = true;
Exit0:
	pReader->MoveToElement();
	if (!bSuccess) {
		DELETE_POINTER(pRangeParam);
		pAggContext->pRangeFunc = NULL;
	}
	return bSuccess;
}

bool AggregatorManagerHelper::ReadElementRealRangeFunc(IXmlReader* pReader, AggregatorContext* pAggContext, AggregatorManager* pAggMgr) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	bool bSuccess = false;
	RealRangeParam* pRangeParam = NULL;
	RETURN_ON_FAIL(NULL != (pRangeParam = new RealRangeParam));
	
	// extract anchor & width attribute values.
	const TCHAR* value;
	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeAutoAnchor, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	BaseUtils::Real anchor = _tstof(value);
	PROCESS_ERROR(errno != EINVAL && errno != ERANGE);

	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeAutoWidth, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	BaseUtils::Real width = _tstof(value);
	PROCESS_ERROR(errno != EINVAL && errno != ERANGE);

	PROCESS_ERROR(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName, \
		AggregatorManager::s_szRangeFuncDefaultNameReal, &(pAggContext->pRangeFunc)));
	
	pRangeParam->anchor = anchor;
	pRangeParam->width = width;

	pAggContext->pRangeParam = pRangeParam;	// only part of this structure is assigned with value
	bSuccess = true;
Exit0:
	pReader->MoveToElement();
	if (!bSuccess) {
		DELETE_POINTER(pRangeParam);
		pAggContext->pRangeFunc = NULL;
	}
	return bSuccess;
}

bool AggregatorManagerHelper::ReadElementTimeRangeFunc(IXmlReader* pReader, AggregatorContext* pAggContext, AggregatorManager* pAggMgr) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	bool bSuccess = false;
	TimestampRangeParam* pRangeParam = NULL;
	RETURN_ON_FAIL(NULL != (pRangeParam = new TimestampRangeParam));
	
	// extract anchor & width attribute values.
	const TCHAR* value;
	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeAutoAnchor, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	PROCESS_ERROR(BaseUtils::Timestamp::ParseTimestampFromString(value, &pRangeParam->anchor));

	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeAutoWidth, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	PROCESS_ERROR(BaseUtils::TimeRange::ParseTimeRangeFromString(value, &pRangeParam->width));

	PROCESS_ERROR(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName, \
		AggregatorManager::s_szRangeFuncDefaultNameTime, &(pAggContext->pRangeFunc)));

	pAggContext->pRangeParam = pRangeParam;	// only part of this structure is assigned with value

	bSuccess = true;
Exit0:
	pReader->MoveToElement();
	if (!bSuccess) {
		DELETE_POINTER(pRangeParam);
		pAggContext->pRangeFunc = NULL;
	}
	return bSuccess;
}

bool AggregatorManagerHelper::ReadElementStringRangeFunc(IXmlReader* pReader, AggregatorContext* pAggContext, AggregatorManager* pAggMgr) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	bool bSuccess = false;
	StringRangeParam* pRangeParam = NULL;
	RETURN_ON_FAIL(NULL != (pRangeParam = new StringRangeParam));
	
	// extract anchor & width attribute values.
	const TCHAR* value;
	bool bCaseSensitive = true;	// true by default
	if (S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeFuncStringCaseSensitive, NULL)) {
		PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
		bCaseSensitive = SAME_TSTR(value, AggregatorManager::s_szAttrValTrue);
	}

	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeFuncStringNumberOfCompChar, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
	int nCompCharsNum = _ttoi(value);
	PROCESS_ERROR(errno != EINVAL && errno != ERANGE);

	bool bReversedComp = true;	// true by default
	if (S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrRangeFuncStringReversedComp, NULL)) {
		PROCESS_ERROR(S_OK == pReader->GetValue(&value, NULL));
		bReversedComp = SAME_TSTR(value, AggregatorManager::s_szAttrValTrue);
	}

	pRangeParam->case_sensitive = bCaseSensitive;
	pRangeParam->reversed_comp = bReversedComp;
	pRangeParam->char_num_to_comp = nCompCharsNum;

	PROCESS_ERROR(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName,
		AggregatorManager::s_szRangeFuncDefaultNameString, &(pAggContext->pRangeFunc)));

	pAggContext->pRangeParam = pRangeParam;	// only part of this structure is assigned with value
	bSuccess = true;
Exit0:
	pReader->MoveToElement();
	if (!bSuccess) {
		DELETE_POINTER(pRangeParam);
		pAggContext->pRangeFunc = NULL;
	}
	return bSuccess;
}

bool AggregatorManagerHelper::ReadElementAggFunc(IXmlReader* pReader, AggregatorContext* pAggContext, AggregatorManager* pAggMgr) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrAggFuncType, NULL));
	const TCHAR* value;
	RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));

	if (SAME_TSTR(value, AggregatorManager::s_szAttrValAggFuncTypeCount)) {	// aggregate type is count
		RETURN_ON_FAIL(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName,
			AggregatorManager::s_szAggFuncNameCount, &(pAggContext->pAggFunc)));
		pAggContext->aggType = AGG_COUNT;
	} else if (SAME_TSTR(value, AggregatorManager::s_szAttrValAggFuncTypeMin)) {	// Min
		RETURN_ON_FAIL(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName,
			AggregatorManager::s_szAggFuncNameMin, &(pAggContext->pAggFunc)));
		pAggContext->aggType = AGG_MIN;
	} else if (SAME_TSTR(value, AggregatorManager::s_szAttrValAggFuncTypeMax)) {	// Max
		RETURN_ON_FAIL(pAggMgr->LoadExternalFunction(AggregatorManager::s_szAttrValDefaultImageName,
			AggregatorManager::s_szAggFuncNameMax, &(pAggContext->pAggFunc)));
		pAggContext->aggType = AGG_MAX;
	} else
		return false;

	pReader->MoveToElement();
	return true;
}

bool AggregatorManagerHelper::ReadElementAccumulate(IXmlReader* pReader, AggregatorContext* pAggContext) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL);
	RETURN_ON_FAIL(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrAccumulateDirection, NULL));
	const TCHAR* value = NULL;
	RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));

	if (SAME_TSTR(value, AggregatorManager::s_szAttrValDirectionLeft)) {	// left accumulation
		pAggContext->direction = Aggregator::accu_left;
	} else if (SAME_TSTR(value, AggregatorManager::s_szAttrValDirectionRight)) {	// right accumulation
		pAggContext->direction = Aggregator::accu_right;
	} else {	// none accumulation by default
		pAggContext->direction = Aggregator::accu_none;
	}

	pReader->MoveToElement();
	return true;
}

bool AggregatorManagerHelper::ReadElementField(IXmlReader* pReader, AggregatorContext* pAggContext, AggregatorManager* pAggMgr) {
	RETURN_ON_FAIL(pReader != NULL && pAggContext != NULL && pAggContext->pCondList != NULL);
	bool bSuccess = false;

	const TCHAR* szID = NULL;
	TCHAR* szCopiedID = NULL;	// make a deep copy for the ID value
	const TCHAR* szImageName = NULL;
	const TCHAR* szFuncName = NULL;
	const TCHAR* szPreferedValue = NULL;
	const TCHAR* szParams = NULL;
	TCHAR* szCopiedParams = NULL;	// make a deep copy for the parameter value
	FIELD_CONDITION* pFieldCond = NULL;
	PROCESS_ERROR(NULL != (pFieldCond = new FIELD_CONDITION));

	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrID, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&szID, NULL));
	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrImageName, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&szImageName, NULL));
	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrFuncName, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&szFuncName, NULL));
	PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrFuncParam, NULL));
	PROCESS_ERROR(S_OK == pReader->GetValue(&szParams, NULL));
	
	pFieldCond->rtn_val = true;	// prefered value is true by default
	if (S_OK == pReader->MoveToAttributeByName(AggregatorManager::s_szAttrCFPreferedValue, NULL)) {
		PROCESS_ERROR(S_OK == pReader->GetValue(&szPreferedValue, NULL));
		pFieldCond->rtn_val = SAME_TSTR(szPreferedValue, AggregatorManager::s_szAttrValTrue);
	}

	int nIDLen = _tcslen(szID) + 1;
	RETURN_ON_FAIL(nIDLen > 1);	// at least the length ID must has 2 character, with the null-teminator added up
	PROCESS_ERROR(NULL != (szCopiedID = new TCHAR[nIDLen]));
	PROCESS_ERROR(0 == _tcscpy_s(szCopiedID, nIDLen, szID));
	pFieldCond->field_id = szCopiedID;
	
	int nParamLen = _tcslen(szParams) + 1;
	RETURN_ON_FAIL(nIDLen > 1);	// at least the length ID must has 2 character, with the null-teminator added up
	PROCESS_ERROR(NULL != (szCopiedParams = new TCHAR[nParamLen]));
	PROCESS_ERROR(0 == _tcscpy_s(szCopiedParams, nIDLen, szParams));
	pFieldCond->params = szCopiedParams;

	// Now try to get the field condition functions
	// Something
	FIELD_CONDITION_FUNC pFunc;
	PROCESS_ERROR(pAggMgr->LoadExternalFunction(szImageName, szFuncName, &pFunc));
	pFieldCond->pCondFunc = pFunc;

	pAggContext->pCondList->push_back(pFieldCond);
	bSuccess = true;
Exit0:
	if (!bSuccess) {
		DELETE_POINTER(pFieldCond);
		DELETE_ARRAY_POINTER(szCopiedID);
		DELETE_ARRAY_POINTER(szCopiedParams);
	}
	pReader->MoveToElement();
	return bSuccess;
}

