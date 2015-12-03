// AggregatorManager.h
// Definition for AggregatorManager class, which is responsible for build and manage
// the aggregator graph.
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"
#include "CoreDeclaration.h"

namespace ProcessCore {
	class AggregatorManager {
		// ===============================================
		// Interfaces

		// Constructors and destructors
	public:
		AggregatorManager();
		virtual ~AggregatorManager();
		bool Init();

		// Public interfaces
	public:
		bool BuildAggListFromXML(const TCHAR*);
		void ReleaseDerivedAggs();
		void ReleaseTemplateAggs();
		// create a new aggregator using the corresponding QuestionTreeNode
		bool CreateAggregator(Aggregator**, const QuestionTreeNode*);
		Aggregator* GetAggregatorByID(int) const;

		bool LoadExternalFunction(const TCHAR* szImageName, const TCHAR* szFuncName, RANGE_FUNC* ppFunc);
		bool LoadExternalFunction(const TCHAR* szImageName, const TCHAR* szFuncName, AGGREGATE_FUNC* ppFunc);
		bool LoadExternalFunction(const TCHAR* szImageName, const TCHAR* szFuncName, FIELD_CONDITION_FUNC* ppFunc);

		bool SaveQuestionResult(void* pXmlWriter = NULL);

		// help functions
	protected:
		// create a new aggregator using the aggregator template
		bool _CreateAggregator(Aggregator**, const Aggregator*);

		// save result of a single aggregator
		bool SaveSingleAggregator(Aggregator* pAgg, void* pXmlWriter = NULL);
		bool SaveRangeContentAttr(Range* pRange, void* pXmlWriter = NULL);
		bool SaveRangeValueAttr(Range* pRange, void* pXmlWriter = NULL);
		
		// =============================================
		// Static members

		// Xml Parser related strings. Following is a sample XML string
		/*
		<?xml version="1.0" encoding="ucs-2"?>
		<request>
			<aggregators number="">
				<agg id="">
					<base_field id="" />
					<range_func auto_range="true" func_name="ExternalRoutes.dll" type="int">
						<int anchor="" width=""/>
						<real anchor="" width=""/>
						<datetime anchor="" width=""/>
						<string case_sensitive="" number_of_char_to_comp="" reversed_comp="" />
					</range_func>
					<aggregate_func type="count|min|max"/>
					<accumulate direction="left|right|none" />
					<condition_fields number="">
						<field id="" func_name="" prefered_value="true" />
						<field id="" func_name="" prefered_value="false" />
						<field id="" func_name="" /> <!-- prefered_value = "true" by default -->
					</condition_fields>
				</agg>
			</aggregators>
		</request>
		*/
	public:

#pragma region Strings For XML Parsing
		static const TCHAR* s_szElemAggregator;
		static const TCHAR* s_szAttrID;
		static const TCHAR* s_szElemBaseField;
		static const TCHAR* s_szElemRangeFunc;
		static const TCHAR* s_szAttrRangeFuncAuto;
		static const TCHAR* s_szAttrFuncName;
		static const TCHAR* s_szAttrImageName;
		static const TCHAR* s_szAttrValDefaultImageName;
		static const TCHAR* s_szAttrRangeFuncType;
		static const TCHAR* s_szAttrValRangeFuncTypeInt;
		static const TCHAR* s_szRangeFuncDefaultNameInt;
		static const TCHAR* s_szAttrValRangeFuncTypeReal;
		static const TCHAR* s_szRangeFuncDefaultNameReal;
		static const TCHAR* s_szAttrValRangeFuncTypeTime;
		static const TCHAR* s_szRangeFuncDefaultNameTime;
		static const TCHAR* s_szAttrValRangeFuncTypeString;
		static const TCHAR* s_szRangeFuncDefaultNameString;
		static const TCHAR* s_szElemRangeFuncInt;
		static const TCHAR* s_szElemRangeFuncReal;
		static const TCHAR* s_szElemRangeFuncTimestamp;
		static const TCHAR* s_szAttrRangeAutoAnchor;
		static const TCHAR* s_szAttrRangeAutoWidth;
		static const TCHAR* s_szElemRangeFuncString;
		static const TCHAR* s_szAttrRangeFuncStringCaseSensitive;
		static const TCHAR* s_szAttrRangeFuncStringNumberOfCompChar;
		static const TCHAR* s_szAttrRangeFuncStringReversedComp;
		static const TCHAR* s_szElemAggFunc;
		static const TCHAR* s_szAttrAggFuncType;
		static const TCHAR* s_szAttrValAggFuncTypeCount;
		static const TCHAR* s_szAttrValAggFuncTypeMin;
		static const TCHAR* s_szAttrValAggFuncTypeMax;
		static const TCHAR* s_szAggFuncNameCount;
		static const TCHAR* s_szAggFuncNameMin;
		static const TCHAR* s_szAggFuncNameMax;
		static const TCHAR* s_szElemAccumulate;
		static const TCHAR* s_szAttrAccumulateDirection;
		static const TCHAR* s_szAttrValDirectionLeft;
		static const TCHAR* s_szAttrValDirectionRight;
		static const TCHAR* s_szAttrValDirectionNone;
		static const TCHAR* s_szElemConditionFields;
		static const TCHAR* s_szElemConditionField;
		static const TCHAR* s_szAttrCFPreferedValue;
		static const TCHAR* s_szAttrFuncParam;
		static const TCHAR* s_szAttrValTrue;
		static const TCHAR* s_szAttrValFalse;

#pragma endregion

#pragma region Strings For Writing XML Result
		static const TCHAR* s_szElemAggregatorOutput;
		static const TCHAR* s_szAttrAggIDOutput;
		static const TCHAR* s_szAttrRangeCountOutput;	
		static const TCHAR* s_szElemRangeOutput;
		static const TCHAR* s_szAttrContentOutput;
		static const TCHAR* s_szAttrValueOutput;

#pragma endregion
		// =============================================
		// Data members
	protected:
		LPAGGVECTOR		m_pAggHolder;	// a holder contains all template aggs
		LPAGGLIST		m_pDerivedAggs;	// those aggregators derived from any template
		LPMODULELIST	m_pDllModules;	// a list contains all the opened dll module

		
	};
};
