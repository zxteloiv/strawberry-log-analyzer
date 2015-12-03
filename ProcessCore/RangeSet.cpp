// RangeSet.cpp
// Implements the RangeSet class.
//

#include "stdafx.h"
#include "RangeSet.h"

#include "range.h"
#include <set>
#include <utility>

using namespace ProcessCore;

bool RangeSetLess::operator () (const _Range_Ptr p1, const _Range_Ptr p2) const {
	return (CompYes == p1->IsLessThan(p2));
}

bool RangeSet::FindOrInsertRange(ProcessCore::_Range_Ptr& pRange) {
	std::pair< RangeSet::iterator, bool > pr = this->insert(pRange);
	pRange = *(pr.first);
	return (pr.second);
}

_Range_Ptr RangeSet::FindLeft(_Range_Ptr pRange) {
	std::pair< RangeSet::iterator, bool > pr = this->insert(pRange);
	RangeSet::iterator iter = pr.first;
	if (!(pr.second)) {
		--iter;
		// returns NULL if there's no range on the right any more
		return ((iter == this->end()) ? NULL : (*iter));
	} else {
		this->erase(iter);
	}
	return NULL;
}

_Range_Ptr RangeSet::FindRight(_Range_Ptr pRange) {
	std::pair< RangeSet::iterator, bool > pr = this->insert(pRange);
	RangeSet::iterator iter = pr.first;
	if (!(pr.second)) {
		++iter;
		// returns NULL if there's no range on the right any more
		return ((iter == this->end()) ? NULL : (*iter));
	} else {
		this->erase(iter);
	}
	return NULL;
}

int RangeSet::GetContainerCount() {
	return this->size();
}
