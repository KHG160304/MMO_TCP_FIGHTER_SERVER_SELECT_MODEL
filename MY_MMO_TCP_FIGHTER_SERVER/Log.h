#pragma once
#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>
#include <strsafe.h>
#include <time.h>

#define	dfLOG_LEVEL_DEBUG	0
#define dfLOG_LEVEL_ERROR	1
#define dfLOG_LEVEL_SYSTEM	2

#define __FILENAME__	strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__

#define	_Log(logLevel, fmt, ...)										\
do {																	\
	tm timestamp;														\
	time_t localTime = time(nullptr);									\
	localtime_s(&timestamp, &localTime);								\
	if (logLevel >= gLogLevel)											\
	{																	\
		StringCchPrintf(gLogBuffer, 2048, L"%-8s [%04d-%02d-%02d %02d:%02d:%02d] [%hs:%hs:%d] " fmt						\
			, strLogLevel[logLevel]										\
			, timestamp.tm_year + 1900									\
			, timestamp.tm_mon + 1										\
			, timestamp.tm_mday											\
			, timestamp.tm_hour											\
			, timestamp.tm_min											\
			, timestamp.tm_sec											\
			, __FILENAME__												\
			, __func__													\
			, __LINE__													\
			, ##__VA_ARGS__);											\
		Log(gLogBuffer, logLevel);										\
	}																	\
} while (0)

extern int gLogLevel;
extern wchar_t gLogBuffer[2048 + 1];
extern const wchar_t* strLogLevel[3];

void Log(wchar_t* strLog, int logLevel);

#endif // !__LOG_H__
