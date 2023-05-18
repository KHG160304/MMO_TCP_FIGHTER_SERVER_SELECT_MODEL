﻿#pragma once
#ifndef __SECTOR_H__
#define	__SECTOR_H__
#include "GameContentValueSetting.h"
#include <map>

#define dfSIX_FRAME_X_DISTANCE	(dfSPEED_PLAYER_X * 10)
#define dfSIX_FRAME_Y_DISTANCE	(dfSPEED_PLAYER_Y * 10)
#define dfSECTOR_WIDTH		(dfRANGE_MOVE_RIGHT / dfSIX_FRAME_X_DISTANCE) + 1
#define dfSECTOR_HEIGHT		(dfRANGE_MOVE_BOTTOM / dfSIX_FRAME_Y_DISTANCE) + 1
#define	USER_VISIBLE_SECTOR_WIDTH	3;
#define	USER_VISIBLE_SECTOR_HEIGHT	3;

#define INVALID_CHARACTER_ID  (DWORD)(~0)

struct CharacterInfo;

typedef unsigned short WORD;

struct SectorPos                   
{
	WORD xPos;
	WORD yPos;
};

struct SectorAround
{
	int cnt;
	SectorPos around[9];
};

size_t GetSectorCharacterCnt(void);

SectorPos ConvertWorldPosToSectorPos(WORD worldXPos, WORD worldYPos);

void Sector_AddCharacter(CharacterInfo* charac);
void Sector_RemoveCharacter(CharacterInfo* charac);
bool Sector_UpdateCharacter(CharacterInfo* charac);

void SendUnicastSector(SectorPos target, const char* buf, int size, DWORD excludeCharacterID = INVALID_CHARACTER_ID);
void SendSectorAround(CharacterInfo* ptrCharac, const char* buf, int size, bool includeMe = false);

void GetSectorAround(SectorPos findPos, SectorAround* pSectorAround, bool includeFindSector = true);
void CharacterSectorUpdatePacket(CharacterInfo* ptrCharac);

void SendToMeOfSectorAroundCharacterInfo(CharacterInfo* ptrCharac);
//내정보를 주변에 뿌리고, 상대정보를 나에게 뿌린다.
void SendPacketByAcceptEvent(CharacterInfo* ptrCharac, const char* buf, int size);

bool SearchCollision(int attackXRange, int attackYRange, const CharacterInfo* characterOnAttack, CharacterInfo** outCharacterIDOnDamage);
#endif // !__SECTOR_H__
