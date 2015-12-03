// RangeHashset.h
// Definition of the RangeHashset class. It is in charge of contain huge amount of ranges
// in an aggregator with left/right accumulation option disabled.
//

#pragma once

#include "CoreDeclaration.h"

#include <hash_set>
#include "IRangeContainer.h"

namespace ProcessCore {
	struct RangeHashsetHashFunc {
		size_t operator() (const _Range_Ptr p) const;
	};
	struct RangeHashsetEquals {
		bool operator() (const _Range_Ptr p1, const _Range_Ptr p2) const;
	};

	class RangeHashset : public std::hash_set<_Range_Ptr, RangeHashsetHashFunc, RangeHashsetEquals>, public IRangeContainer {
	public:
		virtual bool FindOrInsertRange(_Range_Ptr& pRange);
		virtual _Range_Ptr FindLeft(_Range_Ptr pRange);
		virtual _Range_Ptr FindRight(_Range_Ptr pRange);
		virtual int GetContainerCount();
	};
};
