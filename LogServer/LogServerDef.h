// LogServerDef.h
// Contains class forward declarations for LogServer namespace.

#pragma once

#include <vector>

namespace LogServer {
	class IFieldDef;
	class TextFieldDef;
	class XmlFieldDef;

	class RecordDef;
	enum LogTextType;
	class LogHandler;

	class RecordDefAnsi;
	class IFieldDefAnsi;
	class TextFieldDefAnsi;
	class XmlFieldDefAnsi;

	typedef std::vector<IFieldDef*>	FIELDDEFVECTOR;
	typedef std::vector<IFieldDef*>* LPFIELDDEFVECTOR;
	typedef std::vector<IFieldDef*>::iterator FIELDDEFITER;
	typedef std::vector<IFieldDef*>::const_iterator FIELDDEFCONITER;
	
	typedef std::vector<IFieldDefAnsi*>		ANSIFIELDDEFVECTOR;
	typedef std::vector<IFieldDefAnsi*>*	LPANSIFIELDDEFVECTOR;
	typedef std::vector<IFieldDefAnsi*>::iterator		ANSIFIELDDEFITER;
	typedef std::vector<IFieldDefAnsi*>::const_iterator	ANSIFIELDDEFCONITER;
	
	typedef RecordDef*	_RecordDef_Ptr;

	class LogHandler;
	class CmdProcessor;
	class TcpCommBuf;
	class FixedLengthCompareW;
	class FixedLengthCompareA;
	typedef std::vector<FixedLengthCompareW*>	WIDEFLCVECT;
	typedef std::vector<FixedLengthCompareW*>*	LPWIDEFLCVECT;
	typedef std::vector<FixedLengthCompareW*>::iterator	WIDEFLCITER;
	typedef std::vector<FixedLengthCompareA*>	ANSIFLCVECT;
	typedef std::vector<FixedLengthCompareA*>*	LPANSIFLCVECT;
	typedef std::vector<FixedLengthCompareA*>::iterator	ANSIFLCITER;
#ifdef UNICODE
	typedef FixedLengthCompareW FixedLengthCompare;
#else
	typedef FixedLengthCompareA FixedLengthCompare;
#endif
};
