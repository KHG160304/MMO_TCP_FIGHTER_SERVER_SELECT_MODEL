#pragma once
#ifndef __PROFILER_H__
#define __PROFILER_H__
#define PROFILE
//#undef PROFILE

#ifdef PROFILE
	#define PRO_BEGIN(tagName)	BeginProfile(tagName)
	#define PRO_END(tagName)	EndProfile(tagName)
#else
	#define PRO_BEGIN(tagName)
	#define PRO_END(tagName)
#endif // PROFILE

#define _WINSOCKAPI_
#include <windows.h>

class Profile
{
public:
	Profile(const char* tag) : _tag(tag)
	{

	}
	~Profile()
	{

	}
private:
	const char* _tag;
};

struct ProfileSample
{
	long			lFlag;
	wchar_t			szName[64];

	LARGE_INTEGER	iStartTime;

	__int64			iTotalTime;
	__int64			iMinTime[2];
	__int64			iMinTimeCallCount[2];
	__int64			iMaxTime[2];
	__int64			iMaxTimeCallCount[2];
	__int64			iCallCount;
};

void BeginProfile(const WCHAR* tag);
void EndProfile(const WCHAR* tag);
bool SaveProfileSampleToText(const WCHAR* szFileName);

#endif // !__PROFILER_H__

