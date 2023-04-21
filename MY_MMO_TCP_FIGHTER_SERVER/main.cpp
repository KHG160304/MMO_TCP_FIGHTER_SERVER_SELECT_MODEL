#include <stdio.h>
#include <conio.h>
#include "Log.h"
#include "Network.h"
#include "ProcessContentsPacket.h"
#include "Monitoring.h"
#include <locale.h>
#include "Profiler.h"

#define	FPS(FRAME_COUNT)	1000 / FRAME_COUNT

static bool gIsServerRunning = true;

void ServerControl()
{
	int key;
	if (_kbhit())
	{
		key = _getch();
		if (key == 'q' || key == 'Q')
		{
			_Log(dfLOG_LEVEL_SYSTEM, "서버 종료");
			gIsServerRunning = false;
		}

		if (key == 's' || key == 'S')
		{
			SaveProfileSampleToText(L"MMO_TCP_FIGHTER_LOG");
		}
	}
}

int main(void)
{
	setlocale(LC_ALL, "ko");
	InitContents();
	SetCharacterCntHandle(GetCharacterCnt);
	SetSessionCntHandle(GetSessionCnt);
	SetSectorCharacterCntHandle(GetSectorCharacterCnt);
	if (!InitNetwork())
	{
		ShutdownServer();
		return -1;
	}

	while (gIsServerRunning)
	{
		PRO_BEGIN(L"RTT");
		PRO_BEGIN(L"ProcessNetworkIOEvent");
		ProcessNetworkIOEvent();
		PRO_END(L"ProcessNetworkIOEvent");

		PRO_BEGIN(L"Update");
		Update();
		PRO_END(L"Update");

		ServerControl();

		Monitoring();
		PRO_END(L"RTT");
	}

	ShutdownServer();
	return 0;
}