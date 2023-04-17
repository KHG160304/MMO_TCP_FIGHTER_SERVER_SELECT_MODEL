#pragma once
#ifndef __NETWORK_H__
#define __NETWORK_H__
#include <winsock2.h>
#include "RingBuffer.h"
#include "SerializationBuffer.h"

struct Session
{
	SOCKET socket;
	const SOCKADDR_IN addrIn;
	RingBuffer	recvQueue;
	RingBuffer	sendQueue;
	DWORD lastRecvTime;

	Session(SOCKET socket, const SOCKADDR_IN* addrIn)
		: socket(socket)
		, addrIn(*addrIn)
		, recvQueue(3900)
		, sendQueue(3900)
		, lastRecvTime(timeGetTime())
	{}
};

bool InitNetwork(void);

void ProcessNetworkIOEvent(void);

void ProcessSelectIOEvent(const SOCKET* socketTable, FD_SET* rset, FD_SET* wset);

void ProcessAcceptNetworkEvent(void);

bool ProcessRecvNetworkEvent(SOCKET session);

void ProcessSendNetworkEvent(SOCKET session);

bool SendUnicast(SOCKET socket, const char* buf, int size);

void SendBroadcast(const char* buf, int size, SOCKET excludeSessionId = INVALID_SOCKET);

void SendMulticast(SOCKET socket[], int socketCnt, const char* buf, int size);

size_t GetSessionCnt();

Session* FindSession(SOCKET socket);

Session* CreateSession(SOCKET socket, const SOCKADDR_IN* clientAddr);

bool DisconnectSession(SOCKET socket);

void ShutdownServer(void);


void SetProcessContentsAcceptEvent(void (*ProcessContentsAcceptEvent)(void* param));
void SetProcessContentsDisconnectSessionEvent(void (*ProcessContentsDisconnectSessionEvent)(void* param));

void SetPacketHeaderSize(int size);
void SetCheckIfCompltedPacketHandler(bool (*paramCheckIfCompletedPacket)(char* bufPacketHeader, int allRecivedPacketSize, int* outPacketBodySize));
void SetDispatchPacketToContentsHandler(bool (*DispatchPacketToContents)(UINT_PTR sessionKey, char* tmpRecvPacketHeader, SerializationBuffer* recvPacket));


#endif // !__NETWORK_H__
