// record.h
// Definition for record class which is used for holding a record structure
// and its content.
// 

#pragma once

#include <vector>
#include "field.h"

namespace BaseUtils {
	class Record {
		// =====================================================
		// Interfaces

		// Constructors and destructors
	public:
		Record();
		virtual ~Record();
		bool Init();

	private:
		DISALLOW_COPY_AND_ASSIGN(Record);

		// Public interfaces
	public:
		_Field_Ptr	GetField(int index) const;
		_Field_Ptr	GetChildField(int index, int subindex) const;
		_Field_Ptr	GetField(const TCHAR* str) const;	// a string with the pattern "index:subindex"

		int		GetFieldCount() const;

		bool	AddField(Field* pField);

		// Protected interfaces
	protected:
		static bool ParseFieldStr(const TCHAR*, int*, int*);

		// =================================================
		// Data members
	protected:
		std::vector<Field*>*	m_pRecordHolder;

	};

};
