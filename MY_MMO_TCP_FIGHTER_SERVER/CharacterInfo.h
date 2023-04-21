#pragma once
#ifndef __CHARACTERINFO_H__
#define __CHARACTERINFO_H__

#include <winsock2.h>

struct CharacterInfo
{
	SOCKET	socket;
	DWORD	characterID;
	DWORD	dwActionTick;
	WORD	xPos;
	WORD	yPos;
	WORD	actionXpos;
	WORD	actionYpos;
	BYTE	stop2Dir;
	BYTE	move8Dir;
	BYTE	action;
	char	hp;
};

#endif // !__CHARACTERINFO_H__
