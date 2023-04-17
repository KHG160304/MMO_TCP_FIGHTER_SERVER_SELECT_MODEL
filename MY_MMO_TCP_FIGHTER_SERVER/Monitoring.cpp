#include "Monitoring.h"
#include "Log.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib")

DWORD fps = 0;
DWORD loopCnt = 0;
DWORD startMeasurementTime = 0;
size_t (*GetCharacterCnt)(void);
size_t (*GetSessionCnt)(void);

DWORD StartMonitor()
{
	startMeasurementTime = timeGetTime();
	return startMeasurementTime;
}

void Monitoring()
{
	DWORD endMeasurementTime = timeGetTime();
	DWORD interval = endMeasurementTime - startMeasurementTime;
	if (interval >= 1000)
	{
		_Log(dfLOG_LEVEL_SYSTEM, "FPS: %d, Loop: %d, Session Count: %lld, Character Count: %lld", fps, loopCnt, GetSessionCnt(), GetCharacterCnt());
		startMeasurementTime = endMeasurementTime - (interval - 1000);
		fps = 0;
		loopCnt = 0;
	}
}

void CountFrame()
{
	fps += 1;
}

void CountLoop()
{
	loopCnt += 1;
}

void SetCharacterCntHandle(size_t (*size)(void))
{
	GetCharacterCnt = size;
}

void SetSessionCntHandle(size_t (*size)(void))
{
	GetSessionCnt = size;
}
