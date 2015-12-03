// field.h
// Include the fields class declaration
//

#pragma once

#include <string>
#include <vector>

#include "UtilsDef.h"

namespace BaseUtils {
	class Field {
		friend class Record;
		// =================================================
		// Inner class types
	public:
		enum FieldType {
			integer,
			real,
			timestamp,
			string,
			any_field_type
		};

		// =================================================
		// Interfaces

		// constructors and desctructors
	protected:
		Field();
	public:
		virtual ~Field();

	private:
		DISALLOW_COPY_AND_ASSIGN(Field);

		// Public interfaces
	public:
		virtual FieldType GetType() const { return m_fieldType; }
		virtual _Field_Ptr	GetChildField(int) const;
		// Get the pointer of the record that contains this field.
		// This pointer is added here because field number can be much less than the number of ranges.
		virtual _Record_Ptr	GetRecordBelongedTo() const { return m_pRecordBelongedTo; }

		// =================================================
		// members
	protected:
		FieldType				m_fieldType;
		std::vector<Field*>*	m_pChildFields;
		_Record_Ptr				m_pRecordBelongedTo;
	};


};
