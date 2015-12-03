// JobScheduler.cpp
// Implements JobScheduler class.
//

#include "stdafx.h"
#include "JobScheduler.h"
#include "Worker.h"

#include <process.h>
#include "DBAccessLogic.h"
#include "RecordDepository.h"

const TCHAR* g_szConfigFile = _T(".\\coreconf.ini");

using namespace ProcessCore;

const TCHAR*	JobScheduler::s_csConfSectionCore = _T("ProcessCore");
const TCHAR*	JobScheduler::s_csConfItemParallelJobs = _T("concurrent_jobs");

// Disable warning for empty statement in JobScheduler::DBCheckingThread
#pragma warning(disable : 4390)

JobScheduler::JobScheduler() : m_pDBConn(NULL), m_pFuncPrintLog(NULL), m_pcsJobCounterAccess(NULL) {
	m_hAllJobClearEvent = INVALID_HANDLE_VALUE;
}

JobScheduler::~JobScheduler() {
	if (m_pDBConn) {
		m_pDBConn->Close();
		DELETE_POINTER(m_pDBConn);
	}
	StopRunning();	// clean the m_hCheckingThread handle and exit the thread if it's running

	if (m_hAllJobClearEvent != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hAllJobClearEvent);
		m_hAllJobClearEvent = INVALID_HANDLE_VALUE;
	}
	if (m_pcsJobCounterAccess) {
		DeleteCriticalSection(m_pcsJobCounterAccess);
		DELETE_POINTER(m_pcsJobCounterAccess);
	}
}

bool JobScheduler::Init(PRINT_FUNC pFunc) {
	m_pFuncPrintLog = pFunc;
	m_hCheckingThread = INVALID_HANDLE_VALUE;
	m_nCurrentJobs = 0;
	RETURN_ON_FAIL(NULL != (m_pcsJobCounterAccess = new CRITICAL_SECTION));
	InitializeCriticalSection(m_pcsJobCounterAccess);
	// Read concurrent job configuration from config file, 0 is default value
	m_nMaxJobCount = ::GetPrivateProfileInt(s_csConfSectionCore, s_csConfItemParallelJobs, 0, g_szConfigFile);
	RETURN_ON_FAIL(m_nMaxJobCount >= 0);	// the max number of job counter cannot be less than or equal to 0

	m_hAllJobClearEvent = CreateEvent(NULL, TRUE, TRUE, L"easy name");
	RETURN_ON_FAIL(m_hAllJobClearEvent != NULL);

	DELETE_POINTER(m_pDBConn);	// if the Init function is sequentially called for two or more times.
	RETURN_ON_FAIL(NULL != (m_pDBConn = new Toolkit::ADOConnector));
	if (!m_pDBConn->Init()) {
		WriteLog(_T("Cannot initialize Database Connector Object."));
		PROCESS_ERROR(false);
	}

	if (!m_pDBConn->Open()) {
		WriteLog(_T("Cannot open database."));
		PROCESS_ERROR(false);
	}

	return true;

Exit0:
	if (m_pDBConn != NULL) {
		delete m_pDBConn;
		m_pDBConn = NULL;
	}
	return false;
}

void JobScheduler::IncreaseCurrentJobsCounter() {
	EnterCriticalSection(m_pcsJobCounterAccess);
	m_nConcurrentJob++;
	ResetEvent(m_hAllJobClearEvent);	// event reseted to non-signaled if any job is queued
	LeaveCriticalSection(m_pcsJobCounterAccess);
}

void JobScheduler::DecreaseCurrentJobsCounter() {
	EnterCriticalSection(m_pcsJobCounterAccess);
	m_nConcurrentJob--;
	if (0 == m_nConcurrentJob) {	// event is set signaled when all the jobs are finished
		SetEvent(m_hAllJobClearEvent);
	}
	LeaveCriticalSection(m_pcsJobCounterAccess);
}

bool JobScheduler::RunAsync() {
	// At a same time, only one DBChecking thread is allowed to be running.
	RETURN_ON_FAIL(m_hCheckingThread == INVALID_HANDLE_VALUE);	// the thread is not running
	DWORD dwThreadID;
	m_hCheckingThread = (HANDLE)_beginthreadex(NULL, NULL, JobScheduler::DBCheckingThread, (PVOID)this, NULL, (unsigned*)&dwThreadID);

	RETURN_ON_FAIL(m_hCheckingThread != INVALID_HANDLE_VALUE);

	return true;
}

void JobScheduler::StopRunning() {
	if (m_hCheckingThread != INVALID_HANDLE_VALUE) {
		// Queue an object into the Checking Thread's APC queue, to wake it up and let it exit gracefully
		::QueueUserAPC(APCFunc, m_hCheckingThread, NULL);
		WaitForSingleObject(m_hCheckingThread, INFINITE);	// block thread until the scheduler thread stops.
		CloseHandle(m_hCheckingThread);
		m_hCheckingThread = INVALID_HANDLE_VALUE;

		// Wait For all workers stopped.
		WaitForSingleObject(m_hAllJobClearEvent, INFINITE);	// block thread until all the worker threads stop working
	}
}

// --------------------------------------------------------------------------------------------
// static function implementation

unsigned int WINAPI JobScheduler::DBCheckingThread(PVOID pvContext) {
	JobScheduler* pSche = reinterpret_cast<JobScheduler*>(pvContext);

	DWORD dwSleepRtn;
	int* pRequestIDs = NULL;
	RETURN_ON_FAIL(NULL != (pRequestIDs = new int[pSche->m_nMaxJobCount]));
	do {
		// Get ID and submit it
		int nNumIdGot;
		WaitForSingleObjectEx(pSche->m_hAllJobClearEvent, INFINITE, TRUE);	// wait until all the worker threads complete their jobs
		if (DBAccessLogic::GetPendingRequestID(pSche->m_nMaxJobCount, pRequestIDs, pSche->m_pDBConn, &nNumIdGot)) {
			// for each obtained request ID, assign a worker thread to do so
			for (int reqID_index = 0; reqID_index < nNumIdGot; ++reqID_index) {
				JobThreadParam* pParam = JobThreadParam::BuildJobThreadParam(pSche, pRequestIDs[reqID_index]);
				if (pParam != NULL) {
					BOOL bResult = TrySubmitThreadpoolCallback(SimpleCallbackForJobThreadStart, pParam, NULL);

					if (FALSE == bResult)
						// Although submitting a work item to the threadpool can fail at some extreme time,
						// it will just lead that this work item will not be processed at this turn, while the
						// state of this request will not be changed. Next time the DO-WHILE loop will retrieve
						// the "Pending" request again and retry submitting.
						// So when failed to submit the work item, we still do nothing.
						;
				}
			}
		}

		dwSleepRtn = SleepEx(3600000, TRUE);	// wait for an hour to do next, or until others wake up this thread
	} while (dwSleepRtn != WAIT_IO_COMPLETION);

	DELETE_ARRAY_POINTER(pRequestIDs);
	return 0;
}

VOID WINAPI JobScheduler::APCFunc(ULONG_PTR) {
	// Do nothing.
	// This function is implemented emptily to use Alertable I/O to help CheckingThread to exit gracefully.
	// When an APC alert is manually queued, the thread will be waken up from suspended state and execute
	// this APCFunc routine, which does nothing, and the thread will continues executing right after the
	// statement caused its suspension like SleepEx or WaitForSingleObjectEx.
}

VOID NTAPI JobScheduler::SimpleCallbackForJobThreadStart(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext) {
	JobThreadParam* pParam = reinterpret_cast<JobThreadParam*>(pvContext);
	pParam->scheduler_ptr->IncreaseCurrentJobsCounter();

	Worker worker;
	PROCESS_ERROR(worker.Init(pParam->request_id));

	PROCESS_ERROR(worker.DoStuff());

Exit0:
	// After completed work, decrease the counter, which claiming that there's one more thread waiting for
	// process other request.
	// When the counter becomes 0, the function below will signal an event and the scheduler thread can
	// dispatch other work items to the threadpool
	pParam->scheduler_ptr->DecreaseCurrentJobsCounter();
	JobThreadParam::ReleaseParam(pParam);
}

