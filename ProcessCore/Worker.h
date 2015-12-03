// Worker.h
// Worker class is in charge of processing a single piece of request which the JobScheduler give it.
//

#include "CoreDeclaration.h"
#include "..\\Toolkit\\Toolkit.h"

#include <xmllite.h>

namespace ProcessCore {
	class Worker {
		// =======================================================
		// Interfaces

		// Constructors, destructors and initializers.
	public:
		Worker();
		~Worker();
		bool Init(int nRequestID);

	private:
		DISALLOW_COPY_AND_ASSIGN(Worker);

		// Public interfaces
	public:
		bool DoStuff();

		// help functions
	protected:
		bool StartSavingResult();
		bool SaveQuestionResult(int nQuestionID, int nRootAggID);
		bool EndSavingResultAndUpdateDB();

		// =======================================================
		// Static contents
	protected:
		static const TCHAR* s_szElemResult;
		static const TCHAR* s_szElemQuestion;
		static const TCHAR* s_szAttrQuestionID;
		static const TCHAR* s_szAttrRootAggID;

		// ========================================================
		// Data members
	protected:
		int						m_nRequestID;
		int						m_nLogID;
		AggregatorManager*		m_pAggMgr;
		RecordDepository*		m_pDepo;
		LPQUESTLIST				m_pQTreeList;
		Toolkit::ADOConnector*	m_pDBConn;

		IXmlWriter*				m_pWriter;
		IStream*				m_pStream;
		IXmlWriterOutput*		m_pWriterOutput;
	};
};
