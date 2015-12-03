// UtilsDef.h
// Contains definition or declarations for the all classes and types in BaseUtils.
// This is used to prevent include loops.
//

#pragma once

namespace BaseUtils {
	class Field;
	typedef	Field*	_Field_Ptr;
	class TimeRange;
	class Timestamp;
	class TimestampField;
	class StringField;

	typedef double Real;

	class RealField;
	class IntField;

	class Record;
	typedef Record*	_Record_Ptr;
};
