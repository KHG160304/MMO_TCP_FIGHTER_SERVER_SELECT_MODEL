#pragma once
#ifndef __CHARACTERINFO_H__
#define __CHARACTERINFO_H__

#include "Sector.h"
#include <winsock2.h>

/*union WorldPos
{
	struct { WORD xPos, yPos; };
	DWORD dwPos;
};*/

struct Session;
struct CharacterInfo
{
	Session* ptrSession;
	SOCKET	socket;
	DWORD	characterID;
	DWORD	dwActionTick;
	WORD	xPos;
	WORD	yPos;
	//WorldPos pos;
	WORD	actionXpos;
	WORD	actionYpos;
	BYTE	stop2Dir;
	BYTE	move8Dir;
	BYTE	action;
	char	hp;

	SectorPos curPos;
	SectorPos oldPos;
};

#endif // !__CHARACTERINFO_H__
