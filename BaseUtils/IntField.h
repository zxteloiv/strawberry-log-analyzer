// IntField.h
// Contains derivation of int field class from Field class.
//

#pragma once

#include "field.h"

namespace BaseUtils {
	class IntField : public Field {
		// ======================================================
		// Interfaces
		
		// Constructors and destructors
	public:
		IntField();
		IntField(int);
		virtual ~IntField();

	private:
		DISALLOW_COPY_AND_ASSIGN(IntField);

		// Public interface
	public:
		int	GetValueInt() const { return m_value; }

		// ======================================================
		// Data members
	protected:
		int	m_value;
	};
};
