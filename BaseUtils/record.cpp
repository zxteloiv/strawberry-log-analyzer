// record.cpp
// Implements the Record class
//

#include "stdafx.h"
#include "record.h"
#include <ctype.h>
#include <errno.h>	// for errno check in Record::ParseFieldStr()

#pragma warning( disable : 4018 )	// disable waring for '<' signed/unsigned mismatch

using namespace BaseUtils;

Record::Record() : m_pRecordHolder(NULL) {
}

Record::~Record() {
	RELEASE_STD_CONTAINER_CONTENT(std::vector<Field*>, m_pRecordHolder);
	DELETE_POINTER(m_pRecordHolder);
}

bool Record::Init() {
	m_pRecordHolder = new std::vector<Field*>;
	return (m_pRecordHolder != NULL);
}

_Field_Ptr Record::GetField(int index) const {
	if (index < m_pRecordHolder->size()) {
		return m_pRecordHolder->at(index);
	}
	return NULL;
}

_Field_Ptr Record::GetChildField(int index, int subindex) const {
	if (index < m_pRecordHolder->size()) {
		_Field_Ptr pField = m_pRecordHolder->at(index);
		return pField->GetChildField(subindex);
	}
	return NULL;
}

_Field_Ptr Record::GetField(const TCHAR *str) const {
	// parse the string to extract index and subindex
	int index = -1;
	int subindex = -1;

	if (Record::ParseFieldStr(str, &index, &subindex)) {
		if (-1 == subindex) {	// no user-defined subindex
			return GetField(index);
		} else {
			return GetChildField(index, subindex);
		}
	} else {
		return NULL;
	}
}

bool Record::AddField(Field* pField) {
	RETURN_ON_FAIL(m_pRecordHolder != NULL);
	m_pRecordHolder->push_back(pField);
	pField->m_pRecordBelongedTo = this;
	return true;
}

int Record::GetFieldCount() const {
	return m_pRecordHolder->size();
}

bool Record::ParseFieldStr(const TCHAR* str, int* pIndex, int* pSubindex) {
	int len = _tcslen(str);
	
	int cursor = 0;
	// find index
	while (_istdigit(str[cursor]) && cursor < len) {
		cursor++;
	}

	TCHAR indexStr[10] = _T("");	// 10 is large enough for hold an index number
	if (0 != _tcsncpy_s(indexStr, 10, str, cursor)) {	// string copy failed
		return false;
	}
	int tempInt = _ttoi(indexStr);
	if (errno == ERANGE) {	// failed to convert a string to integer
		return false;
	} else {
		*pIndex = tempInt;	// successfully converted the main index
	}

	// skip the delimiters
	while (!_istdigit(str[cursor]) && cursor < len) {
		cursor++;
	}
	if (cursor == len) {	// no integer contained any more, therefore no need to find subindex
		return true;	// up till now only the index was set
	}

	// find subindex
	int sub_begin_cursor = cursor;
	while (_istdigit(str[cursor]) && cursor < len) {
		cursor++;
	}
	if (0 != _tcsncpy_s(indexStr + sub_begin_cursor, 10, str, cursor - sub_begin_cursor)) {
		return false;	// failed to copy subindex string
	}
	tempInt = _ttoi(indexStr);
	if (errno == ERANGE) {
		return false;
	} else {
		*pSubindex = tempInt;
	}

	return true;
}
