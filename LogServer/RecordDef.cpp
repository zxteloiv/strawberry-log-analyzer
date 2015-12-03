// RecordDef.cpp
// Implements the RecordDef class.
//

#include "stdafx.h"
#include "RecordDef.h"
#include "XmlFieldDef.h"
#include "TextFieldDef.h"
#include "FixedLengthCompare.h"

#include "..\\Toolkit\\Toolkit.h"	// for String Stream
#include <string>
#include <vector>

#include "..\\BaseUtils\\BaseUtils.h" // for BaseUtils::Field::FieldType

// The <xmllite.h> must be included after the Toolkit.h which uses ISequentialStream class
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

#pragma warning(disable : 4018)	// for signed/unsigned int comparison

// A UTF32 character is 4 bytes at most &  1 is for null terminator
#define WideCchToCharLen(X) ((X) * 4 + 1)

using namespace LogServer;
using namespace Toolkit;

const TCHAR* RecordDef::s_szElemNameLogDef = _T("LogDefinition");
const TCHAR* RecordDef::s_szElemNameRecord = _T("Record");
const TCHAR* RecordDef::s_szElemNameField = _T("Field");
const TCHAR* RecordDef::s_szAttrNameLogEncoding = _T("encoding");
const TCHAR* RecordDef::s_szAttrNameLogType = _T("type");
const TCHAR* RecordDef::s_szAttrValueLogTypeXml = _T("xml");
const TCHAR* RecordDef::s_szAttrValueLogTypeText = _T("text");
const TCHAR* RecordDef::s_szAttrNameFieldCount = _T("FieldsCount");
const TCHAR* RecordDef::s_szAttrNameRecStart = _T("StartSymbol");
const TCHAR* RecordDef::s_szAttrNameRecEnd = _T("EndSymbol");
const TCHAR* RecordDef::s_szAttrNameFieldStart = _T("StartSymbol");
const TCHAR* RecordDef::s_szAttrNameFieldEnd = _T("EndSymbol");
const TCHAR* RecordDef::s_szAttrNameFieldTrim = _T("trim");
const TCHAR* RecordDef::s_szAttrValueTrimTrue = _T("true");
const TCHAR* RecordDef::s_szAttrValueTrimFalse = _T("false");
const TCHAR* RecordDef::s_szAttrNameFieldType = _T("type");
const TCHAR* RecordDef::s_szAttrValueFieldTypeInt = _T("int");
const TCHAR* RecordDef::s_szAttrValueFieldTypeReal = _T("real");
const TCHAR* RecordDef::s_szAttrValueFieldTypeTime = _T("time");
const TCHAR* RecordDef::s_szAttrValueFieldTypeString = _T("string");
const TCHAR* RecordDef::s_szAttrNameFieldAlias = _T("alias");
const TCHAR* RecordDef::s_szEmpty = _T("");	// Empty string

bool RecordDef::NameIsXmlComponent(const TCHAR* name, const TCHAR* xmlComponent) {
	return (0 == _tcscmp(name, xmlComponent));
}


RecordDef::RecordDef() : m_type(reserved_text_type) {
	m_pFieldDefs = NULL;
	m_szStartSymbol = NULL;
	m_szEndSymbol = NULL;
	m_nStartCch = 0;
	m_nEndCch = 0;
}

RecordDef::~RecordDef() {
	if (m_pFieldDefs != NULL) {
		for (FIELDDEFITER iter = m_pFieldDefs->begin(); iter != m_pFieldDefs->end(); ++iter)
			delete (*iter);
		DELETE_POINTER(m_pFieldDefs);
	}
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
}

bool RecordDef::Init(LogTextType	logType,
					 const TCHAR*	encoding,
					 const TCHAR*	start_symbol,
					 const TCHAR*	end_symbol)
{
	RETURN_ON_FAIL((m_pFieldDefs = new FIELDDEFVECTOR) != NULL);
	if (start_symbol != NULL) {
		PROCESS_ERROR(LogServer::TranslatingCopy(&m_szStartSymbol, start_symbol));
		m_nStartCch = _tcslen(m_szStartSymbol);
	}
	if (end_symbol != NULL) {
		PROCESS_ERROR(LogServer::TranslatingCopy(&m_szEndSymbol, end_symbol));
		m_nEndCch = _tcslen(m_szEndSymbol);
	}
	if (encoding != NULL) {
		PROCESS_ERROR(LogServer::TranslatingCopy(&m_szEncoding, encoding));
	}

	m_type = logType;
	return true;
Exit0:
	DELETE_POINTER(m_pFieldDefs);
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
	DELETE_ARRAY_POINTER(m_szEncoding);
	return false;
}

_RecordDef_Ptr RecordDef::ParseDefFromXML(const TCHAR *xml_string) {
	_RecordDef_Ptr pRecordDef = NULL;
	IXmlReader* pReader = NULL;
	ISequentialStream* pStream = NULL;
	HRESULT hr = S_OK;

	PROCESS_ERROR(S_OK == (hr = CStringStream::Create(xml_string, &pStream)));
	PROCESS_ERROR(S_OK == (hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)));
	PROCESS_ERROR(S_OK == (hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)));
	PROCESS_ERROR(S_OK == (hr = pReader->SetInput(pStream)));

	PROCESS_ERROR(NULL != (pRecordDef = new RecordDef));
	XmlNodeType nodeType;
	LogTextType logType;
	logType = reserved_text_type;
	LPCTSTR	szEncoding = NULL;
	while (S_OK == (hr = pReader->Read(&nodeType))) {
		if (nodeType == XmlNodeType_Element) {
			const TCHAR* elemName = NULL;
			PROCESS_ERROR(S_OK == pReader->GetLocalName(&elemName, NULL));
			if (NameIsXmlComponent(elemName, s_szElemNameLogDef)) {	// <LogDefinition> element
				const TCHAR* type_str = NULL;
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameLogType, NULL));	// type attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&type_str, NULL));
				if (NameIsXmlComponent(type_str, s_szAttrValueLogTypeText)) { // type is "text"
					logType = text_log;
				} else if (NameIsXmlComponent(type_str, s_szAttrValueLogTypeXml)) { // type is "xml"
					logType = xml_log;
				} else
					PROCESS_ERROR(false);	// error log type when neither text nor xml matched

				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameLogEncoding, NULL));
				PROCESS_ERROR(S_OK == pReader->GetValue(&szEncoding, NULL));

				PROCESS_ERROR(S_OK == pReader->MoveToElement());	// no attribute anymore
			} else if (NameIsXmlComponent(elemName, s_szElemNameRecord)) {	// <Record> element
				const TCHAR* start = NULL;
				const TCHAR* end = NULL;
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameRecStart, NULL));	// StartSymbol attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&start, NULL));
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameRecEnd, NULL));	// EndSymbol attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&end, NULL));
				
				PROCESS_ERROR(pRecordDef->Init(logType, szEncoding, start, end));
				PROCESS_ERROR(S_OK == pReader->MoveToElement());	// no attribute needed anymore
			} else if (NameIsXmlComponent(elemName, s_szElemNameField)) {	// <Field> element
				const TCHAR* start = NULL;
				const TCHAR* end = NULL;
				const TCHAR* trim_str = NULL;
				const TCHAR* type_str = NULL;
				const TCHAR* alias = NULL;
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameFieldStart, NULL));	// StartSymbol attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&start, NULL));
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameFieldEnd, NULL));	// EndSymbol attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&end, NULL));
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameFieldTrim, NULL));	// trim attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&trim_str, NULL));
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameFieldType, NULL));	// type attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&type_str, NULL));
				PROCESS_ERROR(S_OK == pReader->MoveToAttributeByName(s_szAttrNameFieldAlias, NULL));	// alias attribute
				PROCESS_ERROR(S_OK == pReader->GetValue(&alias, NULL));
				PROCESS_ERROR(S_OK == pReader->MoveToElement());	// no attribute needed anymore

				// Check field type string
				BaseUtils::Field::FieldType field_type = BaseUtils::Field::any_field_type;
				if (NameIsXmlComponent(type_str, s_szAttrValueFieldTypeInt)) {
					field_type = BaseUtils::Field::integer;
				} else if (NameIsXmlComponent(type_str, s_szAttrValueFieldTypeReal)) {
					field_type = BaseUtils::Field::real;
				} else if (NameIsXmlComponent(type_str, s_szAttrValueFieldTypeTime)) {
					field_type = BaseUtils::Field::timestamp;
				} else if (NameIsXmlComponent(type_str, s_szAttrValueFieldTypeString)) {
					field_type = BaseUtils::Field::string;
				} else 
					PROCESS_ERROR(false);

				if (pRecordDef->m_type == text_log) {
					TextFieldDef* pFieldDef = new TextFieldDef(
						NameIsXmlComponent(trim_str, s_szAttrValueTrimTrue), field_type);
					PROCESS_ERROR(pFieldDef != NULL);
					PROCESS_ERROR(AddFieldToRecord(pRecordDef, pFieldDef));
					pFieldDef->Init(start, end, alias);
				} else if (pRecordDef->m_type == xml_log) {
					//XmlFieldDef* pFieldDef = new XmlFieldDef;	// not supported yet
				} else
					PROCESS_ERROR(false);				
			}	// end of <field> check
		}	// XmlNodeType_Element
		if (nodeType == XmlNodeType_EndElement) {
			const TCHAR* local_name;
			PROCESS_ERROR(S_OK == pReader->GetLocalName(&local_name, NULL));
			if (NameIsXmlComponent(local_name, s_szElemNameRecord))	// </Record>
				break;	// only 1 record definition is allowed for a log file
		}
	}


	RELEASE_COM_OBJECT(pReader);
	RELEASE_COM_OBJECT(pStream);
	return pRecordDef;
Exit0:
	RELEASE_COM_OBJECT(pReader);
	RELEASE_COM_OBJECT(pStream);
	DELETE_POINTER(pRecordDef);
	return NULL;
}

bool RecordDef::AddFieldToRecord(_RecordDef_Ptr pRecordDef, IFieldDef* const & pFieldDef) {
	RETURN_ON_FAIL(pRecordDef != NULL && pRecordDef->m_pFieldDefs != NULL && pFieldDef != NULL);
	pRecordDef->m_pFieldDefs->push_back(pFieldDef);
	return true;
}

bool RecordDef::ReleaseDef(LogServer::_RecordDef_Ptr pRecord) {
	delete pRecord;
	pRecord = NULL;
	return true;
}

int RecordDef::GetFieldCount() {
	return m_pFieldDefs->size();
}

const IFieldDef* RecordDef::GetFieldDefAt(int index) const {
	if (index < m_pFieldDefs->size()) {
		return m_pFieldDefs->at(index);
	}
	return NULL;
}

RecordDefAnsi::RecordDefAnsi() : m_type(reserved_text_type) {
	m_pFieldDefs = NULL;
	m_szStartSymbol = NULL;
	m_szEndSymbol = NULL;
	m_nStartCb = 0;
	m_nEndCb = 0;
}

RecordDefAnsi::~RecordDefAnsi() {
	if (m_pFieldDefs != NULL) {
		for (ANSIFIELDDEFITER iter = m_pFieldDefs->begin(); iter != m_pFieldDefs->end(); ++iter)
			delete (*iter);
		DELETE_POINTER(m_pFieldDefs);
	}
	DELETE_ARRAY_POINTER(m_szStartSymbol);
	DELETE_ARRAY_POINTER(m_szEndSymbol);
}

int RecordDefAnsi::GetFieldCount() {
	return m_pFieldDefs->size();
}

const IFieldDefAnsi* RecordDefAnsi::GetFieldDefAt(int index) const {
	if (index < m_pFieldDefs->size()) {
		return m_pFieldDefs->at(index);
	}
	return NULL;
}

bool RecordDef::ToAnsiDef(LogServer::RecordDefAnsi** ppAnsi, UINT codepage_id) {
	RecordDefAnsi* pAnsi = NULL;
	RETURN_ON_FAIL((pAnsi = new RecordDefAnsi) != NULL);

	// copy type
	pAnsi->m_type = this->m_type;

	// copy record start & end symbol
	int len = WideCchToCharLen(m_nStartCch);
	PROCESS_ERROR(NULL != (pAnsi->m_szStartSymbol = new char[len]));
	// Convertion includes the null-terminator in the WideChar array, too
	PROCESS_ERROR(m_nStartCch + 1 == WideCharToMultiByte(codepage_id, NULL, m_szStartSymbol, m_nStartCch + 1,
		pAnsi->m_szStartSymbol, len, NULL, NULL));
	pAnsi->m_nStartCb = strlen(pAnsi->m_szStartSymbol);

	len = WideCchToCharLen(m_nEndCch);
	PROCESS_ERROR(NULL != (pAnsi->m_szEndSymbol = new char[len]));
	PROCESS_ERROR(m_nEndCch + 1 == WideCharToMultiByte(codepage_id, NULL, m_szEndSymbol, m_nEndCch + 1,
		pAnsi->m_szEndSymbol, len, NULL, NULL));
	pAnsi->m_nEndCb = strlen(pAnsi->m_szEndSymbol);

	PROCESS_ERROR(NULL != (pAnsi->m_pFieldDefs = new ANSIFIELDDEFVECTOR));
	TextFieldDefAnsi* pNewFieldDef = NULL;
	if (m_type == text_log) {
		for (FIELDDEFITER iter = this->m_pFieldDefs->begin(); iter != m_pFieldDefs->end(); ++iter) {
			TextFieldDef* pFieldDef = dynamic_cast<TextFieldDef*>(*iter);
			PROCESS_ERROR(pFieldDef != NULL);

			PROCESS_ERROR(NULL != (pNewFieldDef = new TextFieldDefAnsi));
			PROCESS_ERROR(pFieldDef->ToAnsiDef(pNewFieldDef, codepage_id));

			pAnsi->m_pFieldDefs->push_back(pNewFieldDef);
		}
	} else if (m_type == xml_log) {	// not supported yet
	}

	*ppAnsi = pAnsi;
	return true;

Exit0:
	DELETE_POINTER(pNewFieldDef);
	DELETE_POINTER(pAnsi->m_pFieldDefs);
	DELETE_ARRAY_POINTER(pAnsi->m_szStartSymbol);
	DELETE_ARRAY_POINTER(pAnsi->m_szEndSymbol);
	DELETE_POINTER(pAnsi);
	return false;
}
