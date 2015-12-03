// range.h
// Header for range declaration of base field in aggregator
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"

#include "CoreDeclaration.h"

namespace ProcessCore {
	// AggregateValue is a value that contains the aggregated result, which can be many types according to the
	// chosen aggregate function. For instance, an Aggregate Function of Counting may use an INTEGER(int_value)
	// as its aggregated result and store it in the AggregateValue union.
	// Internal types of AggregateValues depend on what the Aggregate Function is, rather than depend on what
	// the range type is.
	union AggregateValue {
		int						int_value;
		TCHAR*					string_ptr;
		BaseUtils::Real			real_value;
		BaseUtils::Timestamp*	timestamp_ptr;
		//void*	val_collection;
	};

	// CompRangeReturn is used as a return value of comparison of two range object
	enum CompRangeReturn {
		CompYes,
		CompNo,
		CompTypeMismatch,
		CompNullPointerError,
		CompPointerConversionFailed,
		CompUnexpectedError
	};

#define RETURN_YES_WHEN(X) if (X) return CompYes
#define RETURN_NO_WHEN(X) if (X) return CompNo
#define RETURN_TYPEMISMATCH_WHEN(X) if (X) return CompTypeMismatch
#define RETURN_NULLPOINTER_WHEN(X) if (X) return CompNullPointerError
#define RETURN_POINTERCONVERSIONFAILED_WHEN(X) if (X) return CompPointerConversionFailed
#define RETURN_UNEXPECTEDERROR_WHEN(X) if (X) return CompUnexpectedError

	class Range {
		// ============================================
		// Inner declarations

	public:
		// RangeType is used to identify the type of a range, which helps to
		// convert a Range pointer to a derived class object pointer.
		enum RangeType {
			integer_range,
			real_range,
			timestamp_range,
			string_range,
			customized_range,
			reserved_range
		};

		// ============================================
		// Interfaces
		
		// Constructors and destructors
	protected:
		Range(AGGREGATE_TYPE agg_type,
			AGGREGATE_FUNC pFunc = NULL,
			Aggregator* pAgg = NULL);	// no directly range instance allowed

	public:
		virtual ~Range();

		// Public interfaces
	public:
		virtual CompRangeReturn IsLessThan(_Range_Ptr) const = NULL;
		virtual CompRangeReturn EqualsTo(_Range_Ptr) const = NULL;
		
		virtual RangeType GetRangeType() const { return m_rangeType; }
		virtual AGGREGATE_TYPE GetAggType() const { return m_aggType; }

		virtual bool GetRangeAggValue(void**) = NULL;
		virtual bool DoAggregate(BaseUtils::_Field_Ptr) = NULL;
		virtual bool ResetAggregateValue();

		virtual bool SetRangeAggregator(Aggregator* pAgg);

		// ============================================
		// Data members
	protected:
		RangeType		m_rangeType;
		// The aggregated value, manipulated by the aggregate functions only.
		AggregateValue	m_val;
		AGGREGATE_FUNC	m_pFuncAgg;
		AGGREGATE_TYPE	m_aggType;

		// pointer to the aggregator which records distributed to this range shall be aggregated further.
		// It will remain NULL at the beginning until the set function is called.
		Aggregator*		m_pRangeAgg;

	};


};
