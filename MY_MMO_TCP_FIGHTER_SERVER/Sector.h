#pragma once
#ifndef __SECTOR_H__
#define	__SECTOR_H__
#include "GameContentValueSetting.h"
#include <map>
#include "CharacterInfo.h"

#define dfSIX_FRAME_X_DISTANCE	(dfSPEED_PLAYER_X * 6)
#define dfSIX_FRAME_Y_DISTANCE	(dfSPEED_PLAYER_Y * 6)
#define dfSECTOR_WIDTH		(dfRANGE_MOVE_RIGHT / dfSIX_FRAME_X_DISTANCE) + 1
#define dfSECTOR_HEIGHT		(dfRANGE_MOVE_BOTTOM / dfSIX_FRAME_Y_DISTANCE) + 1
#define	USER_VISIBLE_SECTOR_WIDTH	3;
#define	USER_VISIBLE_SECTOR_HEIGHT	3;

#define INVALID_CHARACTER_ID  (DWORD)(~0)

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

struct SectorHistory
{
	SectorPos prevSectorPos;
};

void GetAroundSector(CharacterInfo* charac, SectorAround* outSectorAround);

void GetLeftUpSector(SectorPos pos, SectorAround* outSectorAround);

void GetLeftDownSector(SectorPos pos, SectorAround* outSectorAround);

void GetRightUpSector(SectorPos pos, SectorAround* outSectorAround);

void GetRightDownSector(SectorPos pos, SectorAround* outSectorAround);

void GetLeftSector(SectorPos pos, SectorAround* outSectorAround);

void GetRightSector(SectorPos pos, SectorAround* outSectorAround);

void GetUpSector(SectorPos pos, SectorAround* outSectorAround);

void GetDownSector(SectorPos pos, SectorAround* outSectorAround);

void AddToSector(CharacterInfo* charac);

void RemoveToSector(CharacterInfo* charac);

void SendAroundSector(CharacterInfo* charac, const char* buf, int size, bool excludeMe = true);

void SendAroundSector(const SectorAround& aroundSectorList, const char* buf, int size, DWORD id = INVALID_CHARACTER_ID);

void UpdateSector(CharacterInfo* charac);
#endif // !__SECTOR_H__
