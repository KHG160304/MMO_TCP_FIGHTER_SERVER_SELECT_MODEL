
#include <stdio.h>
#include <time.h>
#include "Profiler.h"

static int sampleCnt = 0;
static ProfileSample gProfileSample[64] = { 0 };
static LARGE_INTEGER start;
static LARGE_INTEGER end;

void BeginProfile(const WCHAR* tag)
{
	//-------------------------------------------------------------------
	//�������ϸ��Ϸ��� ������ ������ �˸��� �Լ��̴�.
	//���ڷ� ���� �±׸��� ������ gProfileSample �迭�� ��ȸ�ϸ鼭
	//���ڷ� ���� �±װ� ��ϵǾ��ִ��� Ȯ���Ѵ�.
	//�±׸��� �������� ������, gProfileSample�� ���ڸ��� 
	//�ش� �±׸� ����Ѵ�.
	//�׸��� QueryPerformanceCounter�� ȣ���Ͽ�
	//����(�������� ����)�� Ÿ�ӽ����� ���� ���Ѵ�.
	//-------------------------------------------------------------------
	int i;
	for (i = 0; i < sampleCnt; ++i)
	{
		if (gProfileSample[i].lFlag == 1
			&& wcscmp(tag, gProfileSample[i].szName) == 0)
		{
			break;
		}
	}
	if (sampleCnt < 64 && i == sampleCnt)
	{
		gProfileSample[sampleCnt].lFlag = 1;
		wcscpy_s(gProfileSample[sampleCnt].szName, tag);
		sampleCnt += 1;
	}
	QueryPerformanceCounter(&start);
}

void EndProfile(const WCHAR* tag)
{
	//-------------------------------------------------------------------
	//�������ϸ��Ϸ��� ������ ���Ḧ �˸��� �Լ��̴�.
	//���ڷ� ���� �±׸��� ������ gProfileSample �迭�� ��ȸ�Ѵ�.
	//�±׸��� ��ϵ� ��ġ�� ã�� ������
	//�������ϸ��Ϸ��� ������ ���� Ƚ���� ī��Ʈ�ϰ�
	//������ �ð����� �� �ð����� ������Ų��.
	//���� ����������, ���� ���� �������� �����Ѵ�.
	//-------------------------------------------------------------------
	QueryPerformanceCounter(&end);
	int i;
	LONGLONG intervalTime;
	for (i = 0; i < sampleCnt; ++i)
	{
		if (gProfileSample[i].lFlag == 1
			&& wcscmp(tag, gProfileSample[i].szName) == 0)
		{
			intervalTime = (end.QuadPart - start.QuadPart);
			if (gProfileSample[i].iCallCount == 0)
			{
				gProfileSample[i].iMaxTime[0] = intervalTime;
				gProfileSample[i].iMinTime[1] = intervalTime;
			}
			else if (gProfileSample[i].iCallCount == 1)
			{
				gProfileSample[i].iMaxTime[1] = intervalTime;
				gProfileSample[i].iMinTime[0] = intervalTime;
			}
			else
			{
				if (gProfileSample[i].iMaxTime[0] < intervalTime)
				{
					gProfileSample[i].iMaxTime[0] = intervalTime;
				}
				else if (gProfileSample[i].iMaxTime[1] < intervalTime)
				{
					gProfileSample[i].iMaxTime[1] = intervalTime;
				}
				else if (gProfileSample[i].iMinTime[0] > intervalTime)
				{
					gProfileSample[i].iMinTime[0] = intervalTime;
				}
				else if (gProfileSample[i].iMinTime[1] > intervalTime)
				{
					gProfileSample[i].iMinTime[1] = intervalTime;
				}
			}

			gProfileSample[i].iCallCount += 1;
			gProfileSample[i].iTotalTime += intervalTime;
		}
	}
}

bool SaveProfileSampleToText(const WCHAR* szFileName)
{
	//-----------------------------------------------
	//gProfileSample �迭�� ����� ���� ������
	//�ؽ�Ʈ ���Ϸ� �����Ѵ�.
	//�±׸�, ��� �����ð�, �ּ� �����ð�, �ִ� �����ð�, ����Ƚ��
	//�� ǥ�� ���� �����Ѵ�.
	//-----------------------------------------------
	tm TM;
	time_t currentTime;
	time(&currentTime);
	localtime_s(&TM, &currentTime);

	FILE* file = nullptr;
	WCHAR  szTime[15];
	swprintf_s(szTime, 15, L"%04d%02d%02d%02d%02d%02d"
		, TM.tm_year + 1900
		, TM.tm_mon + 1
		, TM.tm_mday
		, TM.tm_hour
		, TM.tm_min
		, TM.tm_sec);
	size_t saveFileNameLen = wcslen(szFileName) + wcslen(szTime) + wcslen(L".txt") + 1;
	WCHAR* _szFileName = (WCHAR*)malloc(sizeof(WCHAR) * saveFileNameLen);
	if (_szFileName == nullptr)
	{
		return false;
	}
	memset(_szFileName, 0, sizeof(WCHAR) * saveFileNameLen);
	wcscat_s(_szFileName, saveFileNameLen, szFileName);
	wcscat_s(_szFileName, saveFileNameLen, szTime);
	wcscat_s(_szFileName, saveFileNameLen, L".txt");
	errno_t err = _wfopen_s(&file, _szFileName, L"w");
	if (err != 0 || file == nullptr)
	{
		free(_szFileName);
		return false;
	}

	static LARGE_INTEGER lpFrequency = { 0 };
	if (lpFrequency.QuadPart == 0)
	{
		QueryPerformanceFrequency(&lpFrequency);
	}

	fprintf_s(file, "%-35s |%15s |%15s |%15s |%13s |\n", "Name", "Average", "Min", "Max", "Call");
	fprintf_s(file, "-------------------------------------------------------------------------------------------------------\n");

	for (int i = 0; i < sampleCnt; ++i)
	{
		fprintf_s(file, "%-35ws |%13.4Lf��s |%13.4Lf��s |%13.4Lf��s |%13lld |\n"
			, gProfileSample[i].szName
			, ((long double)((gProfileSample[i].iTotalTime - gProfileSample[i].iMaxTime[0] - gProfileSample[i].iMaxTime[1] - gProfileSample[i].iMinTime[0] - gProfileSample[i].iMinTime[1]) * 1000000)
				/ (long double)lpFrequency.QuadPart) / (long double)gProfileSample[i].iCallCount
			, (long double)gProfileSample[i].iMinTime[0] * 1000000 / (long double)lpFrequency.QuadPart
			, (long double)gProfileSample[i].iMaxTime[0] * 1000000 / (long double)lpFrequency.QuadPart
			, gProfileSample[i].iCallCount);
	}
	free(_szFileName);
	fclose(file);
	return true;
}