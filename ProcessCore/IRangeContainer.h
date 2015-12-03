// IRangeContainer.h
// Define the interface of the Range Container
//

#pragma once

#include "CoreDeclaration.h"

namespace ProcessCore {
	class IRangeContainer {
	public:
		virtual bool FindOrInsertRange(_Range_Ptr& pRange) = NULL;

		virtual _Range_Ptr FindLeft(_Range_Ptr pRange) = NULL;
		virtual _Range_Ptr FindRight(_Range_Ptr pRange) = NULL;

		virtual int GetContainerCount() = NULL;
	};
};
