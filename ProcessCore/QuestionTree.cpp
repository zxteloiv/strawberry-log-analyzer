// QuestionTree.cpp
// Contains implementation for classes related to question trees.
//

#include "stdafx.h"
#include "questiontree.h"

#include <stack>

#include "..\\Toolkit\\Toolkit.h"

#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

using namespace ProcessCore;
using namespace Toolkit;

// strings related to XML parsing
const TCHAR* QuestionTree::s_szQuestionElem = _T("question");
const TCHAR* QuestionTree::s_szRangeAggElem = _T("range_agg");
const TCHAR* QuestionTree::s_szResultAggElem = _T("result_agg");
const TCHAR* QuestionTree::s_szQuestionIDAttr = _T("id");
const TCHAR* QuestionTree::s_szQuestionTextAttr = _T("text");
const TCHAR* QuestionTree::s_szAggIDAttr = _T("agg_id");
const TCHAR* QuestionTree::s_szHasRangeAggAttr = _T("has_range_agg");
const TCHAR* QuestionTree::s_szHasResultAggAttr = _T("has_result_agg");
const TCHAR* QuestionTree::s_szTrueValue = _T("true");
const TCHAR* QuestionTree::s_szFalseValue = _T("false");
const TCHAR* QuestionTree::s_szSaveRangeCountAttr = _T("save_range_count");
const TCHAR* QuestionTree::s_szSaveRangeValueAttr = _T("save_range_value");


// other strings
const TCHAR* QuestionTree::s_szEmptyString = _T("");

#define SAME_TSTR(X,Y)	(0 == _tcscmp((X), (Y)))

bool ReadXmlForNode(IXmlReader* pReader, QuestionTreeNode* pNode, LPNODESTACK pLeftNodeStack, LPNODESTACK pRightNodeStack);

QuestionTree::QuestionTree() {
	m_pRoot = NULL;
	m_pText = const_cast<TCHAR*>(s_szEmptyString);
}

QuestionTree::~QuestionTree() {
	if (m_pText != s_szEmptyString) {
		delete[] m_pText;
	}

	std::stack<QuestionTreeNode*> traverse_stack;
	QuestionTreeNode* pCursor = m_pRoot;
	traverse_stack.push(pCursor);
	while (traverse_stack.size() > 0)  {
		pCursor = traverse_stack.top();
		traverse_stack.pop();
		if (pCursor->m_pRangeAggNode != NULL) {
			traverse_stack.push(pCursor->m_pRangeAggNode);
		}
		if (pCursor->m_pResultAggNode != NULL) {
			traverse_stack.push(pCursor->m_pResultAggNode);
		}
		delete pCursor;
	}
}

void QuestionTree::PrintTree(PRINT_FUNC pFunc) {
	TCHAR szMsg[MAX_PATH] = _T("");

	_stprintf_s(szMsg, MAX_PATH, L"%s: ", m_pText);
	pFunc(szMsg);

	std::stack<QuestionTreeNode*> traverse_stack;
	QuestionTreeNode* pCursor = m_pRoot;
	traverse_stack.push(pCursor);
	while (traverse_stack.size() > 0)  {
		pCursor = traverse_stack.top();
		traverse_stack.pop();
		_stprintf_s(szMsg, MAX_PATH, L"%d ", pCursor->m_nAggID);
		pFunc(szMsg);
		if (pCursor->m_pRangeAggNode != NULL) {
			traverse_stack.push(pCursor->m_pRangeAggNode);
		}
		if (pCursor->m_pResultAggNode != NULL) {
			traverse_stack.push(pCursor->m_pResultAggNode);
		}
	}
	pFunc(L"\r\n");
}

bool QuestionTree::SetQuestionText(const TCHAR *str, int len) {
	if (m_pText != s_szEmptyString) {
		delete[] m_pText;
	}

	// when len == 0, it means to empty the string
	if (0 == len) {
		m_pText = const_cast<TCHAR*>(s_szEmptyString);
		return true;
	}

	int real_len = _tcslen(str);
	// If len < 0, it means to copy the whole text. But when len > 0, even larger than the length
	// of the given text, it means to copy the designated part of whole part of the text.
	len = ((len < 0) ? real_len : min(real_len, len));

	RETURN_ON_FAIL(NULL != (m_pText = new TCHAR[len + 1]));	
	PROCESS_ERROR(0 == _tcsncpy_s(m_pText, len + 1, str, len));

	return true;
Exit0:
	DELETE_ARRAY_POINTER(m_pText);
	m_pText = const_cast<TCHAR*>(s_szEmptyString);
	return false;
}

// static methods implementation
bool QuestionTree::BuildQTreeListFromXML(const TCHAR *xml_string, ProcessCore::LPQUESTLIST *ppQTreeList) {
	IXmlReader* pReader = NULL;
	ISequentialStream* pStream = NULL;
	LPNODESTACK pLeftNodeStack = NULL;
	LPNODESTACK pRightNodeStack = NULL;
	QuestionTree* pCurrentTree = NULL;
	LPQUESTLIST pQTreeList = NULL;

	HRESULT hr = S_OK;
	PROCESS_ERROR(S_OK == (hr = CStringStream::Create(xml_string, &pStream)));
	PROCESS_ERROR(S_OK == (hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)));
	PROCESS_ERROR(S_OK == (hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)));
	PROCESS_ERROR(S_OK == (hr = pReader->SetInput(pStream)));

	XmlNodeType nodeType;
	PROCESS_ERROR(NULL != (pQTreeList = new QUESTLIST));
	pLeftNodeStack = new NODESTACK;
	pRightNodeStack = new NODESTACK;
	PROCESS_ERROR(NULL != pLeftNodeStack && NULL != pRightNodeStack);
	const TCHAR* localName = NULL;
	while (S_OK == (hr = pReader->Read(&nodeType))) {
		PROCESS_ERROR(S_OK == (hr = pReader->GetLocalName(&localName, NULL)));
		if (nodeType == XmlNodeType_Element) {
			if (SAME_TSTR(localName, s_szQuestionElem)) {	// <question>
				// A new question tree is needed
				PROCESS_ERROR(NULL == pCurrentTree && NULL != (pCurrentTree = new QuestionTree));
				const TCHAR* text = NULL;	// text of this new tree
				PROCESS_ERROR(S_OK == (hr = pReader->MoveToAttributeByName(s_szQuestionTextAttr, NULL)));
				PROCESS_ERROR(S_OK == (hr = pReader->GetValue(&text, NULL)));
				PROCESS_ERROR(pCurrentTree->SetQuestionText(text, SET_ENTIRE_STRING));

				// Initialize the root node of this tree
				PROCESS_ERROR(NULL != (pCurrentTree->m_pRoot = new QuestionTreeNode));
				QuestionTreeNode* pRoot = pCurrentTree->m_pRoot;

				PROCESS_ERROR(ReadXmlForNode(pReader, pRoot, pLeftNodeStack, pRightNodeStack));

				if (pReader->IsEmptyElement()) {	// no more aggregators
					pQTreeList->push_back(pCurrentTree);
					pCurrentTree = NULL;
				}
			} else if (SAME_TSTR(localName, s_szRangeAggElem)) {	// <range_agg>
				// Each time for a <range_agg> element, read the top of the left node stack, assign the members
				// of the node by parsing the attributes of <range_agg>. And if the range_agg has inner aggregatos,
				// push them to the left or right node stack.
				PROCESS_ERROR(0 < pLeftNodeStack->size());
				QuestionTreeNode* pNode = pLeftNodeStack->top();
				pLeftNodeStack->pop();	// pop out itself

				PROCESS_ERROR(ReadXmlForNode(pReader, pNode, pLeftNodeStack, pRightNodeStack));
			} else if (SAME_TSTR(localName, s_szResultAggElem)) {	// <result_agg>, similar to <range_agg> above
				PROCESS_ERROR(0 < pRightNodeStack->size());
				QuestionTreeNode* pNode = pRightNodeStack->top();
				pRightNodeStack->pop();	// pop out itself

				PROCESS_ERROR(ReadXmlForNode(pReader, pNode, pLeftNodeStack, pRightNodeStack));
			}	// end-if. Other elements except the three above will not be processed
		} else if (nodeType == XmlNodeType_EndElement) {
			if (SAME_TSTR(localName, s_szQuestionElem)) {	// </question>
				// a question definition ends, push this tree to the treelist
				PROCESS_ERROR(pLeftNodeStack->size() == 0 && pRightNodeStack->size() == 0);
				pQTreeList->push_back(pCurrentTree);
				pCurrentTree = NULL;
			}
		}
	}
	
	*ppQTreeList = pQTreeList;
	if (pReader != NULL) pReader->Release();
	if (pStream != NULL) pStream->Release();
	DELETE_POINTER(pLeftNodeStack);
	DELETE_POINTER(pRightNodeStack);
	return true;
Exit0:
	if (pReader != NULL) pReader->Release();
	if (pStream != NULL) pStream->Release();
	DELETE_POINTER(pCurrentTree);
	RELEASE_STD_CONTAINER_CONTENT(QUESTLIST, pQTreeList);
	DELETE_POINTER(pQTreeList);
	DELETE_POINTER(pLeftNodeStack);
	DELETE_POINTER(pRightNodeStack);
	return false;
}

bool ReadXmlForNode(IXmlReader *pReader, QuestionTreeNode *pNode, LPNODESTACK pLeftNodeStack, LPNODESTACK pRightNodeStack) {
	HRESULT hr = S_OK;
	const TCHAR* agg_id_str = NULL;
	const TCHAR* has_left_str = NULL;	// left node is RangeAgg of this agg
	const TCHAR* has_right_str = NULL;	// right node is ResultAgg of this agg
	
	// aggregator id must exist.
	RETURN_ON_FAIL(S_OK == (hr = pReader->MoveToAttributeByName(QuestionTree::s_szAggIDAttr, NULL)));
	RETURN_ON_FAIL(S_OK == (hr = pReader->GetValue(&agg_id_str, NULL)));
	pNode->m_nAggID = _ttoi(agg_id_str);

	pNode->m_bSaveRangeCount = false;
	if (S_OK == (hr = pReader->MoveToAttributeByName(QuestionTree::s_szSaveRangeCountAttr, NULL))) {
		const TCHAR* value = NULL;
		RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));
		pNode->m_bSaveRangeCount = SAME_TSTR(value, QuestionTree::s_szTrueValue);
	}
	pNode->m_bSaveRangeValue = false;
	if (S_OK == (hr = pReader->MoveToAttributeByName(QuestionTree::s_szSaveRangeValueAttr, NULL))) {
		const TCHAR* value = NULL;
		RETURN_ON_FAIL(S_OK == pReader->GetValue(&value, NULL));
		pNode->m_bSaveRangeValue = SAME_TSTR(value, QuestionTree::s_szTrueValue);
	}

	// Left and right node of the RangeAgg & ResultAgg can be undefined, leaving the default value of false.
	if (S_OK == (hr = pReader->MoveToAttributeByName(QuestionTree::s_szHasRangeAggAttr, NULL))) {
		RETURN_ON_FAIL(S_OK == (hr = pReader->GetValue(&has_left_str, NULL)));
		if (SAME_TSTR(has_left_str, QuestionTree::s_szTrueValue)) {	// has_range_agg = "true"
			RETURN_ON_FAIL(NULL != (pNode->m_pRangeAggNode = new QuestionTreeNode));
			pLeftNodeStack->push(pNode->m_pRangeAggNode);
		}
	}
	if (S_OK == (hr = pReader->MoveToAttributeByName(QuestionTree::s_szHasResultAggAttr, NULL))) {
		RETURN_ON_FAIL(S_OK == (hr = pReader->GetValue(&has_right_str, NULL)));
		if (SAME_TSTR(has_right_str, QuestionTree::s_szTrueValue)) {	// has_result_agg = "true"
			RETURN_ON_FAIL(NULL != (pNode->m_pResultAggNode = new QuestionTreeNode));
			pRightNodeStack->push(pNode->m_pResultAggNode);
		}
	}

	// move back to element
	RETURN_ON_FAIL(S_OK == (hr = pReader->MoveToElement()));
	return true;
}

void QuestionTree::CleanQTreeList(ProcessCore::LPQUESTLIST* ppQTree) {
	RELEASE_STD_CONTAINER_CONTENT(QUESTLIST, *ppQTree);
	DELETE_POINTER(*ppQTree);
}
