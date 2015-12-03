// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <tchar.h>

// Winsocket headers

// This header is important to prevent windows.h from include the conflicting Winsock definitions
// against the following winsock2.h header.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// winsock2.h has included essential portion of definitions in windows.h. Here this include line
// for windows.h is only for other function use.
#include <windows.h>
// The Winsock2.h header file contains most of the Winsock functions, structures, and definitions.
#include <winsock2.h>
// The Ws2tcpip.h header file contains definitions introduced in the WinSock 2 Protocol-Specific
// Annex document for TCP/IP that includes newer functions and structures used to retrieve IP addresses.
#include <ws2tcpip.h>
// The Iphlpapi.h header file is required if an application is using the IP Helper APIs. When the
// Iphlpapi.h header file is required, the Winsock2.h header should be included ahead.
#include <iphlpapi.h>

// As with winsock2.h, it's necessary to compile this cpp with the library ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename("EOF", "EndOfFile")

// TODO: reference additional headers your program requires here
#include "..\\BaseUtils\\ErrorHandler.h"
