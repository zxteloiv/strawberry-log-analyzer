// CoreDeclaration.h
// Declarations for all the classes in Process Core, together with their typedef-ed types
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"

#include <vector>
#include <list>
#include <queue>
#include <stack>

namespace ProcessCore {
	class AggregatorManager;
	class AggregatorManagerHelper;
	class Aggregator;
	typedef std::vector<Aggregator*>	AGGVECTOR;
	typedef std::vector<Aggregator*>*	LPAGGVECTOR;
	typedef std::vector<Aggregator*>::iterator	AGGVECTITER;
	typedef std::list<Aggregator*>		AGGLIST;
	typedef std::list<Aggregator*>*		LPAGGLIST;
	typedef std::list<Aggregator*>::iterator	AGGLISTITER;
	typedef std::vector<HMODULE>		MODULELIST;
	typedef std::vector<HMODULE>*		LPMODULELIST;
	typedef std::vector<HMODULE>::iterator		MODULEITER;

	struct FIELD_CONDITION;
	typedef std::list<FIELD_CONDITION*>		CONDLIST;
	typedef std::list<FIELD_CONDITION*>*	LPCONDLIST;
	typedef std::list<FIELD_CONDITION*>::iterator	CONDITER;
	typedef std::list<FIELD_CONDITION*>::const_iterator	CONSTCONDITER;

	union AggregateValue;
	typedef bool (WINAPI *AGGREGATE_FUNC)(BaseUtils::_Field_Ptr, AggregateValue*);

	enum AGGREGATE_TYPE;
	
	class Range;
	// the derived classes need not to be declared together with their base classes
	class IntRange;
	class RealRange;
	class StringRange;
	class TimestampRange;
	typedef Range* _Range_Ptr;
	struct RangeParam;
	struct IntRangeParam;
	struct RealRangeParam;
	struct StringRangeParam;
	struct TimestampRangeParam;
	struct CustomizedRangeParam;
	typedef	bool (WINAPI *FIELD_CONDITION_FUNC)(const BaseUtils::_Field_Ptr, const TCHAR*);
	typedef	bool (WINAPI *RANGE_FUNC)(const BaseUtils::_Field_Ptr, const RangeParam*, Aggregator*, Range**);
	enum CompRangeReturn;
	
	struct QuestionTreeNode;
	class QuestionTree;
	typedef std::list<QuestionTree*>			QUESTLIST;
	typedef std::list<QuestionTree*>*			LPQUESTLIST;
	typedef std::list<QuestionTree*>::iterator	QUESTITER;
	typedef std::stack<QuestionTreeNode*>		NODESTACK;
	typedef std::stack<QuestionTreeNode*>*		LPNODESTACK;
	typedef void (*PRINT_FUNC)(const TCHAR*);


	struct JobThreadParam;
	class JobScheduler;
	class Worker;
	class RecordDepository;
	typedef std::queue<BaseUtils::_Record_Ptr>	RECQUEUE;
	typedef std::queue<BaseUtils::_Record_Ptr>*	LPRECQUEUE;

	class IRangeContainer;
	class RangeHashset;
	class RangeSet;

	struct RangeHashsetHashFunc;
	struct RangeHashsetEquals;
	struct RangeSetLess;

	class TcpClientBuf;
	class TcpTranslator;

	class DBAccessLogic;

};
