#include "Log.h"
#include "CharacterInfo.h"
#include "Sector.h"
#include "Network.h"
#include "ProcessContentsPacket.h"

std::map<DWORD, CharacterInfo*> sectorList[dfSECTOR_HEIGHT][dfSECTOR_WIDTH];
std::map<DWORD, SectorHistory> sectorHistoryList;

void GetAroundSector(CharacterInfo* charac, SectorAround* outSectorAround)
{
	WORD sectorXpos = charac->xPos / dfSIX_FRAME_X_DISTANCE;
	WORD sectorYpos = charac->yPos / dfSIX_FRAME_Y_DISTANCE;

	WORD sectorXStart;
	WORD sectorXEnd;
	WORD sectorYStart;
	WORD sectorYEnd;
	if (sectorXpos == 0)
	{
		sectorXStart = 0;
		sectorXEnd = 1;
	}
	else if (sectorXpos == dfSECTOR_WIDTH - 1)
	{
		sectorXStart = dfSECTOR_WIDTH - 2;
		sectorXEnd = dfSECTOR_WIDTH - 1;
	}
	else
	{
		sectorXStart = sectorXpos - 1;
		sectorXEnd = sectorXpos + 1;
	}

	if (sectorYpos == 0)
	{
		sectorYStart = 0;
		sectorYEnd = 1;
	}
	else if (sectorYpos == dfSECTOR_HEIGHT - 1)
	{
		sectorYStart = dfSECTOR_HEIGHT - 2;
		sectorYEnd = dfSECTOR_HEIGHT - 1;
	}
	else
	{
		sectorYStart = sectorYpos - 1;
		sectorYEnd = sectorYpos + 1;
	}

	int userSectorWidth = sectorXEnd - sectorXStart + 1;
	int userSectorHeight = sectorYEnd - sectorYStart + 1;
	outSectorAround->cnt = userSectorWidth * userSectorHeight;
	for (int y = 0; y < userSectorHeight; ++y)
	{
		for (int x = 0; x < userSectorWidth; ++x)
		{
			outSectorAround->around[y * userSectorWidth + x].xPos = sectorXStart + x;
			outSectorAround->around[y * userSectorWidth + x].yPos = sectorYStart + y;

		}
	}
}

void GetAroundSector(SectorPos pos, SectorAround* outSectorAround)
{
	WORD sectorXpos = pos.xPos;
	WORD sectorYpos = pos.yPos;

	WORD sectorXStart;
	WORD sectorXEnd;
	WORD sectorYStart;
	WORD sectorYEnd;
	if (sectorXpos == 0)
	{
		sectorXStart = 0;
		sectorXEnd = 1;
	}
	else if (sectorXpos == dfSECTOR_WIDTH - 1)
	{
		sectorXStart = dfSECTOR_WIDTH - 2;
		sectorXEnd = dfSECTOR_WIDTH - 1;
	}
	else
	{
		sectorXStart = sectorXpos - 1;
		sectorXEnd = sectorXpos + 1;
	}

	if (sectorYpos == 0)
	{
		sectorYStart = 0;
		sectorYEnd = 1;
	}
	else if (sectorYpos == dfSECTOR_HEIGHT - 1)
	{
		sectorYStart = dfSECTOR_HEIGHT - 2;
		sectorYEnd = dfSECTOR_HEIGHT - 1;
	}
	else
	{
		sectorYStart = sectorYpos - 1;
		sectorYEnd = sectorYpos + 1;
	}

	int userSectorWidth = sectorXEnd - sectorXStart + 1;
	int userSectorHeight = sectorYEnd - sectorYStart + 1;
	outSectorAround->cnt = userSectorWidth * userSectorHeight;
	for (int y = 0; y < userSectorHeight; ++y)
	{
		for (int x = 0; x < userSectorWidth; ++x)
		{
			outSectorAround->around[y * userSectorWidth + x].xPos = sectorXStart + x;
			outSectorAround->around[y * userSectorWidth + x].yPos = sectorYStart + y;

		}
	}
}

void GetLeftUpSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.xPos == 0 && pos.yPos == 0)
	{
		outSectorAround->cnt = 0;
	}
	else if (pos.xPos == 0)
	{
		outSectorAround->around[0].xPos = 0;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = 1;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.yPos == 0)
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = 0;

		outSectorAround->around[1].xPos = pos.xPos - 1;
		outSectorAround->around[1].yPos = 1;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->around[2].xPos = pos.xPos - 1;
		outSectorAround->around[2].yPos = pos.yPos;

		outSectorAround->around[3].xPos = pos.xPos - 1;
		outSectorAround->around[3].yPos = pos.yPos + 1;

		outSectorAround->around[4].xPos = pos.xPos - 1;
		outSectorAround->around[4].yPos = pos.yPos - 1;

		outSectorAround->cnt = 5;
	}
}

void GetLeftDownSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.xPos == 0 && pos.yPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->cnt = 0;
		return;
	}
	else if (pos.xPos == 0)
	{
		outSectorAround->around[0].xPos = 0;
		outSectorAround->around[0].yPos = pos.yPos + 1;

		outSectorAround->around[1].xPos = 1;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.xPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->around[0].xPos = pos.xPos;
		outSectorAround->around[0].yPos = dfSECTOR_HEIGHT - 1;

		outSectorAround->around[1].xPos = pos.yPos;
		outSectorAround->around[1].yPos = dfSECTOR_HEIGHT - 2;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos;
		outSectorAround->around[0].yPos = pos.yPos + 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->around[2].xPos = pos.xPos - 1;
		outSectorAround->around[2].yPos = pos.yPos;

		outSectorAround->around[3].xPos = pos.xPos - 1;
		outSectorAround->around[3].yPos = pos.yPos - 1;

		outSectorAround->around[4].xPos = pos.xPos - 1;
		outSectorAround->around[4].yPos = pos.yPos + 1;

		outSectorAround->cnt = 5;
	}
}

void GetRightUpSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.xPos == dfSECTOR_WIDTH - 1 && pos.yPos == 0)
	{
		outSectorAround->cnt = 0;
	}
	else if (pos.xPos == dfSECTOR_WIDTH - 1)
	{
		outSectorAround->around[0].xPos = dfSECTOR_WIDTH - 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = dfSECTOR_WIDTH - 2;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.yPos == 0)
	{
		outSectorAround->around[0].xPos = dfSECTOR_WIDTH - 1;
		outSectorAround->around[0].yPos = 0;

		outSectorAround->around[1].xPos = dfSECTOR_WIDTH - 1;
		outSectorAround->around[1].yPos = 1;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos - 1;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->around[2].xPos = pos.xPos + 1;
		outSectorAround->around[2].yPos = pos.yPos;

		outSectorAround->around[3].xPos = pos.xPos + 1;
		outSectorAround->around[3].yPos = pos.yPos + 1;

		outSectorAround->around[4].xPos = pos.xPos + 1;
		outSectorAround->around[4].yPos = pos.yPos - 1;

		outSectorAround->cnt = 5;
	}
}

void GetRightDownSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.xPos == dfSECTOR_WIDTH - 1 && pos.yPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->cnt = 0;
	}
	else if (pos.xPos == dfSECTOR_WIDTH - 1)
	{
		outSectorAround->around[0].xPos = dfSECTOR_WIDTH - 1;
		outSectorAround->around[0].yPos = pos.yPos + 1;

		outSectorAround->around[1].xPos = dfSECTOR_WIDTH - 2;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.yPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->around[0].xPos = pos.xPos + 1;
		outSectorAround->around[0].yPos = dfSECTOR_HEIGHT - 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = dfSECTOR_HEIGHT - 2;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos + 1;
		outSectorAround->around[0].yPos = pos.yPos;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->around[2].xPos = pos.xPos;
		outSectorAround->around[2].yPos = pos.yPos + 1;

		outSectorAround->around[3].xPos = pos.xPos - 1;
		outSectorAround->around[3].yPos = pos.yPos + 1;

		outSectorAround->around[4].xPos = pos.xPos + 1;
		outSectorAround->around[4].yPos = pos.yPos + 1;

		outSectorAround->cnt = 5;
	}
}

void GetLeftSector(SectorPos pos, SectorAround* outSectorAround)
{

	if (pos.xPos == 0)
	{
		outSectorAround->cnt = 0;
		return;
	}

	if (pos.yPos == 0)
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos;

		outSectorAround->around[1].xPos = pos.xPos - 1;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.yPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos - 1;
		outSectorAround->around[1].yPos = pos.yPos;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos - 1;
		outSectorAround->around[1].yPos = pos.yPos;

		outSectorAround->around[2].xPos = pos.xPos - 1;
		outSectorAround->around[2].yPos = pos.yPos + 1;

		outSectorAround->cnt = 3;
	}
}

void GetRightSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.xPos == dfSECTOR_WIDTH - 1)
	{
		outSectorAround->cnt = 0;
		return;
	}

	if (pos.yPos == 0)
	{
		outSectorAround->around[0].xPos = pos.xPos + 1;
		outSectorAround->around[0].yPos = pos.yPos;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.yPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->around[0].xPos = pos.xPos + 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos + 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos;

		outSectorAround->around[2].xPos = pos.xPos + 1;
		outSectorAround->around[2].yPos = pos.yPos + 1;

		outSectorAround->cnt = 3;
	}

	
}

void GetUpSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.yPos == 0)
	{
		outSectorAround->cnt = 0;
		return;
	}

	if (pos.xPos == 0)
	{
		outSectorAround->around[0].xPos = pos.xPos;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.xPos == dfSECTOR_WIDTH - 1)
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos - 1;

		outSectorAround->around[1].xPos = pos.xPos;
		outSectorAround->around[1].yPos = pos.yPos - 1;

		outSectorAround->around[2].xPos = pos.xPos + 1;
		outSectorAround->around[2].yPos = pos.yPos - 1;

		outSectorAround->cnt = 3;
	}
}

void GetDownSector(SectorPos pos, SectorAround* outSectorAround)
{
	if (pos.yPos == dfSECTOR_HEIGHT - 1)
	{
		outSectorAround->cnt = 0;
		return;
	}

	if (pos.xPos == 0)
	{
		outSectorAround->around[0].xPos = pos.xPos;
		outSectorAround->around[0].yPos = pos.yPos + 1;

		outSectorAround->around[1].xPos = pos.xPos + 1;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->cnt = 2;
	}
	else if (pos.xPos == dfSECTOR_WIDTH - 1)
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos + 1;

		outSectorAround->around[1].xPos = pos.xPos;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->cnt = 2;
	}
	else
	{
		outSectorAround->around[0].xPos = pos.xPos - 1;
		outSectorAround->around[0].yPos = pos.yPos + 1;

		outSectorAround->around[1].xPos = pos.xPos;
		outSectorAround->around[1].yPos = pos.yPos + 1;

		outSectorAround->around[2].xPos = pos.xPos + 1;
		outSectorAround->around[2].yPos = pos.yPos + 1;

		outSectorAround->cnt = 3;
	}
	
}

SectorPos ConvertCharcterPosToSectorPos(CharacterInfo* charac)
{
	SectorPos pos;
	pos.xPos = charac->xPos / dfSIX_FRAME_X_DISTANCE;
	pos.yPos = charac->yPos / dfSIX_FRAME_Y_DISTANCE;
	return pos;
}

void AddToSector(CharacterInfo* charac)
{
	WORD sectorXpos = charac->xPos / dfSIX_FRAME_X_DISTANCE;
	WORD sectorYpos = charac->yPos / dfSIX_FRAME_Y_DISTANCE;

	sectorList[sectorYpos][sectorXpos].insert({ charac->characterID, charac });
	SectorHistory history;
	history.prevSectorPos = { sectorXpos, sectorYpos };
	sectorHistoryList.insert({ charac->characterID,  history });
}

void AddToSector(SectorPos pos, CharacterInfo* charac)
{
	sectorList[pos.yPos][pos.xPos].insert({ charac->characterID, charac });
}

void RemoveToSector(CharacterInfo* charac)
{
	WORD sectorXpos = charac->xPos / dfSIX_FRAME_X_DISTANCE;
	WORD sectorYpos = charac->yPos / dfSIX_FRAME_Y_DISTANCE;

	sectorList[sectorYpos][sectorXpos].erase(charac->characterID);
}

void RemoveToSector(SectorPos pos, CharacterInfo* charac)
{
	sectorList[pos.yPos][pos.xPos].erase(charac->characterID);
}

void SendAroundSector(CharacterInfo* charac, const char* buf, int size, bool excludeMe)
{
	SectorAround aroundSectorList;
	GetAroundSector(charac, &aroundSectorList);

	SectorPos sectorPos;

	if (excludeMe == false)
	{
		for (int i = 0; i < aroundSectorList.cnt; ++i)
		{
			sectorPos = aroundSectorList.around[i];
			std::map<DWORD, CharacterInfo*>::iterator iter = sectorList[sectorPos.yPos][sectorPos.xPos].begin();
			for (; iter != sectorList[sectorPos.yPos][sectorPos.xPos].end(); ++ iter)
			{
				SendUnicast(iter->second->socket, buf, size);
			}
		}
		return;
	}

	for (int i = 0; i < aroundSectorList.cnt; ++i)
	{
		sectorPos = aroundSectorList.around[i];
		std::map<DWORD, CharacterInfo*>::iterator iter = sectorList[sectorPos.yPos][sectorPos.xPos].begin();
		for (; iter != sectorList[sectorPos.yPos][sectorPos.xPos].end(); ++iter)
		{
			if (iter->first != charac->characterID)
			{
				SendUnicast(iter->second->socket, buf, size);
			}
		}
	}
}

void SendAroundSector(const SectorAround& aroundSectorList, const char* buf, int size, DWORD id)
{
	_Log(dfLOG_LEVEL_SYSTEM, "Send AroundSector packet size: %d, charac id: %d", size, id);
	for (int i = 0; i < size; ++i)
	{
		_Log(dfLOG_LEVEL_SYSTEM, "0x%02x", buf[i]);
	}

	if (id == INVALID_CHARACTER_ID)
	{
		SectorPos sectorPos;
		for (int i = 0; i < aroundSectorList.cnt; ++i)
		{
			sectorPos = aroundSectorList.around[i];
			std::map<DWORD, CharacterInfo*>::iterator iter = sectorList[sectorPos.yPos][sectorPos.xPos].begin();
			for (; iter != sectorList[sectorPos.yPos][sectorPos.xPos].end(); ++iter)
			{
				SendUnicast(iter->second->socket, buf, size);
			}
		}
		return;
	}

	SectorPos sectorPos;
	for (int i = 0; i < aroundSectorList.cnt; ++i)
	{
		sectorPos = aroundSectorList.around[i];
		std::map<DWORD, CharacterInfo*>::iterator iter = sectorList[sectorPos.yPos][sectorPos.xPos].begin();
		for (; iter != sectorList[sectorPos.yPos][sectorPos.xPos].end(); ++iter)
		{
			if (iter->first != id)
			{
				SendUnicast(iter->second->socket, buf, size);
			}
		}
	}
}

SectorHistory& FindSectorHistory(CharacterInfo* charac)
{
	return sectorHistoryList[charac->characterID];
}

bool isChangedSectorPos(CharacterInfo* charac)
{
	WORD sectorXpos = charac->xPos / dfSIX_FRAME_X_DISTANCE;
	WORD sectorYpos = charac->yPos / dfSIX_FRAME_Y_DISTANCE;

	SectorHistory sectorHistory = FindSectorHistory(charac);
	if (sectorHistory.prevSectorPos.xPos != sectorXpos
		|| sectorHistory.prevSectorPos.yPos != sectorYpos)
	{
		return true;
	}

	return false;
}

void UpdateSectorHistory(CharacterInfo* charac)
{
	SectorHistory sectorHistory = FindSectorHistory(charac);
	sectorHistory.prevSectorPos.xPos = charac->xPos;
	sectorHistory.prevSectorPos.yPos = charac->yPos;
}

void UpdateSector(CharacterInfo* charac)
{
	if (!isChangedSectorPos(charac))
	{
		return;
	}

	SerializationBuffer packet(20);

	SectorHistory sectorHistory = FindSectorHistory(charac);
	SectorAround sectorAround;
	SectorAround deleteSectorAround;
	SectorPos newSectorPos;
	switch (charac->action)
	{
	case dfPACKET_MOVE_DIR_LL:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetLeftSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetRightSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_LU:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetLeftUpSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetRightDownSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_UU:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetUpSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetDownSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_RU:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetRightUpSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetLeftDownSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_RR:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetRightSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetLeftSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_RD:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetRightDownSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetLeftUpSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_DD:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetDownSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetUpSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	case dfPACKET_MOVE_DIR_LD:
		newSectorPos = ConvertCharcterPosToSectorPos(charac);
		GetLeftDownSector(newSectorPos, &sectorAround);
		if (sectorAround.cnt)
		{
			MakePacketCreateOtherCharater(&packet, charac->characterID, charac->stop2Dir, charac->xPos, charac->yPos, charac->hp);
			MakePacketMoveStart(&packet, charac->characterID, charac->action, charac->xPos, charac->yPos);
			AddToSector(newSectorPos, charac);
			SendAroundSector(sectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			packet.ClearBuffer();

			GetRightUpSector(sectorHistory.prevSectorPos, &deleteSectorAround);
			MakePacketDeleteCharacter(&packet, charac->characterID);
			SendAroundSector(deleteSectorAround, packet.GetFrontBufferPtr(), packet.GetUseSize(), charac->characterID);
			RemoveToSector(sectorHistory.prevSectorPos, charac);
		}
		break;
	}

	UpdateSectorHistory(charac);
}
