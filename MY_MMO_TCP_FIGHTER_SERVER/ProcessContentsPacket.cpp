#include "Profiler.h"
#include "Network.h"
#include "ProcessContentsPacket.h"
#include "GameContentValueSetting.h"
#include "Log.h"
#include "Monitoring.h"
#include <map>

#define INVALID_ACTION	0xff
#define	INTERVAL_FPS(FRAME_COUNT)	1000 / FRAME_COUNT

static DWORD gCharacterID = 1;
static std::map<SOCKET, CharacterInfo*> characterList;
static const wchar_t* dirTable[8] = { L"LL", L"LU", L"UU", L"RU", L"RR", L"RD", L"DD", L"LD" };

void InitContents(void)
{
	SetProcessContentsAcceptEvent(ProcessAcceptEvent);
	SetProcessContentsDisconnectSessionEvent(ProcessDisconnectSessionEvent);
	SetPacketHeaderSize(sizeof(CommonPacketHeader));
	SetCheckIfCompltedPacketHandler(CheckIfCompletedPacket);
	SetDispatchPacketToContentsHandler(DispatchPacketToContents);
}

void ProcessAcceptEvent(void* param)
{
	CharacterInfo* characInfo = CreateCharacterInfo((SOCKET)param);

	SerializationBuffer sendPacket;
	MakePacketCreateMyCharater(&sendPacket, characInfo->characterID, dfPACKET_MOVE_DIR_LL, characInfo->xPos, characInfo->yPos, defCHARACTER_DEFAULT_HP);
	int i = SendUnicast((SOCKET)param, sendPacket.GetFrontBufferPtr(), sendPacket.GetUseSize());
	_Log(dfLOG_LEVEL_DEBUG, "CREATE CHARACTER_ID[%d] PACKET SEND [%d]"
		, characInfo->characterID, i);

	ConvertPacketCreateMyCharaterToCreateOtherCharacter(&sendPacket);
	// �ӽ÷� �ϴ�
	SendBroadcast(sendPacket.GetFrontBufferPtr(), sendPacket.GetUseSize(), (SOCKET)param);
	// Send Sector ���ֺ� ����;

	CharacterInfo* otherCharac;
	std::map<SOCKET, CharacterInfo*>::iterator iter = characterList.begin();
	for (; iter != characterList.end(); ++iter)
	{
		sendPacket.ClearBuffer();
		otherCharac = iter->second;
		MakePacketCreateOtherCharater(&sendPacket, otherCharac->characterID
			, otherCharac->stop2Dir, otherCharac->xPos, otherCharac->yPos, otherCharac->hp);
		SendUnicast((SOCKET)param, sendPacket.GetFrontBufferPtr(), sendPacket.GetUseSize());
	}

	characterList.insert({ (SOCKET)param, characInfo });
}

void ProcessDisconnectSessionEvent(void* param)
{
	SerializationBuffer sendPacket;
	MakePacketDeleteCharacter(&sendPacket, FindCharacter((SOCKET)param)->characterID);
	SendBroadcast(sendPacket.GetFrontBufferPtr(), sendPacket.GetUseSize(), (SOCKET)param);
	characterList.erase((SOCKET)param);
}

bool CheckIfCompletedPacket(char* packetHeader, int allRecivedPacketSize, int* outPacketBody)
{
	if (((CommonPacketHeader*)packetHeader)->byCode != dfPACKET_CODE)
	{
		return false;
	}

	int packetBodySize = ((CommonPacketHeader*)packetHeader)->bySize;
	if (packetBodySize + sizeof(CommonPacketHeader) > allRecivedPacketSize)
	{
		return false;
	}
	*outPacketBody = packetBodySize;
	return true;
}

CharacterInfo* CreateCharacterInfo(UINT_PTR sessionKey)
{
	CharacterInfo* characInfo = new CharacterInfo();
	characInfo->socket = (SOCKET)sessionKey;
	characInfo->characterID = gCharacterID;
	characInfo->hp = defCHARACTER_DEFAULT_HP;
	characInfo->action = INVALID_ACTION;
	characInfo->move8Dir = dfPACKET_MOVE_DIR_LL;
	characInfo->stop2Dir = dfPACKET_MOVE_DIR_LL;
	characInfo->xPos = rand() % dfRANGE_MOVE_RIGHT;
	characInfo->yPos = rand() % dfRANGE_MOVE_BOTTOM;

	gCharacterID += 1;
	return characInfo;
}

size_t GetCharacterCnt(void)
{
	return characterList.size();
}

CharacterInfo* FindCharacter(SOCKET socket)
{
	return characterList[socket];
}

bool DispatchPacketToContents(UINT_PTR sessionKey, char* tmpRecvPacketHeader, SerializationBuffer* tmpRecvPacketBody)
{
	switch (((CommonPacketHeader*)tmpRecvPacketHeader)->byType)
	{
	case dfPACKET_CS_MOVE_START:
		return ProcessPacketMoveStart(sessionKey, tmpRecvPacketBody);
	case dfPACKET_CS_MOVE_STOP:
		return ProcessPacketMoveStop(sessionKey, tmpRecvPacketBody);
	case dfPACKET_CS_ATTACK1:
		return ProcessPacketAttack1(sessionKey, tmpRecvPacketBody);
	case dfPACKET_CS_ATTACK2:
		return ProcessPacketAttack2(sessionKey, tmpRecvPacketBody);
	case dfPACKET_CS_ATTACK3:
		return ProcessPacketAttack3(sessionKey, tmpRecvPacketBody);
	case dfPACKET_CS_ECHO:
		return ProcessPacketEcho(sessionKey, tmpRecvPacketBody);
	}
	return false;
}

void MakePacketEcho(SerializationBuffer* packetBuf, DWORD time)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(time);
	packetHeader.byType = dfPACKET_SC_ECHO;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << time;
}

void MakePacketSyncXYPos(SerializationBuffer* packetBuf, DWORD id, WORD xPos, WORD yPos)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(xPos) + sizeof(yPos);
	packetHeader.byType = dfPACKET_SC_SYNC;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << xPos << yPos;
}

void MakePacketCreateMyCharater(SerializationBuffer* packetBuf, DWORD id, BYTE stop2Dir, WORD xPos, WORD yPos, BYTE hp)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(stop2Dir) + sizeof(xPos) + sizeof(yPos) + sizeof(hp);
	packetHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << stop2Dir << xPos << yPos << hp;
}

void MakePacketCreateOtherCharater(SerializationBuffer* packetBuf, DWORD id, BYTE stop2Dir, WORD xPos, WORD yPos, BYTE hp)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(stop2Dir) + sizeof(xPos) + sizeof(yPos) + sizeof(hp);
	packetHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << stop2Dir << xPos << yPos << hp;
}

void ConvertPacketCreateMyCharaterToCreateOtherCharacter(SerializationBuffer* packetBuf)
{
	((CommonPacketHeader*)packetBuf->GetFrontBufferPtr())->byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
}

void MakePacketDeleteCharacter(SerializationBuffer* packetBuf, DWORD id)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id);
	packetHeader.byType = dfPACKET_SC_DELETE_CHARACTER;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id;
}

void MakePacketMoveStart(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(dir) + sizeof(xPos) + sizeof(yPos);
	packetHeader.byType = dfPACKET_SC_MOVE_START;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << dir << xPos << yPos;
}

void MakePacketMoveStop(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(dir) + sizeof(xPos) + sizeof(yPos);
	packetHeader.byType = dfPACKET_SC_MOVE_STOP;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << dir << xPos << yPos;
}

void MakePacketAttack1(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(dir) + sizeof(xPos) + sizeof(yPos);
	packetHeader.byType = dfPACKET_SC_ATTACK1;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << dir << xPos << yPos;
}

void MakePacketAttack2(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(dir) + sizeof(xPos) + sizeof(yPos);
	packetHeader.byType = dfPACKET_SC_ATTACK2;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << dir << xPos << yPos;
}

void MakePacketAttack3(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos)
{
	CommonPacketHeader packetHeader;
	packetHeader.byCode = dfPACKET_CODE;
	packetHeader.bySize = sizeof(id) + sizeof(dir) + sizeof(xPos) + sizeof(yPos);
	packetHeader.byType = dfPACKET_SC_ATTACK3;

	packetBuf->Enqueue((char*)&packetHeader, sizeof(CommonPacketHeader));
	(*packetBuf) << id << dir << xPos << yPos;
}

bool ProcessPacketMoveStart(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody)
{
	SerializationBuffer packetBuf;
	BYTE move8Dir;
	WORD clientXpos;
	WORD clientYpos;
	*tmpRecvPacketBody >> move8Dir >> clientXpos >> clientYpos;
	CharacterInfo* ptrCharacter = FindCharacter(sessionKey);

	_Log(dfLOG_LEVEL_DEBUG, "CHARACTER_ID[%d] PACKET MOVE_START [DIR: %s/X: %d/Y: %d]"
		, ptrCharacter->characterID, dirTable[move8Dir], clientXpos, clientYpos);

	if (abs(ptrCharacter->xPos - clientXpos) > dfERROR_RANGE
		|| abs(ptrCharacter->yPos - clientYpos) > dfERROR_RANGE)
	{	
		DWORD currentTick = timeGetTime();
		_Log(dfLOG_LEVEL_SYSTEM, "��ũ �߻�: CHARACTER_ID[%d] [tickInterval: %d] [actionX: %d/actionY: %d] [dir: %s/cx: %d/cy: %d] ==> [dir: %s/sx: %d/sy: %d]"
			, ptrCharacter->characterID, currentTick - ptrCharacter->dwActionTick, ptrCharacter->actionXpos, ptrCharacter->actionYpos, dirTable[move8Dir], clientXpos, clientYpos
			, dirTable[ptrCharacter->move8Dir], ptrCharacter->xPos, ptrCharacter->yPos);
		clientXpos = ptrCharacter->xPos;
		clientYpos = ptrCharacter->yPos;

		MakePacketSyncXYPos(&packetBuf, ptrCharacter->characterID, clientXpos, clientYpos);
		SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		packetBuf.ClearBuffer();
	}
	else
	{
		ptrCharacter->xPos = clientXpos;
		ptrCharacter->yPos = clientYpos;
	}

	ptrCharacter->action = move8Dir;
	ptrCharacter->move8Dir = move8Dir;

	switch (move8Dir)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
		ptrCharacter->stop2Dir = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RD:
		ptrCharacter->stop2Dir = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LD:
		ptrCharacter->stop2Dir = dfPACKET_MOVE_DIR_LL;
		break;
	}

	PRO_BEGIN(L"MakePacketMoveStart");
	MakePacketMoveStart(&packetBuf, ptrCharacter->characterID, move8Dir, clientXpos, clientYpos);
	PRO_END(L"MakePacketMoveStart");
	PRO_BEGIN(L"SendBroadcast");
	SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharacter->socket);
	PRO_END(L"SendBroadcast");

	ptrCharacter->dwActionTick = timeGetTime();
	ptrCharacter->actionXpos = clientXpos;
	ptrCharacter->actionYpos = clientYpos;
	return true;
}

bool ProcessPacketMoveStop(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody)
{
	SerializationBuffer packetBuf;
	BYTE stop2Dir;
	WORD clientXpos;
	WORD clientYpos;

	*tmpRecvPacketBody >> stop2Dir >> clientXpos >> clientYpos;
	CharacterInfo* ptrCharacter = FindCharacter(sessionKey);

	_Log(dfLOG_LEVEL_DEBUG, "CHARACTER_ID[%d] PACKET MOVE_STOP [DIR: %s/X: %d/Y: %d]"
		, ptrCharacter->characterID, dirTable[stop2Dir], clientXpos, clientYpos);

	if (abs(ptrCharacter->xPos - clientXpos) > dfERROR_RANGE
		|| abs(ptrCharacter->yPos - clientYpos) > dfERROR_RANGE)
	{
		DWORD currentTick = timeGetTime();
		_Log(dfLOG_LEVEL_SYSTEM, "��ũ �߻�: CHARACTER_ID[%d] [tickInterval: %d] [actionX: %d/actionY: %d] [dir: %s/cx: %d/cy: %d] ==> [dir: %s/sx: %d/sy: %d]"
			, ptrCharacter->characterID, currentTick - ptrCharacter->dwActionTick, ptrCharacter->actionXpos, ptrCharacter->actionYpos, dirTable[stop2Dir], clientXpos, clientYpos
			, dirTable[ptrCharacter->move8Dir], ptrCharacter->xPos, ptrCharacter->yPos);
		clientXpos = ptrCharacter->xPos;
		clientYpos = ptrCharacter->yPos;

		MakePacketSyncXYPos(&packetBuf, ptrCharacter->characterID, clientXpos, clientYpos);
		SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		packetBuf.ClearBuffer();
	}
	else
	{
		ptrCharacter->xPos = clientXpos;
		ptrCharacter->yPos = clientYpos;
	}

	ptrCharacter->action = INVALID_ACTION;
	ptrCharacter->stop2Dir = stop2Dir;

	MakePacketMoveStop(&packetBuf, ptrCharacter->characterID, stop2Dir, clientXpos, clientYpos);
	SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharacter->socket);

	ptrCharacter->dwActionTick = timeGetTime();
	ptrCharacter->actionXpos = clientXpos;
	ptrCharacter->actionYpos = clientYpos;

	return true;
}

bool ProcessPacketAttack1(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody)
{
	SerializationBuffer packetBuf;
	BYTE stop2Dir;
	WORD clientXpos;
	WORD clientYpos;

	*tmpRecvPacketBody >> stop2Dir >> clientXpos >> clientYpos; 
	CharacterInfo* ptrCharacter = FindCharacter(sessionKey);
	if (abs(ptrCharacter->xPos - clientXpos) > dfERROR_RANGE
		|| abs(ptrCharacter->yPos - clientYpos) > dfERROR_RANGE)
	{
		_Log(dfLOG_LEVEL_SYSTEM, "��ũ �߻�: CHARACTER_ID[%d] [dir: %s/cx: %d/cy: %d] ==> [dir: %s/sx: %d/sy: %d]"
			, ptrCharacter->characterID, dirTable[stop2Dir], clientXpos, clientYpos
			, dirTable[ptrCharacter->stop2Dir], ptrCharacter->xPos, ptrCharacter->yPos);
		clientXpos = ptrCharacter->xPos;
		clientYpos = ptrCharacter->yPos;

		MakePacketSyncXYPos(&packetBuf, ptrCharacter->characterID, clientXpos, clientYpos);
		SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		packetBuf.ClearBuffer();
	}
	else
	{
		ptrCharacter->xPos = clientXpos;
		ptrCharacter->yPos = clientYpos;
	}

	ptrCharacter->stop2Dir = stop2Dir;

	MakePacketAttack1(&packetBuf, ptrCharacter->characterID, stop2Dir, clientXpos, clientYpos);
	SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharacter->socket);
	return true;
}

bool ProcessPacketAttack2(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody)
{
	SerializationBuffer packetBuf;
	BYTE stop2Dir;
	WORD clientXpos;
	WORD clientYpos;

	*tmpRecvPacketBody >> stop2Dir >> clientXpos >> clientYpos;
	CharacterInfo* ptrCharacter = FindCharacter(sessionKey);
	if (abs(ptrCharacter->xPos - clientXpos) > dfERROR_RANGE
		|| abs(ptrCharacter->yPos - clientYpos) > dfERROR_RANGE)
	{
		_Log(dfLOG_LEVEL_SYSTEM, "��ũ �߻�: CHARACTER_ID[%d] [dir: %s/cx: %d/cy: %d] ==> [dir: %s/sx: %d/sy: %d]"
			, ptrCharacter->characterID, dirTable[stop2Dir], clientXpos, clientYpos
			, dirTable[ptrCharacter->stop2Dir], ptrCharacter->xPos, ptrCharacter->yPos);
		clientXpos = ptrCharacter->xPos;
		clientYpos = ptrCharacter->yPos;

		MakePacketSyncXYPos(&packetBuf, ptrCharacter->characterID, clientXpos, clientYpos);
		SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		packetBuf.ClearBuffer();
	}
	else
	{
		ptrCharacter->xPos = clientXpos;
		ptrCharacter->yPos = clientYpos;
	}

	ptrCharacter->stop2Dir = stop2Dir;

	MakePacketAttack2(&packetBuf, ptrCharacter->characterID, stop2Dir, clientXpos, clientYpos);
	SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharacter->socket);
	return true;
}

bool ProcessPacketAttack3(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody)
{
	SerializationBuffer packetBuf;
	BYTE stop2Dir;
	WORD clientXpos;
	WORD clientYpos;

	*tmpRecvPacketBody >> stop2Dir >> clientXpos >> clientYpos;
	CharacterInfo* ptrCharacter = FindCharacter(sessionKey);
	if (abs(ptrCharacter->xPos - clientXpos) > dfERROR_RANGE
		|| abs(ptrCharacter->yPos - clientYpos) > dfERROR_RANGE)
	{
		_Log(dfLOG_LEVEL_SYSTEM, "��ũ �߻�: CHARACTER_ID[%d] [dir: %s/cx: %d/cy: %d] ==> [dir: %s/sx: %d/sy: %d]"
			, ptrCharacter->characterID, dirTable[stop2Dir], clientXpos, clientYpos
			, dirTable[ptrCharacter->stop2Dir], ptrCharacter->xPos, ptrCharacter->yPos);
		clientXpos = ptrCharacter->xPos;
		clientYpos = ptrCharacter->yPos;

		MakePacketSyncXYPos(&packetBuf, ptrCharacter->characterID, clientXpos, clientYpos);
		SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		packetBuf.ClearBuffer();
	}
	else
	{
		ptrCharacter->xPos = clientXpos;
		ptrCharacter->yPos = clientYpos;
	}

	ptrCharacter->stop2Dir = stop2Dir;

	MakePacketAttack3(&packetBuf, ptrCharacter->characterID, stop2Dir, clientXpos, clientYpos);
	SendBroadcast(packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharacter->socket);
	return true;
}

bool ProcessPacketEcho(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody)
{
	SerializationBuffer packetBuf;
	DWORD time;

	*tmpRecvPacketBody >> time;
	MakePacketEcho(&packetBuf, time);
	SendUnicast(sessionKey, packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
	return true;
}

void Update()
{
	CountLoop();
	static DWORD startTime = StartMonitor();
	DWORD endTime = timeGetTime();
	DWORD intervalTime = endTime - startTime;
	if (intervalTime < INTERVAL_FPS(25))
	{
		return;
	}
	startTime = endTime - (intervalTime - INTERVAL_FPS(25));
	/*_Log(dfLOG_LEVEL_SYSTEM, "���� ����: %d"
		, intervalTime - INTERVAL_FPS(25));*/
	CountFrame();

	CharacterInfo* ptrCharac;
	std::map<SOCKET, CharacterInfo*>::iterator iter = characterList.begin();
	for (; iter != characterList.end();)
	{
		ptrCharac = (iter++)->second;
		if (ptrCharac->hp < 1)
		{
			DisconnectSession(ptrCharac->socket);
		}
		else if (endTime - FindSession(ptrCharac->socket)->lastRecvTime > dfNETWORK_PACKET_RECV_TIMEOUT)
		{
			DisconnectSession(ptrCharac->socket);
		}
		else if (ptrCharac->action != INVALID_ACTION)
		{
			switch (ptrCharac->action)
			{
			case dfPACKET_MOVE_DIR_LL:
				ptrCharac->xPos = max(ptrCharac->xPos - dfSPEED_PLAYER_X, dfRANGE_MOVE_LEFT);
				break;
			case dfPACKET_MOVE_DIR_LU:
				ptrCharac->xPos = max(ptrCharac->xPos - dfSPEED_PLAYER_X, dfRANGE_MOVE_LEFT);
				ptrCharac->yPos = max(ptrCharac->yPos - dfSPEED_PLAYER_Y, dfRANGE_MOVE_TOP);
				break;
			case dfPACKET_MOVE_DIR_LD:
				ptrCharac->xPos = max(ptrCharac->xPos - dfSPEED_PLAYER_X, dfRANGE_MOVE_LEFT);
				ptrCharac->yPos = min(ptrCharac->yPos + dfSPEED_PLAYER_Y, dfRANGE_MOVE_BOTTOM);
				break;
			case dfPACKET_MOVE_DIR_UU:
				ptrCharac->yPos = max(ptrCharac->yPos - dfSPEED_PLAYER_Y, dfRANGE_MOVE_TOP);
				break;
			case dfPACKET_MOVE_DIR_RU:
				ptrCharac->xPos = min(ptrCharac->xPos + dfSPEED_PLAYER_X, dfRANGE_MOVE_RIGHT);
				ptrCharac->yPos = max(ptrCharac->yPos - dfSPEED_PLAYER_Y, dfRANGE_MOVE_TOP);
				break;
			case dfPACKET_MOVE_DIR_RR:
				ptrCharac->xPos = min(ptrCharac->xPos + dfSPEED_PLAYER_X, dfRANGE_MOVE_RIGHT);
				break;
			case dfPACKET_MOVE_DIR_RD:
				ptrCharac->xPos = min(ptrCharac->xPos + dfSPEED_PLAYER_X, dfRANGE_MOVE_RIGHT);
				ptrCharac->yPos = min(ptrCharac->yPos + dfSPEED_PLAYER_Y, dfRANGE_MOVE_BOTTOM);
				break;
			case dfPACKET_MOVE_DIR_DD:
				ptrCharac->yPos = min(ptrCharac->yPos + dfSPEED_PLAYER_Y, dfRANGE_MOVE_BOTTOM);
				break;
			}

			/*_Log(dfLOG_LEVEL_SYSTEM, "[SESSION_ID=%d] gameRun: %hs, X: %d, Y: %d"
				, ptrCharac->characterID, dirTable[ptrCharac->action]
				, ptrCharac->xPos, ptrCharac->yPos);*/

			/*���� ������Ʈ �ڵ�*/
		}
	}
}