#include "Log.h"
#include "Network.h"
#include "SerializationBuffer.h"
#include <WS2tcpip.h>
#include <map>
#include "Profiler.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm")

#define MAX_CON_SESSION	6000
#define	SERVERPORT		11601
#define USER_FD_SETSIZE	FD_SETSIZE - 1 

//-----------------------------------------------------------------
// 메시지 검증값
//-----------------------------------------------------------------
#define dfPACKET_CODE		0x89

struct CommonPacketHeader
{
	BYTE	byCode;
	BYTE	bySize;
	BYTE	byType;
};

static SOCKET gListenSocket;
static SOCKADDR_IN gServerSockAddr;
static std::map<SOCKET, Session*> sessionList;

static int packetHeaderSize = -1;
static char* bufPacketHeader;

static void (*ProcessContentsAcceptEvent)(void* param) = nullptr;
static void (*ProcessContentsDisconnectSessionEvent)(void* param) = nullptr;
static bool (*CheckIfCompletedPacketHandler)(char* bufPacketHeader, int allRecivedPacketSize, int* outPacketBodySize) = nullptr;
static bool (*DispatchPacketToContentsHandler)(UINT_PTR sessionKey, char* tmpRecvPacketHeader, SerializationBuffer* recvPacket) = nullptr;

bool InitNetwork(void)
{
	timeBeginPeriod(1);

	if (ProcessContentsAcceptEvent == nullptr || ProcessContentsDisconnectSessionEvent == nullptr)
	{
		_Log(dfLOG_LEVEL_ERROR, "컨텐츠의 ACCEPT 또는 DISCONNECT EVENT nullptr 초기화");
		return false;
	}

	if (packetHeaderSize == -1)
	{
		_Log(dfLOG_LEVEL_ERROR, "컨텐츠 패킷의 공통헤더 크기 설정 초기화 실패");
		return false;
	}

	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		_Log(dfLOG_LEVEL_ERROR, "윈속 초기화 실패: %d", WSAGetLastError());
		return false;
	}

	gListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (gListenSocket == INVALID_SOCKET)
	{
		_Log(dfLOG_LEVEL_ERROR, "리슨 소켓 생성 실패: %d", WSAGetLastError());
		return false;
	}

	gServerSockAddr.sin_family = AF_INET;
	gServerSockAddr.sin_port = htons(SERVERPORT);
	gServerSockAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(gListenSocket, (SOCKADDR*)&gServerSockAddr, sizeof(gServerSockAddr)) == SOCKET_ERROR)
	{
		_Log(dfLOG_LEVEL_ERROR, "리슨 소켓 바인딩 실패: %d", WSAGetLastError());
		return false;
	}

	if (listen(gListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		_Log(dfLOG_LEVEL_ERROR, "리슨 소켓 리스닝 실패: %d", WSAGetLastError());
		return false;
	}

	linger optLinger;
	optLinger.l_onoff = 1;
	optLinger.l_linger = 0;
	if (setsockopt(gListenSocket, SOL_SOCKET, SO_LINGER, (char*)&optLinger, sizeof(optLinger)) != 0)
	{
		_Log(dfLOG_LEVEL_ERROR, "리슨 소켓 LINGER OPTION 실패: %d", WSAGetLastError());
		return false;
	}

	WCHAR wstrServerIp[16];
	_Log(dfLOG_LEVEL_SYSTEM, "서버 접속정보: %s/%d", InetNtop(AF_INET, &gServerSockAddr.sin_addr, wstrServerIp, 16), SERVERPORT);
	_Log(dfLOG_LEVEL_SYSTEM, "서버 초기화 및 시작 성공");
	return true;
}

void ProcessNetworkIOEvent(void)
{
	Session* tmpPtrSesion;
	SOCKET socketTable[FD_SETSIZE];
	int socketCnt = 1;

	FD_SET rset;
	FD_SET wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	
	socketTable[0] = gListenSocket;
	FD_SET(gListenSocket, &rset);

	std::map<SOCKET, Session*>::iterator iter = sessionList.begin();
	for (; iter != sessionList.end();)
	{
		tmpPtrSesion = iter->second;
		socketTable[socketCnt] = tmpPtrSesion->socket;
		FD_SET(tmpPtrSesion->socket, &rset);
		if (tmpPtrSesion->sendQueue.GetUseSize() > 0)
		{
			FD_SET(tmpPtrSesion->socket, &wset);
		}

		++iter;
		++socketCnt;

		if (socketCnt == FD_SETSIZE)
		{
			PRO_BEGIN(L"ProcessSelectIOEvent");
			ProcessSelectIOEvent(socketTable, &rset, &wset);
			PRO_END(L"ProcessSelectIOEvent");
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			
			FD_SET(gListenSocket, &rset);
			socketTable[0] = gListenSocket;
			socketCnt = 1;
		}
	}
	PRO_BEGIN(L"ProcessSelectIOEvent2");
	ProcessSelectIOEvent(socketTable, &rset, &wset);
	PRO_END(L"ProcessSelectIOEvent2");
}


void ProcessSelectIOEvent(const SOCKET* socketTable, FD_SET* rset, FD_SET* wset)
{
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	int selectCnt = select(0, rset, wset, nullptr, &tv);
	if (selectCnt > 0)
	{
		if (FD_ISSET(socketTable[0], rset))
		{
			selectCnt -= 1;
			PRO_BEGIN(L"ProcessAcceptNetworkEvent");
			ProcessAcceptNetworkEvent();
			PRO_END(L"ProcessAcceptNetworkEvent");
		}

		int isNetworkIOPossible = true;
		for (int i = 1; selectCnt > 0 && i < FD_SETSIZE; ++i)
		{
			if (FD_ISSET(socketTable[i], wset))
			{
				selectCnt -= 1;
				//if (isNetworkIOPossible)
				//{
				PRO_BEGIN(L"ProcessSendNetworkEvent");
				isNetworkIOPossible = ProcessSendNetworkEvent(socketTable[i]);
				PRO_END(L"ProcessSendNetworkEvent");
				//}
			}

			if (FD_ISSET(socketTable[i], rset))
			{
				selectCnt -= 1;
				if (isNetworkIOPossible)
				{
					PRO_BEGIN(L"ProcessRecvNetworkEvent");
					ProcessRecvNetworkEvent(socketTable[i]);
					PRO_END(L"ProcessRecvNetworkEvent");
				}
			}
		}
	}
}

void ProcessAcceptNetworkEvent(void)
{
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	SOCKET clientSock = accept(gListenSocket, (SOCKADDR*)&clientAddr, &addrlen);
	if (clientSock == INVALID_SOCKET)
	{
		_Log(dfLOG_LEVEL_ERROR, "클라이언트 소켓 생성 실패: %d", WSAGetLastError());
		return;
	}

	if (sessionList.size() > MAX_CON_SESSION)
	{
		_Log(dfLOG_LEVEL_ERROR, "최대 접속가능 세션 초과(%d)", MAX_CON_SESSION);
		return;
	}

	//WCHAR wstrClientIp[16];
	Session* newSession = CreateSession(clientSock, &clientAddr);
	//_Log(dfLOG_LEVEL_DEBUG, "[%s/%d] 클라이언트 접속"
	//	, InetNtop(AF_INET, &clientAddr.sin_addr, wstrClientIp, 16)
	//	, ntohs(clientAddr.sin_port));

	ProcessContentsAcceptEvent((void*)clientSock);
}

bool ProcessRecvNetworkEvent(SOCKET socket)
{
	//Session* ptrSession = FindSession(socket);
	Session* ptrSession = sessionList[socket];
	RingBuffer* ptrRecvQueue = &ptrSession->recvQueue;
	int receivedSize = recv(socket, ptrRecvQueue->GetRearBufferPtr(), ptrRecvQueue->GetDirectEnqueueSize(), 0);
	if (receivedSize == SOCKET_ERROR)
	{
		//_Log(dfLOG_LEVEL_DEBUG, "[소켓ID: %lld] RECV ERROR 발생(CODE: %d)", socket, WSAGetLastError());
		DisconnectSession(socket);
		return false;
	}
	else if (receivedSize == 0)
	{
		//_Log(dfLOG_LEVEL_DEBUG, "[소켓ID: %lld] RECV 클라이언트 FIN 수신확인", socket);
		DisconnectSession(socket);
		return false;
	}
	ptrSession->lastRecvTime = timeGetTime();
	ptrRecvQueue->MoveRear(receivedSize);

	CommonPacketHeader tmpRecvPacketHeader;
	SerializationBuffer tmpRecvPacketBody;
	int queueUseSize;
	for (;;)
	{
		queueUseSize = ptrRecvQueue->GetUseSize();
		if (queueUseSize < sizeof(CommonPacketHeader))
		{
			break;
		}

		ptrRecvQueue->Peek((char*)&tmpRecvPacketHeader, sizeof(CommonPacketHeader));
		
		if (tmpRecvPacketHeader.byCode != dfPACKET_CODE)
		{
			break;
		}

		if (tmpRecvPacketHeader.bySize + sizeof(CommonPacketHeader) > queueUseSize)
		{
			break;
		}
		ptrRecvQueue->MoveFront(sizeof(CommonPacketHeader));
		tmpRecvPacketBody.MoveRear(ptrRecvQueue->Dequeue(tmpRecvPacketBody.GetRearBufferPtr(), tmpRecvPacketHeader.bySize));
		DispatchPacketToContentsHandler(socket, (char*)&tmpRecvPacketHeader, &tmpRecvPacketBody);
		tmpRecvPacketBody.ClearBuffer();
	}

	return true;
}

bool ProcessSendNetworkEvent(SOCKET socket)
{
	//Session* ptrSession1 = FindSession(socket);
	//Session* ptrSession = sessionList[socket];
	RingBuffer* ptrSendQueue = &sessionList[socket]->sendQueue;
	int sendSize;

	while (ptrSendQueue->GetUseSize() > 0)
	{
		sendSize = send(socket, ptrSendQueue->GetFrontBufferPtr(), ptrSendQueue->GetDirectDequeueSize(), 0);
		if (sendSize == SOCKET_ERROR)
		{
			_Log(dfLOG_LEVEL_ERROR, "[소켓ID: %lld] SEND ERROR 발생(CODE: %d)", socket, WSAGetLastError());
			DisconnectSession(socket);
			return false;
		}
		ptrSendQueue->MoveFront(sendSize);
		//_Log(dfLOG_LEVEL_ERROR, "[소켓ID: %lld] SEND BYTE(%d)", socket, ptrSendQueue->MoveFront(sendSize));
	}
	return true;
}

size_t GetSessionCnt()
{
	return sessionList.size();
}

Session* FindSession(SOCKET socket)
{
	return sessionList[socket];
}

Session* CreateSession(SOCKET socket, const SOCKADDR_IN* clientAddr)
{
	Session* newSession = new Session(socket, clientAddr);
	sessionList.insert({ socket, newSession });
	return newSession;
}

bool DisconnectSession(SOCKET socket)
{
	//WCHAR	wstrClientIp[16];
	Session* disconnectSession = sessionList[socket];
	if (disconnectSession != nullptr)
	{
		//_Log(dfLOG_LEVEL_DEBUG, "[%s/%d] 클라이언트 접속 종료, 소켓ID=%lld"
		//	, InetNtop(AF_INET, &disconnectSession->addrIn.sin_addr, wstrClientIp, 16), ntohs(disconnectSession->addrIn.sin_port), socket);
		//disconnectSession->sendQueue.ClearBuffer();
		ProcessContentsDisconnectSessionEvent((void*)socket);
		delete disconnectSession;
	}

	closesocket(socket);
	sessionList.erase(socket);
	
	return true;
}

bool DisconnectSession(Session* disconnectSession)
{
	//WCHAR	wstrClientIp[16];
	if (disconnectSession == nullptr)
	{
		return false;
	}
	//_Log(dfLOG_LEVEL_DEBUG, "[%s/%d] 클라이언트 접속 종료, 소켓ID=%lld"
	//	, InetNtop(AF_INET, &disconnectSession->addrIn.sin_addr, wstrClientIp, 16), ntohs(disconnectSession->addrIn.sin_port), disconnectSession->socket);
	//disconnectSession->sendQueue.ClearBuffer();
	closesocket(disconnectSession->socket);
	sessionList.erase(disconnectSession->socket);
	ProcessContentsDisconnectSessionEvent((void*)disconnectSession->socket);

	return true;
}

bool SendUnicast(SOCKET socket, const char* buf, int size)
{
bool SendUnicast(Session* ptrSession, const char* buf, int size)
{
	return ptrSession->sendQueue.Enqueue(buf, size) > 0;
}

void SendBroadcast(const char* buf, int size, SOCKET excludeSessionId)
{
	if (excludeSessionId == INVALID_SOCKET)
	{
		std::map<SOCKET, Session*>::iterator iter = sessionList.begin();
		for (; iter != sessionList.end(); ++iter)
		{
			(iter->second)->sendQueue.Enqueue(buf, size);
		}
		return;
	}

	std::map<SOCKET, Session*>::iterator iter = sessionList.begin();
	for (; iter != sessionList.end(); ++iter)
	{
		if ((iter->first) != excludeSessionId)
		{
			(iter->second)->sendQueue.Enqueue(buf, size);
		}
	}
	return;
}

void SendMulticast(SOCKET socket[], int socketCnt, const char* buf, int size)
{
	for (int i = 0; i < socketCnt; ++i)
	{
		FindSession(socket[i])->sendQueue.Enqueue(buf, size);
	}
}

void SetPacketHeaderSize(int size)
{
	packetHeaderSize = size;
	bufPacketHeader = new char[size];
}

void SetProcessContentsAcceptEvent(void (*pProcessContentsAcceptEvent)(void* param))
{
	ProcessContentsAcceptEvent = pProcessContentsAcceptEvent;
}

void SetProcessContentsDisconnectSessionEvent(void (*pProcessContentsDisconnectSessionEvent)(void* param))
{
	ProcessContentsDisconnectSessionEvent = pProcessContentsDisconnectSessionEvent;
}

void SetDispatchPacketToContentsHandler(bool (*paramDispatchPacketToContents)(UINT_PTR sessionKey, char* tmpRecvPacketHeader, SerializationBuffer* recvPacket))
{
	DispatchPacketToContentsHandler = paramDispatchPacketToContents;
}

void ShutdownServer(void)
{
	timeEndPeriod(1);
	closesocket(gListenSocket);

	std::map<SOCKET, Session*>::iterator iter = sessionList.begin();
	for (; iter != sessionList.end();)
	{
		closesocket(iter->first);
		iter = sessionList.erase(iter);
	}
}