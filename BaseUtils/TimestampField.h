// TimestampField.h
// Contains derivation of int field class from Field class.
//

#pragma once

#include "field.h"
#include "timestamp.h"


namespace BaseUtils {
	class TimestampField : public Field {
		// ======================================================
		// Interfaces
		
		// Constructors and destructors
	public:
		TimestampField(const Timestamp&);
		TimestampField(short, char, char, char, char, char);
		virtual ~TimestampField();

	private:
		DISALLOW_COPY_AND_ASSIGN(TimestampField);

		// Public interfaces
	public:
		const Timestamp* GetPointerTime() const;

		// ======================================================
		// Data members
	protected:
		Timestamp	m_value;
	};
};
