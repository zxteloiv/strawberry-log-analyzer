// Aggregator.h
// Definition of Aggregator class which is in charge of accumulating different
// types of records.
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"

#include "CoreDeclaration.h"

namespace ProcessCore {
	struct FIELD_CONDITION {
		const TCHAR*			field_id;
		const TCHAR*			params;
		FIELD_CONDITION_FUNC	pCondFunc;	// condition function for that field
		bool					rtn_val;	// preferred returned value for condition function

	public:
		FIELD_CONDITION() : field_id(NULL), params(NULL), pCondFunc(NULL), rtn_val(true) {}
		~FIELD_CONDITION() {
			DELETE_ARRAY_POINTER(field_id);
			DELETE_ARRAY_POINTER(params);
		}
		
	private:
		DISALLOW_COPY_AND_ASSIGN(FIELD_CONDITION);
	};

	enum AGGREGATE_TYPE {
		AGG_COUNT,
		AGG_MAX,
		AGG_MIN
	};

	class Aggregator {
		// =============================================
		// Inner declarations

		// Accumulated direction is used when distribute a record.
		// For instance, Left Accumulation means when a record is distributed into a range,
		// since ranges are never overlapped, that record is also distributed into all the
		// ranges less than it.
		// The term "left" or "right" means other ranges are at the "left" or "right" side
		// to the range on a range axis. And left ranges are always less than right ranges.
	public:
		enum ACCUMULATION_DIRECTION {
			accu_none,
			accu_left,	
			accu_right
		};

		// =============================================
		// Interfaces
		
		// Constructors and destructors
	protected:
		Aggregator(
			const TCHAR*				szBasefield,
			LPCONDLIST					pCondList,
			RANGE_FUNC					pRangeFunc,
			RangeParam*					pRangeParam,
			AGGREGATE_FUNC				pAggFunc,
			AGGREGATE_TYPE				aggType,
			const QuestionTreeNode*		pQTNode,
			AggregatorManager*			pAggMgr,
			ACCUMULATION_DIRECTION		direction
			);
	public:
		virtual ~Aggregator();
		bool Init(const QuestionTreeNode* pQTNode = NULL, AggregatorManager* const pAggMgr = NULL);

		// Public interfaces
	public:
		bool ReceiveRecord(const BaseUtils::_Record_Ptr pRecord);
		bool ResetAggregateValue();

		IRangeContainer* GetContainer() { return m_pRangeContainer; }

		// Protected functions
	protected:
		bool CheckForCondition(const BaseUtils::_Record_Ptr pRecord, bool* pIsAccepted);
		bool ExtractBaseField(const BaseUtils::_Record_Ptr pRecord, BaseUtils::Field** ppBaseField);
		bool DistributeBaseField(const BaseUtils::_Field_Ptr pField, Range** ppRange);
		BaseUtils::_Record_Ptr FindOrInsertRange(BaseUtils::_Record_Ptr);

		// ==============================================
		// Data members
	protected:
		const TCHAR*					m_szBaseField;	// string indicator the field, with the pattern "id[:sub-id]"
		
		// This pointer may be assigned with another pointer to a new Condition List or can be set NULL.
		// Use the detailed type rather then the typedef-ed LPCONDLIST.
		// Recall that the LPCONDLIST is exactly the std::list<FIELD_CONDITION*>* type as defined in "CoreDeclaration.h".
		LPCONDLIST					m_pConditionList;

		IRangeContainer*			m_pRangeContainer;

		RANGE_FUNC					m_pFuncRange;
		RangeParam*					m_pRangeParam;
		
		AGGREGATE_FUNC				m_pFuncAgg;
		AGGREGATE_TYPE				m_aggType;

		const QuestionTreeNode*		m_pQuestionNode;	// the position in the question tree belongs to this aggregator
		AggregatorManager*			m_pMgr;		// the manager

		ACCUMULATION_DIRECTION		m_direction;

		// This friend class declaration is used for AggregatorManager to retrieve protected members
		// to initialize a new Aggregator object.
		friend class AggregatorManager;
		
	};
};
