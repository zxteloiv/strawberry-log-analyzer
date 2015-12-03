// CStringStream.h
// Define a class inherited from IStream interface for a string.
//

#pragma once

//#include <Objidl.h>
//#pragma comment(lib, "uuid.lib")

namespace Toolkit {
	//
	// this class creates an ISequentialStream from a string
	//
	class CStringStream : public ISequentialStream {
	public:
		// factory method
		__checkReturn static HRESULT Create(
			__in LPCWSTR psBuffer,
			__deref_out ISequentialStream **ppStream) {
			HRESULT hr = S_OK;
			void *pNewBuff = NULL;
			size_t buffSize = 0;

			if (!psBuffer) {
				return E_INVALIDARG;
			}

			*ppStream = NULL;

			buffSize = (wcslen(psBuffer)+1) * sizeof(wchar_t);
			pNewBuff = malloc(buffSize);

			if (!pNewBuff) {
				return E_OUTOFMEMORY;
			}

			int res = memcpy_s(pNewBuff, buffSize, psBuffer, buffSize);

			if (0 == res) {
				*ppStream = new CStringStream(
					buffSize,
					pNewBuff);
				hr = S_OK;
			}

			if (!*ppStream) {
				hr = E_FAIL;
			}

			return hr;
		}

		// ISequentialStream
		__checkReturn HRESULT STDMETHODCALLTYPE Read(
			__out_bcount_part(cb, *pcbRead) void *pv,
			/* [in] */ ULONG cb,
			__out_opt  ULONG *pcbRead) {
			HRESULT hr = S_OK;

			for (*pcbRead = 0; *pcbRead < cb; ++*pcbRead, ++m_buffSeekIndex) {
				// we are seeking past the end of the buffer
				if (m_buffSeekIndex == m_buffSize) {
					hr = S_FALSE;
					break;
				}

				((BYTE*)pv)[*pcbRead] = ((BYTE*)m_pBuffer)[m_buffSeekIndex];
			}

			return hr;
		}

		HRESULT STDMETHODCALLTYPE Write(
			__in_bcount(cb)  const void *pv,
			/* [in] */ ULONG cb,
			__out_opt  ULONG *pcbWritten) {
			return E_NOTIMPL;
		}

		// IUnknown
		STDMETHODIMP_(ULONG) AddRef() {
			return InterlockedIncrement(&m_cRef);
		}

		STDMETHODIMP_(ULONG) Release() {
			LONG cRef = InterlockedDecrement(&m_cRef);

			if (0 == cRef) {
				delete this;
			}

			return cRef;
		}

		STDMETHODIMP QueryInterface(REFIID riid, __deref_out_opt void **ppv) {
			HRESULT hr = S_OK;

			if (ppv) {
				*ppv = NULL;
			} else {
				hr = E_INVALIDARG;
			}

			if (S_OK == hr) {
				if ((__uuidof(IUnknown) == riid) || (riid == __uuidof(ISequentialStream))) {
					AddRef();
					*ppv = (ISequentialStream*)this;
				} else {
					hr = E_NOINTERFACE;
				}
			}

			return hr;
		}

	protected:
		LONG m_cRef;
		void *m_pBuffer;
		size_t m_buffSize;
		size_t m_buffSeekIndex;

		// constructor/deconstructor
		CStringStream(
			__in size_t buffSize,
			__in void *pBuff)
				:
				m_cRef(1),
				m_pBuffer(pBuff),
				m_buffSize(buffSize),
				m_buffSeekIndex(0) {
		}

		~CStringStream() {
			free(m_pBuffer);
		}
	};

};
