#pragma once
#ifndef __PROCESS_CONTENTS_PACKET_H__
#define	__PROCESS_CONTENTS_PACKET_H__
#include "SerializationBuffer.h"
#include "CharacterInfo.h"

struct CommonPacketHeader
{
	BYTE	byCode;
	BYTE	bySize;
	BYTE	byType;
};

void InitContents(void);

void ProcessAcceptEvent(void* param);

void ProcessDisconnectSessionEvent(void* param);

bool CheckIfCompletedPacket(char* recvQueue, int allRecivedPacketSize, int* outPacketBodySize);

bool DispatchPacketToContents(UINT_PTR sessionKey, char* tmpRecvPacketHeader, SerializationBuffer* tmpRecvPacketBody);

CharacterInfo* CreateCharacterInfo(UINT_PTR sessionKey);

size_t GetCharacterCnt(void);

CharacterInfo* FindCharacter(SOCKET socket);

void MakePacketEcho(SerializationBuffer* packetBuf, DWORD time);

void MakePacketSyncXYPos(SerializationBuffer* packetBuf, DWORD id, WORD xPos, WORD yPos);

void MakePacketCreateMyCharacter(SerializationBuffer* packetBuf, DWORD id, BYTE stop2Dir, WORD xPos, WORD yPos, BYTE hp);

void MakePacketCreateMyCharacter(SerializationBuffer* packetBuf, CharacterInfo* charac);

void MakePacketCreateOtherCharacter(SerializationBuffer* packetBuf, DWORD id, BYTE stop2Dir, WORD xPos, WORD yPos, BYTE hp);

void MakePacketCreateOtherCharacter(SerializationBuffer* packetBuf, CharacterInfo* charac);

void MakePacketDeleteCharacter(SerializationBuffer* packetBuf, DWORD id);

void MakePacketMoveStart(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos);

void MakePacketAttack1(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos);

void MakePacketAttack2(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos);

void MakePacketAttack3(SerializationBuffer* packetBuf, DWORD id, BYTE dir, WORD xPos, WORD yPos);

void MakePacketDamage(SerializationBuffer* packetBuf, DWORD attackerID, DWORD damagedID, BYTE damageHP);

void ConvertPacketCreateMyCharaterToCreateOtherCharacter(SerializationBuffer* packetBuf);

bool ProcessPacketMoveStart(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody);

bool ProcessPacketMoveStop(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody);

bool ProcessPacketAttack1(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody);

bool ProcessPacketAttack2(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody);

bool ProcessPacketAttack3(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody);

bool ProcessPacketEcho(UINT_PTR sessionKey, SerializationBuffer* tmpRecvPacketBody);

void Update();
#endif // !PROCESS_CONTENTS_PACKET
