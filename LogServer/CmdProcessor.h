// CmdProcessor.h
// Process the command with a buffer.
//

#pragma once

#include "LogServerDef.h"
#include "LogExchangeProtocol.h"
#include <list>
#include "..\\Toolkit\\Toolkit.h"
#include "..\\BaseUtils\\BaseUtils.h"

namespace LogServer {
	class CodePage {
	public:
		const static UINT Shift_JIS_ANSI = 932;
		const static UINT GB2312_ANSI = 936;
		const static UINT BIG5_ANSI = 950;
		const static UINT Unified_Hangul_ANSI = 949;
		const static UINT UCS2_LE = 1200;
		const static UINT UCS2_BE = 1201;
		const static UINT JPN_MAC = 10001;
		const static UINT BIG5_MAC = 10002;
		const static UINT KOREAN_MAC = 10003;
		const static UINT GB2312_MAC = 10008;
		const static UINT UCS4_LE = 12000;
		const static UINT UCS4_BE = 12001;
		const static UINT ASCII_US = 20127;
		const static UINT GB18030 = 54936;
		const static UINT UTF7 = 65000;
		const static UINT UTF8 = 65001;

		const static int cp_count = 16;

		static const TCHAR* cp_string[16];

		static UINT GetCodePageID(const TCHAR*);
	};


	class CmdProcessor {
		// ===========================================================
		// Inner definitions
	protected:
		typedef bool (CmdProcessor::*BOOL_FUNC_VOID)();
		typedef bool (CmdProcessor::*BOOL_FUNC_INT)(int);

		typedef std::list<BaseUtils::_Record_Ptr>	RECORD_TEMP_LIST;
		typedef std::list<BaseUtils::_Record_Ptr>*	RECORD_TEMP_LIST_PTR;
		typedef std::list<BaseUtils::_Record_Ptr>::iterator	RECORD_TEMP_ITER;

		// ============================================================
		// Interfaces
	public:
		CmdProcessor();
		~CmdProcessor();
		bool Init();

		// Public Interfaces
	public:
		bool ParseCmdBuf(
			const char*	pBuf,
			int			nBufSize = CMD_HEADER_SIZE,
			int*		pVersion = NULL,
			int*		pCmdID = NULL,
			int*		pCmdParamCount = NULL,
			int*		pSize = NULL);

		int CheckVersion() const { return m_nVersion; }
		int GetCmdID() const { return m_nCmdID; }
		int GetParameterCount() const { return m_nParamCount; }
		int GetCmdSize() const { return m_nCmdSize; }

		char* GetResponseBuf() { return m_pResponseBuf; }
		int	GetResponseBufSize() { return m_nResponseSize; }

		bool ExecuteCommand(
			const char* pParamBuf,
			int			nParamBufSize,
			const char*	pHeaderBuf = NULL,
			int			nHeaderBufSize = CMD_HEADER_SIZE);

		// Help methods
	protected:
		bool DispatchNoParam(BOOL_FUNC_VOID pFun, const char* pParamBuf, int nBufSize);
		bool DispatchParamWithInt(BOOL_FUNC_INT pFun, const char* pParamBuf, int nBufSize);

		bool BuildResponseWithBool(bool rtn);
		void CleanResponse();

		bool ValidateLog(int nLogID);
		bool RequestForLog(int nLogID);
		bool CloseRequest();
		bool HasMoreRecords();

		bool LoadRecord(int nCount);
		bool ReadRecordUTF16LE(int nCount);
		bool ReadRecordUTF16BE(int nCount);
		bool ReadRecordUTF32LE(int nCount);
		bool ReadRecordUTF32BE(int nCount);
		bool ReadRecordANSI(int nCount);

		bool CheckAndSkipBOM(int* pBom);
		bool ReverseEndian(void* pStart, int len);
		bool PrepareCheckers(bool ansi_ver);
		void CleanCheckers();

		bool UseRecordStartChecker(FixedLengthCompareA*);
		bool UseRecordStartChecker(FixedLengthCompareW*);
		bool UseRecordEndChecker(FixedLengthCompareA*);
		bool UseRecordEndChecker(FixedLengthCompareW*);
		bool UseFieldStartChecker(const IFieldDefAnsi*, FixedLengthCompareA*);
		bool UseFieldStartChecker(const IFieldDef*, FixedLengthCompareW*);

		BaseUtils::Field* GetFieldByString(const IFieldDefAnsi*, const char*);
		BaseUtils::Field* GetFieldByString(const IFieldDef*, const TCHAR*);

		bool BuildResponseWithRecords(RECORD_TEMP_LIST_PTR pList);
		bool CalculateResponseSize(RECORD_TEMP_LIST_PTR pList);
		bool SetResponseHeaderWithRecords(int nTotalFieldCount);
		bool WriteIntFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID);
		bool WriteRealFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID);
		bool WriteStringFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID);
		bool WriteTimeFieldToBuffer(int& offset, BaseUtils::_Field_Ptr pField, int nRecID);

		void CloseCFile() {
			if (m_pFile) {
				fclose(m_pFile);
				m_pFile = NULL;
			}
		}


		// ============================================================
		// Data members
	protected:
		int		m_nVersion;
		int		m_nCmdID;
		int		m_nParamCount;
		int		m_nCmdSize;

		char*	m_pResponseBuf;	// it is a binary buffer rather than an ANSI char string
		int		m_nResponseSize;

		RecordDef*	m_pRecDef;
		RecordDefAnsi* m_pAnsiRecDef;

		FixedLengthCompareA* m_pAnsiRecStartChecker;
		FixedLengthCompareA* m_pAnsiRecEndChecker;
		LPANSIFLCVECT m_pAnsiFieldStartCheckers;
		LPANSIFLCVECT m_pAnsiFieldEndCheckers;
		
		FixedLengthCompareW* m_pWideRecStartChecker;
		FixedLengthCompareW* m_pWideRecEndChecker;
		LPWIDEFLCVECT m_pWideFieldStartCheckers;
		LPWIDEFLCVECT m_pWideFieldEndCheckers;

		TCHAR*	m_szFilename;
		FILE*	m_pFile;
		int		m_nBOM;
		Toolkit::ADOConnector* m_pDBConn;

		// ============================================================
		// Static members
	protected:
		static const int NO_BOM = 0;
		static const int UTF16LE_BOM = 1;
		static const int UTF16BE_BOM = 2;
		static const int UTF32LE_BOM = 3;
		static const int UTF32BE_BOM = 4;
		static const int UTF8_BOM = 5;
		static const int UTF16_BYTE_LEN = 2;
		static const int UTF32_BYTE_LEN = 4;

	};
};
