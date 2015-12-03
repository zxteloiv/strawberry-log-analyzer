// StawberryLogAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "..\\BaseUtils\\BaseUtils.h"
//#include "..\\Toolkit\\Toolkit.h"
#include "..\\ProcessCore\\ProcessCore.h"
//#include "..\\ExternalRoutes\\ExternalRoutes.h"

#include <string.h>
#include <iostream>
#include <process.h>
#include <string>

// Global functions
HANDLE	g_schedulerThread = INVALID_HANDLE_VALUE;
HANDLE	g_hIOCP = INVALID_HANDLE_VALUE;	// the I/O completion port for admin UI to communicate with scheduler thread

std::wstring g_strCmdBuf;
CRITICAL_SECTION g_csCmdBuf;

TCHAR* question_xml = _T("<?xml version=\"1.0\" encoding=\"ucs-2\"?><request><aggregators>blablabla</aggregators><questions><question id=\"1\" text=\"question 1\" agg_id=\"1\" has_range_agg=\"true\" has_result_agg=\"true\">	<range_agg agg_id=\"2\" has_range_agg=\"true\" has_result_agg=\"true\">		<range_agg agg_id=\"4\" has_range_agg=\"false\" has_result_agg=\"false\"/>		<result_agg agg_id=\"5\" has_range_agg=\"true\" has_result_agg=\"false\">			<range_agg agg_id=\"7\" has_range_agg=\"false\" has_result_agg=\"false\"/>		</result_agg>	</range_agg>	<result_agg agg_id=\"3\" has_range_agg=\"false\" has_result_agg=\"true\">		<result_agg agg_id=\"6\" has_range_agg=\"false\" has_result_agg=\"false\"/>	</result_agg></question><question id=\"2\" text=\"question 2\" agg_id=\"3\"/></questions></request>");

// Scheduler Thread Command ID (Completion Keys)
#define CK_EXIT_THREAD			0
#define CK_PAUSE_SCHEDULER		1
#define CK_RESUME_SCHEDULER		2
#define CK_EXIT_PROCESS			3
#define CK_SHOW_STATUS			4

// Print out log
void Out(const TCHAR* str);
int	WaitForUserInput();
unsigned int WINAPI SchedulerThread(PVOID);
bool MyCompare(const TCHAR* str1, int len1, const TCHAR* str2, int len2);

int _tmain(int argc, _TCHAR* argv[]) {
	// Initilaization
	::InitializeCriticalSection(&g_csCmdBuf);
	g_schedulerThread = INVALID_HANDLE_VALUE;
	g_hIOCP = INVALID_HANDLE_VALUE;
	g_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NULL);
	DWORD dwThreadID;
	g_schedulerThread = (HANDLE)_beginthreadex(NULL, NULL, SchedulerThread, NULL, NULL, (unsigned*)&dwThreadID);

	WaitForUserInput();	// read & parse characters user inputed

	WaitForSingleObject(g_schedulerThread, INFINITE);

	DWORD dwExitCode = 0;
	::GetExitCodeThread(g_schedulerThread, &dwExitCode);
	if (dwExitCode) {
		Out(_T("Scheduler exits unexpectedly."));
	}
	::CloseHandle(g_schedulerThread);
	::DeleteCriticalSection(&g_csCmdBuf);
	::CloseHandle(g_hIOCP);

	//system("pause");
	return 0;
}

unsigned int WINAPI SchedulerThread(PVOID) {
	ProcessCore::JobScheduler scheduler;
	if (!scheduler.Init(::Out)) {
		return -1;
	}

	if (!ProcessCore::RecordDepository::InitDepository()) {
		return -1;
	}

	if (!scheduler.RunAsync()) {
		return -1;
	}

	::Out(_T("Scheduler started."));

	while (true) {
		ULONG_PTR completionKey;
		OVERLAPPED* pOverlapped;
        DWORD dwNumBytes;	// this value is only used to trap the System API
		// Here we use I/O Completion port only for thread communication & synchronization,
		// therefore the OVERLAPPED structure is not used at all.
		if (!::GetQueuedCompletionStatus(g_hIOCP, &dwNumBytes, &completionKey, &pOverlapped, INFINITE)) {
			scheduler.StopRunning();
			return -1;
		}

		if (completionKey == CK_EXIT_THREAD) {
			break;	// exit the infinit loop
		} else if (completionKey == CK_PAUSE_SCHEDULER) {
			scheduler.StopRunning();
			::Out(_T("Scheduler successfully stopped."));
		} else if (completionKey == CK_RESUME_SCHEDULER) {
			if (scheduler.RunAsync()) {
				::Out(_T("^_^ Scheduler started successfully!!"));
			} else {
				::Out(_T("Ooooopps! Scheduler failed to start."));
			}
		} else if (completionKey == CK_SHOW_STATUS) {
			if (scheduler.IsRuning()) {
				::Out(_T("Status: Scheduler is now running."));
			} else {
				::Out(_T("Status: Scheduler is now waiting."));
			}
		}
	}

	ProcessCore::RecordDepository::Cleanup();
	
	return 0;
}

// WaitForUserInput
// This function will take this thread into blocked in order to accept user input.
// Params:	none
// Return:	CK_EXIT_PROCESS to indicate the user wants to exit this program.
int WaitForUserInput() {
	_tprintf(_T(">"));
	bool bContinue = true;
	while (bContinue) {
		TCHAR buf[2] = { 0, 0 };
		buf[0] = _gettch();
		_tprintf(_T("%c"), buf[0]);
		
		if (buf[0] == _T('\r')) {
			if (MyCompare(g_strCmdBuf.c_str(), g_strCmdBuf.size(), _T("show status"), 0)) {
				::PostQueuedCompletionStatus(g_hIOCP, 0, CK_SHOW_STATUS, (LPOVERLAPPED)((LONG_PTR)-1));
			} else if (MyCompare(g_strCmdBuf.c_str(), g_strCmdBuf.size(), _T("pause scheduler"), 0)) {
				::PostQueuedCompletionStatus(g_hIOCP, 0, CK_PAUSE_SCHEDULER, (LPOVERLAPPED)((LONG_PTR)-1));
			} else if (MyCompare(g_strCmdBuf.c_str(), g_strCmdBuf.size(), _T("resume scheduler"), 0)) {
				::PostQueuedCompletionStatus(g_hIOCP, 0, CK_RESUME_SCHEDULER, (LPOVERLAPPED)((LONG_PTR)-1));
			} else if (MyCompare(g_strCmdBuf.c_str(), g_strCmdBuf.size(), _T("exit"), 0)) {
				::PostQueuedCompletionStatus(g_hIOCP, 0, CK_EXIT_THREAD, (LPOVERLAPPED)((LONG_PTR)-1));
				bContinue = false;
			}
			EnterCriticalSection(&g_csCmdBuf);
			g_strCmdBuf.clear();
			LeaveCriticalSection(&g_csCmdBuf);
			_tprintf(_T(">"));
		} else {
			EnterCriticalSection(&g_csCmdBuf);
			g_strCmdBuf.append(buf);
			LeaveCriticalSection(&g_csCmdBuf);
		}
	}	
	return CK_EXIT_PROCESS;
}

void Out(const TCHAR* str) {
	EnterCriticalSection(&g_csCmdBuf);
	_tprintf(_T("\r\n%s\r\n"), str);
	_tprintf(_T(">%s"), g_strCmdBuf.c_str());
	LeaveCriticalSection(&g_csCmdBuf);
}

// MyCompare
// Compare two strings & process backspace characters inside.
// Params:	str1, str2: strings to compare
//			len1, len2: characters of str1 & str2 to compare,
//						can be 0 to indicate the full length of string
// Return:	true if two strings are the same, otherwise false.
bool MyCompare(const TCHAR* str1, int len1, const TCHAR* str2, int len2) {
	// compare the entire string if length is specified 0
	len1 = ((0 == len1) ? (_tcslen(str1)) : len1);
	len2 = ((0 == len2) ? (_tcslen(str2)) : len2);

	// The count of backspace characters that are already met and not processed yet in both strings.
	int rtn_count1 = 0;
	int rtn_count2 = 0;
	// when both strings are not fully processed from right to left
	while (0 < len1 && 0 < len2) {
		// a backspace is met, from the string end to its beginning
		if (str1[len1] == _T('\b')) {
			len1--;
			rtn_count1++;
			continue;
		}
		if (str2[len2] == _T('\b')) {
			len2--;
			rtn_count2++;
			continue;
		}
		// If there's any backspace character, the following characters are the nearest to it.
		// Therefore this character must be ignored.
		if (rtn_count1) {
			len1--;
			rtn_count1--;
			continue;
		}
		if (rtn_count2) {
			len2--;
			rtn_count2--;
			continue;
		}
		// after processing all backspaces currently met, check the string content.
		if (str1[len1] != str2[len2])
			return false;
		len1--;
		len2--;
	}

	for (int ch_id = 0; ch_id < len1; ++ch_id) {
		if (str1[ch_id] == _T('\b')) {
			rtn_count1++;
		}
	}
	if (rtn_count1 < len1)
		return false;
	for (int ch_id = 0; ch_id < len2; ++ch_id) {
		if (str2[ch_id] == _T('\b')) {
			rtn_count2++;
		}
	}
	if (rtn_count2 < len1)
		return false;

	return true;
}
