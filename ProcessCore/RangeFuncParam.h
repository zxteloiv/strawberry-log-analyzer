// RangeFuncParam.h
// Define the structs that contain parameters for range functions.
//

#pragma once

#include "..\\BaseUtils\\BaseUtils.h"
#include "CoreDeclaration.h"

namespace ProcessCore {
	struct RangeParam {
		AGGREGATE_TYPE agg_type;
		AGGREGATE_FUNC agg_func;
	};

	struct IntRangeParam : public RangeParam {
		int		anchor;
		int		width;
	};
	
	struct RealRangeParam : public RangeParam {
		BaseUtils::Real	anchor;
		BaseUtils::Real	width;
	};

	struct StringRangeParam : public RangeParam {
		int		char_num_to_comp;
		bool	case_sensitive;
		bool	reversed_comp;
	};

	struct TimestampRangeParam : public RangeParam {
		BaseUtils::Timestamp	anchor;
		BaseUtils::TimeRange	width;
	};

	struct CustomizedRangeParam : public RangeParam {
	};
};
