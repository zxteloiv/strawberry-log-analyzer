// LogServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "LogHandler.h"
#include "FixedLengthCompare.h"
#include "..\\Toolkit\\Toolkit.h"
#include <string>

using namespace LogServer;

TCHAR* xml = _T("<?xml version=\"1.0\" encoding=\"ucs-2\"?> <LogDefinition encoding=\"ascii\" type=\"text\"> <Record FieldsCount=\"5\" StartSymbol=\"\" EndSymbol=\"\n\"> <Field StartSymbol=\"\" Split=\",\" EndSymbol=\"\" trim=\"true\" type=\"string\" alias=\"user id\"/> <Field StartSymbol=\"\" Split=\",\" EndSymbol=\"\" trim=\"true\" type=\"string\" alias=\"search keywords\" /> <Field StartSymbol=\"\" Split=\",\" EndSymbol=\"\" trim=\"true\" type=\"string\" alias=\"url\" /> <Field StartSymbol=\"\" Split=\",\" EndSymbol=\"\" trim=\"true\" type=\"int\" alias=\"rank\" /> <Field StartSymbol=\"\" Split=\"\n\" EndSymbol=\"\" trim=\"true\" type=\"time\" alias=\"click time\" /> </Record> </LogDefinition>");

int _tmain(int argc, _TCHAR* argv[]) {
	LogHandler::Startup();
	if (LogHandler::Init() && LogHandler::ListenAsync()) {
		_tprintf(L"Now waiting...");
		while (true) {
			Sleep(3600000);
		}
	}

	LogHandler::CleanUpHandler();

	system("pause");
	return 0;
}

