// RangeSet.h
// Define the RangeSet class which is inherited from the IRangeContainer interface.
//

#pragma once

#include "CoreDeclaration.h"
#include <set>

#include "IRangeContainer.h"

namespace ProcessCore {
	struct RangeSetLess {
		bool operator() (const _Range_Ptr p1, const _Range_Ptr p2) const;
	};

	class RangeSet : public std::set<_Range_Ptr, RangeSetLess>, public IRangeContainer {
	public:
		virtual bool FindOrInsertRange(_Range_Ptr& pRange);
		virtual _Range_Ptr FindLeft(_Range_Ptr pRange);
		virtual _Range_Ptr FindRight(_Range_Ptr pRange);
		virtual int GetContainerCount();
	};
};
