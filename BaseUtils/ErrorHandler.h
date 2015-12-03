// ErrorHandler.h
// macro definitions for error handling or some rules
//

#pragma once

#include <assert.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#ifdef _DEBUG

#define RETURN_ON_FAIL(STATEMENT) \
	if (!(STATEMENT)) {	\
		assert(false);	\
		return false;	\
	}

#define PROCESS_ERROR(STATEMENT) \
	if (!(STATEMENT)) {	\
		assert(false);	\
		goto Exit0;	\
	}

#else

#define RETURN_ON_FAIL(STATEMENT) \
	if (!(STATEMENT)) {	\
		return false;	\
	}

#define PROCESS_ERROR(STATEMENT) \
	if (!(STATEMENT)) {	\
		goto Exit0;	\
	}

#endif

#define DELETE_POINTER(X) \
	if ((X) != NULL) { \
		delete (X); \
		(X) = NULL; \
	}

#define DELETE_ARRAY_POINTER(X) \
	if ((X) != NULL) { \
		delete[] (X); \
		(X) = NULL; \
	}

#define RELEASE_COM_OBJECT(X) \
	if ((X) != NULL) { \
		(X)->Release(); \
		(X) = NULL; \
	}

#define RELEASE_STD_CONTAINER_CONTENT(CONTAINER_TYPE, POINTER)	\
	if ((POINTER) != NULL)	{	\
		for (CONTAINER_TYPE::iterator iter = (POINTER)->begin(); iter != (POINTER)->end(); ++iter) {	\
			DELETE_POINTER(*iter);	\
			(*iter) = NULL;	\
		}	\
	}

#define RELEASE_STD_CONTAINER_CONST_CONTENT(CONTAINER_TYPE, POINTER)	\
	if ((POINTER) != NULL)	{	\
		for (CONTAINER_TYPE::const_iterator iter = (POINTER)->begin(); iter != (POINTER)->end(); ++iter) {	\
			delete (*iter);	\
		}	\
	}

// Disable warning in STLport for alignment issues
#pragma warning(disable: 4103)
