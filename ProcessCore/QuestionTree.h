// QuestionTree.h
// Contains declarations of classes forming the question tree.
// Questions are what users want to know about their log. And a question tree is an aggregator
// organization.
//

#pragma once

#include "CoreDeclaration.h"

namespace ProcessCore {
	struct QuestionTreeNode {
		int		m_nAggID;
		bool	m_bSaveRangeCount;
		bool	m_bSaveRangeValue;
		QuestionTreeNode*	m_pResultAggNode;
		QuestionTreeNode*	m_pRangeAggNode;

		// constructor for easier use
		QuestionTreeNode() : m_nAggID(0), m_pResultAggNode(NULL), m_pRangeAggNode(NULL),
			m_bSaveRangeValue(false), m_bSaveRangeCount(false) {}
	};

	class QuestionTree {
		// ==================================================
		// Interfaces

		// Constructors & destructors
	protected:
		QuestionTree();
	public:
		virtual ~QuestionTree();		
	private:
		DISALLOW_COPY_AND_ASSIGN(QuestionTree);

		// Public Interfaces
	public:
		const TCHAR* GetQuestionText() const { return m_pText; }
		const QuestionTreeNode* GetRootNode() const { return m_pRoot; }
		bool SetQuestionText(const TCHAR* str, int len);
		void PrintTree(PRINT_FUNC pFunc);

		// ==================================================
		// Data members
	protected:
		QuestionTreeNode*	m_pRoot;
		TCHAR*				m_pText;

		// ==================================================
		// Static members
	public:
		// strings related to XML parsing
		static const TCHAR* s_szQuestionElem;
		static const TCHAR* s_szRangeAggElem;
		static const TCHAR* s_szResultAggElem;
		static const TCHAR* s_szQuestionIDAttr;
		static const TCHAR* s_szQuestionTextAttr;
		static const TCHAR* s_szAggIDAttr;
		static const TCHAR*	s_szSaveRangeCountAttr;
		static const TCHAR*	s_szSaveRangeValueAttr;
		static const TCHAR* s_szHasRangeAggAttr;
		static const TCHAR* s_szHasResultAggAttr;
		static const TCHAR* s_szTrueValue;
		static const TCHAR* s_szFalseValue;
		
		// other strings
		static const TCHAR* s_szEmptyString;

		// other static members
		static const int SET_ENTIRE_STRING = -1;
		static const int SET_EMPTY_STRING = 0;

		// factory method
	public:
		static bool BuildQTreeListFromXML(const TCHAR* xml_string, LPQUESTLIST* ppQTreeList);
		static void CleanQTreeList(LPQUESTLIST* ppQTree);

	};
};

