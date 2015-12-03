// JobScheduler.h
// JobScheduler class is responsible for scheduling all the request user submit and assigns each request
// a worker object to do, which is in a new thread. Then JobScheduler may have a threadpool.
//

#include "CoreDeclaration.h"

#include "..\\Toolkit\\Toolkit.h"

namespace ProcessCore {
	struct JobThreadParam {
		int				request_id;
		JobScheduler*	scheduler_ptr;

	public:
		static JobThreadParam* BuildJobThreadParam(JobScheduler* pSche, int reqID) {
			JobThreadParam* ptr = NULL;
			ptr = new JobThreadParam;
			if (ptr) {
				ptr->request_id = reqID;
				ptr->scheduler_ptr = pSche;
			}
			return ptr;
		}

		static void ReleaseParam(JobThreadParam* pParam) {
			delete pParam;
		}
	};

	class JobScheduler {
		// ==================================================
		// Interfaces

		// Constructor, destructor and two-phase initializer
	public:
		JobScheduler();
		~JobScheduler();
		bool Init(PRINT_FUNC pFunc = NULL);

	private:
		DISALLOW_COPY_AND_ASSIGN(JobScheduler);

		// public interfaces
	public:
		// Run a infinite loop to check pending user request until the adminstrator stops it.
		bool RunAsync();
		void StopRunning();
		bool IsRuning() { return (m_hCheckingThread != INVALID_HANDLE_VALUE); }
		void WriteLog(const TCHAR* str) { if (m_pFuncPrintLog != NULL) m_pFuncPrintLog(str); }

		// help functions
	protected:
		void IncreaseCurrentJobsCounter();
		void DecreaseCurrentJobsCounter();

		// =================================================
		// static functions

		// Thread start point
	protected:
		static VOID NTAPI SimpleCallbackForJobThreadStart(PTP_CALLBACK_INSTANCE	pInstance, PVOID pvContext);
		static unsigned int WINAPI DBCheckingThread(PVOID pvContext);
		static VOID WINAPI APCFunc(ULONG_PTR);

	public:
		static const TCHAR*	s_csConfSectionCore;
		static const TCHAR*	s_csConfItemParallelJobs;

		// ==================================================
		// Data members
	protected:
		PRINT_FUNC				m_pFuncPrintLog;

		HANDLE					m_hCheckingThread;
		HANDLE					m_hAllJobClearEvent;	// this event is signaled only when

		int						m_nConcurrentJob;
		int						m_nMaxJobCount;
		PCRITICAL_SECTION		m_pcsJobCounterAccess;
		int						m_nCurrentJobs;
		Toolkit::ADOConnector*	m_pDBConn;

	};
};
