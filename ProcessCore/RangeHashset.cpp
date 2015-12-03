// RangeHashset.cpp
// Implement the RangeHashset class.
//

#include "stdafx.h"
#include "RangeHashset.h"

#include "range.h"
#include <utility>
using namespace ProcessCore;

size_t RangeHashsetHashFunc::operator ()(const _Range_Ptr p) const {
	size_t size = reinterpret_cast<size_t>(const_cast<_Range_Ptr>(p));
	return size;
}

bool RangeHashsetEquals::operator ()(const _Range_Ptr p1, const _Range_Ptr p2) const {
	return (CompYes == p1->EqualsTo(p2));
}

bool RangeHashset::FindOrInsertRange(ProcessCore::_Range_Ptr& pRange) {
	std::pair<RangeHashset::iterator, bool> pr = this->insert(pRange);
	pRange = *(pr.first);
	return (pr.second);
}

_Range_Ptr RangeHashset::FindLeft(_Range_Ptr pRange) {
	return NULL;
}

_Range_Ptr RangeHashset::FindRight(_Range_Ptr pRange) {
	return NULL;
}

int RangeHashset::GetContainerCount() {
	return this->size();
}
