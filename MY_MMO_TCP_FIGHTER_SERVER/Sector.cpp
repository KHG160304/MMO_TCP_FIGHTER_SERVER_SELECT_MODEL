#include "Log.h"
#include "Sector.h"
#include "CharacterInfo.h"
#include "Network.h"
#include "ProcessContentsPacket.h"
#include "GameContentValueSetting.h"

static const int SECTOR_HEIGHT = dfSECTOR_HEIGHT;
static const int SECTOR_WIDTH = dfSECTOR_WIDTH;

std::map<DWORD, CharacterInfo*> sectorList[SECTOR_HEIGHT][SECTOR_WIDTH];

SectorPos ConvertWorldPosToSectorPos(WORD worldXPos, WORD worldYPos)
{
	return {
		(WORD)(worldXPos / dfSIX_FRAME_X_DISTANCE),
		(WORD)(worldYPos / dfSIX_FRAME_Y_DISTANCE)
	};
}

void Sector_AddCharacter(CharacterInfo* charac)
{
	sectorList[charac->curPos.yPos][charac->curPos.xPos].insert({ charac->characterID, charac });
}

void Sector_RemoveCharacter(CharacterInfo* charac)
{
	sectorList[charac->curPos.yPos][charac->curPos.xPos].erase(charac->characterID);
}

bool Sector_UpdateCharacter(CharacterInfo* charac)
{
	SectorPos curPos = ConvertWorldPosToSectorPos(charac->xPos, charac->yPos);

	if (curPos.yPos >= SECTOR_HEIGHT || curPos.xPos >= SECTOR_WIDTH)
	{
		DisconnectSession(charac->socket);
		return false;
	}

	if ( *((DWORD*)&curPos) == *((DWORD*)&(charac->curPos)) )
	{
		return false;
	}

	//charac->oldPos.yPos = charac->curPos.yPos;
	//charac->oldPos.xPos = charac->curPos.xPos;
	charac->oldPos = charac->curPos;
	Sector_RemoveCharacter(charac);

	charac->curPos = curPos;
	Sector_AddCharacter(charac);

	return true;
}

size_t GetSectorCharacterCnt(void)
{
	size_t cnt = 0;
	for (int y = 0; y < dfSECTOR_HEIGHT; ++y)
	{
		for (int x = 0; x < dfSECTOR_WIDTH; ++x)
		{
			cnt += sectorList[y][x].size();
		}
	}
	return cnt;
}

void GetSectorAround(SectorPos findSector, SectorAround* pSectorAround, bool includeFindSector)
{
	WORD y, x;
	WORD endYPos = findSector.yPos + 1;
	WORD endXPos = findSector.xPos + 1;
	
	if (includeFindSector == false)
	{
		int idx = 0;
		for (y = endYPos - 2; y <= endYPos; ++y)
		{
			if (y < 0 || y >= dfSECTOR_HEIGHT)
			{
				continue;
			}
			for (x = endXPos - 2; x <= endXPos; ++x)
			{
				if (x < 0 || x >= dfSECTOR_WIDTH)
				{
					continue;
				}

				if (findSector.xPos == x && findSector.yPos == y)
				{
					continue;
				}

				pSectorAround->around[idx++] = { x, y };
			}
		}
		pSectorAround->cnt = idx;

		return;
	}

	int idx = 0;
	for (y = endYPos - 2; y <= endYPos; ++y)
	{
		if (y < 0 || y >= dfSECTOR_HEIGHT)
		{
			continue;
		}
		for (x = endXPos - 2; x <= endXPos; ++x)
		{
			if (x < 0 || x >= dfSECTOR_WIDTH)
			{
				continue;
			}

			pSectorAround->around[idx++] = {x, y};
		}
	}

	pSectorAround->cnt = idx;
}

void GetUpdateSectorAround(CharacterInfo* ptrCharac, SectorAround* ptrRemoveSectors, SectorAround* ptrAddSectors)
{
	int idxOld;
	int idxCur;
	bool isFind;
	int removeSectorsCnt = 0;
	int addSectorsCnt = 0;

	SectorAround oldSectors;
	SectorAround curSectors;
	GetSectorAround(ptrCharac->oldPos, &oldSectors);
	GetSectorAround(ptrCharac->curPos, &curSectors);

	for (idxOld = 0; idxOld < oldSectors.cnt; ++idxOld)
	{
		isFind = false;
		for (idxCur = 0; idxCur < curSectors.cnt; ++idxCur)
		{
			if (*((DWORD*)(oldSectors.around + idxOld)) == *((DWORD*)(curSectors.around + idxCur)))
			{
				isFind = true;
				break;
			}
		}

		if (isFind == false)
		{
			ptrRemoveSectors->around[removeSectorsCnt++] = oldSectors.around[idxOld];
		}
	}
	ptrRemoveSectors->cnt = removeSectorsCnt;

	for (idxCur = 0; idxCur < curSectors.cnt; ++idxCur)
	{
		isFind = false;
		for (idxOld = 0; idxOld < oldSectors.cnt; ++idxOld)
		{
			if (*((DWORD*)(curSectors.around + idxCur)) == *((DWORD*)(oldSectors.around + idxOld)))
			{
				isFind = true;
				break;
			}
		}

		if (isFind == false)
		{
			ptrAddSectors->around[addSectorsCnt++] = curSectors.around[idxCur];
		}
	}
	ptrAddSectors->cnt = addSectorsCnt;
}

void CharacterSectorUpdatePacket(CharacterInfo* ptrCharac)
{
	SerializationBuffer packetBuf;

	SectorAround removeSectors;
	SectorAround addSectors;

	int idxRemoveSectors;
	int idxAddSectors;

	SectorPos removeSectorPos;
	SectorPos addSectorPos;

	CharacterInfo* addSecterCharac;

	GetUpdateSectorAround(ptrCharac, &removeSectors, &addSectors);

	/*
		removeSectors에 존재하는 캐릭터들에게,
		ptrCharac의 정보를 화면에서 지우라고 함
	*/
	MakePacketDeleteCharacter(&packetBuf, ptrCharac->characterID);
	for (idxRemoveSectors = 0; idxRemoveSectors < removeSectors.cnt; ++idxRemoveSectors)
	{
		SendUnicastSector(removeSectors.around[idxRemoveSectors], packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharac->characterID);
	}
	
	/*
		ptrCharac에개 존재하는 캐릭터들에게,
		removeSectors의 정보를 화면에서 지우라고 함
	*/
	for (idxRemoveSectors = 0; idxRemoveSectors < removeSectors.cnt; ++idxRemoveSectors)
	{
		removeSectorPos = removeSectors.around[idxRemoveSectors];
		std::map<DWORD, CharacterInfo*>& removeCharacterMap = sectorList[removeSectorPos.yPos][removeSectorPos.xPos];
		std::map<DWORD, CharacterInfo*>::iterator iter = removeCharacterMap.begin();
		for (; iter != removeCharacterMap.end(); ++iter)
		{
			if (iter->first == ptrCharac->characterID)
			{
				continue;
			}
			packetBuf.ClearBuffer();
			MakePacketDeleteCharacter(&packetBuf, iter->first);
			SendUnicast(ptrCharac->socket, packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		}
	}

	/*
		이동 중 새로 보이는 시야에 존재하는 캐릭터들에게
		이동 하는 캐릭터의 정보를 전송;
	*/
	packetBuf.ClearBuffer();

	//MakePacketCreateOtherCharacter(&packetBuf, ptrCharac->characterID
	//	, ptrCharac->stop2Dir, ptrCharac->xPos, ptrCharac->yPos, ptrCharac->hp);
	MakePacketCreateOtherCharacter(&packetBuf, ptrCharac);
	MakePacketMoveStart(&packetBuf, ptrCharac->characterID, ptrCharac->move8Dir, ptrCharac->xPos, ptrCharac->yPos);
	for (idxAddSectors = 0; idxAddSectors < addSectors.cnt; ++idxAddSectors)
	{
		SendUnicastSector(addSectors.around[idxAddSectors], packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize(), ptrCharac->characterID);
	}

	/*
		새로 보이는 시야에 있는 캐릭터 들의 정보를 내게 가져온다.
		이동 하는 캐릭터의 정보를 전송;
	*/
	for (idxAddSectors = 0; idxAddSectors < addSectors.cnt; ++idxAddSectors)
	{
		addSectorPos = addSectors.around[idxAddSectors];
		std::map<DWORD, CharacterInfo*>& addCharacterMap = sectorList[addSectorPos.yPos][addSectorPos.xPos];
		std::map<DWORD, CharacterInfo*>::iterator iter = addCharacterMap.begin();
		for (; iter != addCharacterMap.end(); ++iter)
		{
			if (iter->first == ptrCharac->characterID)
			{
				continue;
			}
			packetBuf.ClearBuffer();

			
			addSecterCharac = iter->second;
			MakePacketCreateOtherCharacter(&packetBuf, addSecterCharac);
			//MakePacketCreateOtherCharacter(&packetBuf, addSecterCharac->characterID
			//	, addSecterCharac->stop2Dir, addSecterCharac->xPos, addSecterCharac->yPos, addSecterCharac->hp);

			switch (addSecterCharac->action)
			{
			case dfPACKET_MOVE_DIR_LL:
			case dfPACKET_MOVE_DIR_LU:
			case dfPACKET_MOVE_DIR_UU:
			case dfPACKET_MOVE_DIR_RU:
			case dfPACKET_MOVE_DIR_RR:
			case dfPACKET_MOVE_DIR_RD:
			case dfPACKET_MOVE_DIR_DD:
			case dfPACKET_MOVE_DIR_LD:
				MakePacketMoveStart(&packetBuf, addSecterCharac->characterID, addSecterCharac->move8Dir, addSecterCharac->xPos, addSecterCharac->yPos);
			}

			SendUnicast(ptrCharac->socket, packetBuf.GetFrontBufferPtr(), packetBuf.GetUseSize());
		}
	}
}

void SendUnicastSector(SectorPos target, const char* buf, int size, DWORD excludeCharacterID)
{
	if (excludeCharacterID != INVALID_CHARACTER_ID)
	{
		std::map<DWORD, CharacterInfo*>::iterator iter = sectorList[target.yPos][target.xPos].begin();
		for (; iter != sectorList[target.yPos][target.xPos].end(); ++iter)
		{
			if (excludeCharacterID != iter->first)
			{
				SendUnicast(iter->second->socket, buf, size);
			}
		}
		return;
	}

	std::map<DWORD, CharacterInfo*>::iterator iter = sectorList[target.yPos][target.xPos].begin();
	for (; iter != sectorList[target.yPos][target.xPos].end(); ++iter)
	{
		SendUnicast(iter->second->socket, buf, size);
	}
}

void SendSectorAround(CharacterInfo* ptrCharac, const char* buf, int size, bool includeMe)
{
	SectorAround sendTargetSectors;
	GetSectorAround(ptrCharac->curPos, &sendTargetSectors, false);

	int idxSectors;
	for (idxSectors = 0; idxSectors < sendTargetSectors.cnt; ++idxSectors)
	{
		SendUnicastSector(sendTargetSectors.around[idxSectors], buf, size);
	}

	if (includeMe == false)
	{
		SendUnicastSector(ptrCharac->curPos, buf, size, ptrCharac->characterID);
	}
	else 
	{
		SendUnicastSector(ptrCharac->curPos, buf, size);
	}
}

void SendToMeOfSectorAroundCharacterInfo(CharacterInfo* ptrCharac)
{
	SerializationBuffer sendPacket;
	SectorAround sectors;
	int idxSectors;
	//CharacterInfo* otherCharac;

	GetSectorAround(ptrCharac->curPos, &sectors);

	for (idxSectors = 0; idxSectors < sectors.cnt; ++idxSectors)
	{
		std::map<DWORD, CharacterInfo*>& sector = sectorList[sectors.around[idxSectors].yPos][sectors.around[idxSectors].xPos];
		std::map<DWORD, CharacterInfo*>::iterator iter = sector.begin();
		for (; iter != sector.end(); ++iter)
		{
			//otherCharac = iter->second;
			MakePacketCreateOtherCharacter(&sendPacket, iter->second);
			/*MakePacketCreateOtherCharacter(&sendPacket, otherCharac->characterID
				, otherCharac->stop2Dir, otherCharac->xPos, otherCharac->yPos, otherCharac->hp);*/
			SendUnicast(ptrCharac->socket, sendPacket.GetFrontBufferPtr(), sendPacket.GetUseSize());
			sendPacket.ClearBuffer();
		}
	}
}

bool SearchCollision(int attackXRange, int attackYRange, const CharacterInfo* characterOnAttack, CharacterInfo** outCharacterIDOnDamage)
{
	CharacterInfo* charac;
	SectorPos posOnAttacker = characterOnAttack->curPos;
	int attackerXPos = characterOnAttack->xPos;
	int attackerYPos = characterOnAttack->yPos;
	//int attackerXPosPlusDir;

	int tmpDistanceX;
	int tmpDistanceY;
	int minDistanceX = attackXRange;
	int minDistanceY = attackYRange;

	int tmpX;
	int cnt;
	CharacterInfo* targetCharacter = nullptr;

	/*if (characterOnAttack->stop2Dir == dfPACKET_MOVE_DIR_LL)
	{
		attackerXPosPlusDir = attackerXPos;
	}
	else if (characterOnAttack->stop2Dir == dfPACKET_MOVE_DIR_RR)
	{
		attackerXPosPlusDir = attackerXPos * -1;
	}
	else
	{
		return false;
	}

	for (tmpX = posOnAttacker.xPos, cnt = 2; targetCharacter == nullptr && tmpX > -1 && cnt != 0; --tmpX, --cnt)
	{
		std::map<DWORD, CharacterInfo*> characterList = sectorList[posOnAttacker.yPos][tmpX];
		std::map<DWORD, CharacterInfo*>::iterator iter = characterList.begin();
		for (; iter != characterList.end(); ++iter)
		{
			charac = iter->second;
			if (charac->characterID == characterOnAttack->characterID)
			{
				continue;
			}
			tmpDistanceX = attackerXPosPlusDir - charac->xPos;
			tmpDistanceY = abs(charac->yPos - attackerYPos);

			if ((tmpDistanceX >= 0 && tmpDistanceX <= attackXRange) && tmpDistanceY <= attackYRange)
			{
				if ((tmpDistanceX < minDistanceX)
					|| (tmpDistanceX == minDistanceX && tmpDistanceY < minDistanceY))
				{
					minDistanceX = tmpDistanceX;
					minDistanceY = tmpDistanceY;
					targetCharacter = charac;
				}
			}
		}

		if (targetCharacter != nullptr)
		{
			*outCharacterIDOnDamage = targetCharacter;
			return true;
		}
	}

	int tmpYUp = ((attackerYPos - attackYRange) / dfSIX_FRAME_Y_DISTANCE);
	int tmpYDown = ((attackerYPos + attackYRange) / dfSIX_FRAME_Y_DISTANCE);
	int searchEndY = -1;

	if (tmpYUp > -1 && tmpYUp == posOnAttacker.yPos - 1)
	{
		searchEndY = tmpYUp;
	}
	else if (tmpYDown < dfSECTOR_HEIGHT&& tmpYDown == posOnAttacker.yPos + 1)
	{
		searchEndY = tmpYDown;
	}

	if (searchEndY != -1)
	{
		for (tmpX = posOnAttacker.xPos, cnt = 2; targetCharacter == nullptr && tmpX > -1 && cnt != 0; --tmpX, --cnt)
		{
			std::map<DWORD, CharacterInfo*> characterList = sectorList[searchEndY][tmpX];
			std::map<DWORD, CharacterInfo*>::iterator iter = characterList.begin();
			for (; iter != characterList.end(); ++iter)
			{
				charac = iter->second;
				if (charac->characterID == characterOnAttack->characterID)
				{
					continue;
				}
				tmpDistanceX = attackerXPos - charac->xPos;
				tmpDistanceY = abs(charac->yPos - attackerYPos);

				if ((tmpDistanceX >= 0 && tmpDistanceX <= attackXRange) && tmpDistanceY <= attackYRange)
				{
					if ((tmpDistanceX < minDistanceX)
						|| (tmpDistanceX == minDistanceX && tmpDistanceY < minDistanceY))
					{
						minDistanceX = tmpDistanceX;
						minDistanceY = tmpDistanceY;
						targetCharacter = charac;
					}
				}
			}

			if (targetCharacter != nullptr)
			{
				*outCharacterIDOnDamage = targetCharacter;
				return true;
			}
		}
	}

	return false;*/

	if (characterOnAttack->stop2Dir == dfPACKET_MOVE_DIR_LL)
	{
		for (tmpX = posOnAttacker.xPos, cnt = 2; targetCharacter == nullptr && tmpX > -1 && cnt != 0; --tmpX, --cnt)
		{
			std::map<DWORD, CharacterInfo*> characterList = sectorList[posOnAttacker.yPos][tmpX];
			std::map<DWORD, CharacterInfo*>::iterator iter = characterList.begin();
			for (; iter != characterList.end(); ++iter)
			{
				charac = iter->second;
				if (charac->characterID == characterOnAttack->characterID)
				{
					continue;
				}
				tmpDistanceX = attackerXPos - charac->xPos;
				tmpDistanceY = abs(charac->yPos - attackerYPos);

				if ((tmpDistanceX >= 0 && tmpDistanceX <= attackXRange) && tmpDistanceY <= attackYRange)
				{
					if ((tmpDistanceX < minDistanceX)
						|| (tmpDistanceX == minDistanceX && tmpDistanceY < minDistanceY))
					{
						minDistanceX = tmpDistanceX;
						minDistanceY = tmpDistanceY;
						targetCharacter = charac;
					}
				}
			}

			if (targetCharacter != nullptr)
			{
				*outCharacterIDOnDamage = targetCharacter;
				return true;
			}
		}

		int tmpYUp = ((attackerYPos - attackYRange) / dfSIX_FRAME_Y_DISTANCE);
		int tmpYDown = ((attackerYPos + attackYRange) / dfSIX_FRAME_Y_DISTANCE);
		int searchEndY = -1;

		if (tmpYUp > -1 && tmpYUp == posOnAttacker.yPos - 1)
		{
			searchEndY = tmpYUp;
		}
		else if (tmpYDown < dfSECTOR_HEIGHT && tmpYDown == posOnAttacker.yPos + 1)
		{
			searchEndY = tmpYDown;
		}

		if (searchEndY != -1)
		{
			for (tmpX = posOnAttacker.xPos, cnt = 2; targetCharacter == nullptr && tmpX > -1 && cnt != 0; --tmpX, --cnt)
			{
				std::map<DWORD, CharacterInfo*> characterList = sectorList[searchEndY][tmpX];
				std::map<DWORD, CharacterInfo*>::iterator iter = characterList.begin();
				for (; iter != characterList.end(); ++iter)
				{
					charac = iter->second;
					if (charac->characterID == characterOnAttack->characterID)
					{
						continue;
					}
					tmpDistanceX = attackerXPos - charac->xPos;
					tmpDistanceY = abs(charac->yPos - attackerYPos);

					if ((tmpDistanceX >= 0 && tmpDistanceX <= attackXRange) && tmpDistanceY <= attackYRange)
					{
						if ((tmpDistanceX < minDistanceX)
							|| (tmpDistanceX == minDistanceX && tmpDistanceY < minDistanceY))
						{
							minDistanceX = tmpDistanceX;
							minDistanceY = tmpDistanceY;
							targetCharacter = charac;
						}
					}
				}

				if (targetCharacter != nullptr)
				{
					*outCharacterIDOnDamage = targetCharacter;
					return true;
				}
			}
		}

		return false;
	}
	else if (characterOnAttack->stop2Dir == dfPACKET_MOVE_DIR_RR)
	{
		for (tmpX = posOnAttacker.xPos, cnt = 2; tmpX < dfSECTOR_WIDTH && cnt != 0; ++tmpX, --cnt)
		{
			std::map<DWORD, CharacterInfo*> characterList = sectorList[posOnAttacker.yPos][tmpX];
			std::map<DWORD, CharacterInfo*>::iterator iter = characterList.begin();
			for (; iter != characterList.end(); ++iter)
			{
				charac = iter->second;
				if (charac->characterID == characterOnAttack->characterID)
				{
					continue;
				}
				tmpDistanceX = charac->xPos - attackerXPos;
				tmpDistanceY = abs(charac->yPos - attackerYPos);

				if ((tmpDistanceX >= 0 && tmpDistanceX <= attackXRange) && tmpDistanceY <= attackYRange)
				{
					if ((tmpDistanceX < minDistanceX)
						|| (tmpDistanceX == minDistanceX && tmpDistanceY < minDistanceY))
					{
						minDistanceX = tmpDistanceX;
						minDistanceY = tmpDistanceY;
						targetCharacter = charac;
					}
				}
			}

			if (targetCharacter != nullptr)
			{
				*outCharacterIDOnDamage = targetCharacter;
				return true;
			}
		}

		int tmpYUp = ((attackerYPos - attackYRange) / dfSIX_FRAME_Y_DISTANCE);
		int tmpYDown = ((attackerYPos + attackYRange) / dfSIX_FRAME_Y_DISTANCE);
		int searchEndY = -1;

		if (tmpYUp > -1 && tmpYUp == posOnAttacker.yPos - 1)
		{
			searchEndY = tmpYUp;
		}
		else if (tmpYDown < dfSECTOR_HEIGHT&& tmpYDown == posOnAttacker.yPos + 1)
		{
			searchEndY = tmpYDown;
		}

		if (searchEndY != -1)
		{
			for (tmpX = posOnAttacker.xPos, cnt = 2; tmpX < dfSECTOR_WIDTH && cnt != 0; ++tmpX, --cnt)
			{
				std::map<DWORD, CharacterInfo*> characterList = sectorList[searchEndY][tmpX];
				std::map<DWORD, CharacterInfo*>::iterator iter = characterList.begin();
				for (; iter != characterList.end(); ++iter)
				{
					charac = iter->second;
					if (charac->characterID == characterOnAttack->characterID)
					{
						continue;
					}
					tmpDistanceX = charac->xPos - attackerXPos;
					tmpDistanceY = abs(charac->yPos - attackerYPos);

					if ((tmpDistanceX >= 0 && tmpDistanceX <= attackXRange) && tmpDistanceY <= attackYRange)
					{
						if ((tmpDistanceX < minDistanceX)
							|| (tmpDistanceX == minDistanceX && tmpDistanceY < minDistanceY))
						{
							minDistanceX = tmpDistanceX;
							minDistanceY = tmpDistanceY;
							targetCharacter = charac;
						}
					}
				}

				if (targetCharacter != nullptr)
				{
					*outCharacterIDOnDamage = targetCharacter;
					return true;
				}
			}
		}

		return false;
	}

	return false;
}
