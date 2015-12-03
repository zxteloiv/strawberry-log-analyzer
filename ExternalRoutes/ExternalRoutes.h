#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXTERNALROUTES_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXTERNALROUTES_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EXTERNALROUTES_EXPORTS
#define EXTERNALROUTES_API __declspec(dllexport)
#else
#define EXTERNALROUTES_API __declspec(dllimport)
#endif


#include "..\\BaseUtils\\BaseUtils.h"
#include "..\\ProcessCore\\ProcessCore.h"


// Aggregate Functions
// Sample:
//		bool (*AGGREGATE_FUNC)(BaseUtils::_Field_Ptr, AggregateValue*);
class AggregateFunctions {
public:
	static bool WINAPI Count(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
	static bool WINAPI Max(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
	static bool WINAPI Min(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
};

// Range Functions
// Sample:
//		typedef	bool (*RANGE_FUNC)(const BaseUtils::_Field_Ptr, const void*, Range**);
class RangeFunctions {
public:
	static bool WINAPI RangeForInt(const BaseUtils::_Field_Ptr, const ProcessCore::RangeParam*,
		ProcessCore::Aggregator*, ProcessCore::Range**);
	static bool WINAPI RangeForReal(const BaseUtils::_Field_Ptr, const ProcessCore::RangeParam*,
		ProcessCore::Aggregator*,  ProcessCore::Range**);
	static bool WINAPI RangeForTime(const BaseUtils::_Field_Ptr, const ProcessCore::RangeParam*,
		ProcessCore::Aggregator*,  ProcessCore::Range**);
	static bool WINAPI RangeForString(const BaseUtils::_Field_Ptr, const ProcessCore::RangeParam*,
		ProcessCore::Aggregator*,  ProcessCore::Range**);
};

// Condition Functions
// Sample:
//		typedef	bool (WINAPI *FIELD_CONDITION_FUNC)(const BaseUtils::_Field_Ptr, const TCHAR*);
class ConditionFunctions {
public:
	static bool WINAPI IntLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI IntEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI IntLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI RealLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI RealEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI RealLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI TimeLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI TimeEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI TimeLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI StringIsEmpty(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI StringLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI StringEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	static bool WINAPI StringLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
};

// -------------------------------------------------------------------------------------------------------------
// Function type definitions in the DLL.
// These types of functions are the same as those defined in the ProcessCore, but in this default dll library
// of ExternalRoutes, these types are used in ProcessCore for casting a function address got by GetProcAddress.
// It is convinced that casting to types defined in the dll can be acceptable by the compiler.
// Although casting to types already defined in ProcessCore directly is also acceptable, it is important to check
// function pointers obtained by the GetProcAddress call is correct.
// We choose to cast pointers to DLL_* types and later use assignment to types defined in the ProcessCore.
// In this way, the compiler will check whether the DLL_* types can be converted to those types.
// Anyway, we must ensure that these DLL_* types match the functions to be exported also.
// But it is convenient to check pointers & types in the same project & namespace of the ExternalRoutes, rather
// than check those between ExternalRoutes functions and ProcessCore function pointer types, isn't it?
// 
// For example, in a code segment of ProcessCore, we write:
//		AGG_FUNC pFunc = (DLL_AGG_FUNC)::GetProcAddress(some_handle, "function_name");
// rather than:
//		AGG_FUNC pFunc = (AGG_FUNC)::GetProcAddress(some_handle, "function_name");

typedef bool (WINAPI *DLL_AGG_FUNC)(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
typedef bool (WINAPI *DLL_RANGE_FUNC)(const BaseUtils::_Field_Ptr, const ProcessCore::RangeParam*, \
		ProcessCore::Aggregator*, ProcessCore::Range**);
typedef bool (WINAPI *DLL_FILTER_FUNC)(const BaseUtils::_Field_Ptr, const TCHAR*);

// -------------------------------------------------------------------------------------------------------------
// Following functions decorated with "extern "C"" are pure C functions to be exported from this DLL.

extern "C" {
	EXTERNALROUTES_API bool WINAPI AggFuncCount(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
	EXTERNALROUTES_API bool WINAPI AggFuncMax(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
	EXTERNALROUTES_API bool WINAPI AggFuncMin(BaseUtils::_Field_Ptr pField, ProcessCore::AggregateValue* pValue);
};

extern "C" {
	EXTERNALROUTES_API bool WINAPI RangeFuncInt(
		const BaseUtils::_Field_Ptr		pField,
		const ProcessCore::RangeParam*	pRangeParam,
		ProcessCore::Aggregator*		pAgg,
		ProcessCore::Range**			ppRange
		);

	EXTERNALROUTES_API bool WINAPI RangeFuncReal(
		const BaseUtils::_Field_Ptr		pField,
		const ProcessCore::RangeParam*	pRangeParam,
		ProcessCore::Aggregator*		pAgg,
		ProcessCore::Range**			ppRange
		);

	EXTERNALROUTES_API bool WINAPI RangeFuncTime(
		const BaseUtils::_Field_Ptr		pField,
		const ProcessCore::RangeParam*	pRangeParam,
		ProcessCore::Aggregator*		pAgg,
		ProcessCore::Range**			ppRange
		);

	EXTERNALROUTES_API bool WINAPI RangeFuncString(
		const BaseUtils::_Field_Ptr		pField,
		const ProcessCore::RangeParam*	pRangeParam,
		ProcessCore::Aggregator*		pAgg,
		ProcessCore::Range**			ppRange
		);

};

extern "C" {
	EXTERNALROUTES_API bool WINAPI IntLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI IntEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI IntLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI RealLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI RealEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI RealLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI TimeLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI TimeEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI TimeLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI StringIsEmpty(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI StringLessThan(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI StringEqualTo(const BaseUtils::_Field_Ptr, const TCHAR*);
	EXTERNALROUTES_API bool WINAPI StringLargerThan(const BaseUtils::_Field_Ptr, const TCHAR*);
};
