// RealField.h
// Contains derivation of int field class from Field class.
//

#pragma once

#include "UtilsDef.h"
#include "field.h"

namespace BaseUtils {
	class RealField : public Field {
		// ======================================================
		// Interfaces
		
		// Constructors and destructors
	public:
		RealField();
		RealField(Real&);	
		virtual ~RealField();
	
	private:
		DISALLOW_COPY_AND_ASSIGN(RealField);

		// Public interfaces
	public:
		const Real&	GetValueReal() const { return m_value; }	// return reference type in case of
													// Real is implemented as a class

		// ======================================================
		// Data members
	protected:
		Real	m_value;
	};
};
