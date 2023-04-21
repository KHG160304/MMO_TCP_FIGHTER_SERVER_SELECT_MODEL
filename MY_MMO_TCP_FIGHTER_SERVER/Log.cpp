#include "Log.h"

int		gLogLevel = dfLOG_LEVEL_SYSTEM;
wchar_t	gLogBuffer[2048 + 1];
const wchar_t* strLogLevel[3] = { L"[DEBUG]", L"[ERROR]", L"[SYSTEM]" };

void Log(wchar_t* strLog, int logLevel)
{
	wprintf_s(L"%s\n", strLog);
}

