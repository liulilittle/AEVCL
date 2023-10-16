#include "vstime.h"
#include <winsock2.h>
#include <time.h>
#include "trace.h"

void __VS_Delay(unsigned int _msec)
{
	SleepEx(_msec, FALSE);
}

unsigned int _VS_GetTime(VS_TIME_T *_usagetime)
{
	SYSTEMTIME	systemTime;
	GetSystemTime(&systemTime);
	tm   tmpTm;
	tmpTm.tm_year = systemTime.wYear - 1900;
	tmpTm.tm_mon = systemTime.wMonth - 1;
	tmpTm.tm_mday = systemTime.wDay;
	tmpTm.tm_hour = systemTime.wHour;
	tmpTm.tm_min = systemTime.wMinute;
	tmpTm.tm_sec = systemTime.wSecond;
	unsigned int uiTime = (unsigned int)mktime(&tmpTm);
	if (uiTime == 0x00)	{
		return -1;
	}
	if (NULL != _usagetime) {
		_usagetime->time_sec = uiTime;
		_usagetime->msec = systemTime.wMilliseconds;
	}
	return uiTime;
}

unsigned int _VS_CalcTimeInterval(VS_TIME_T *_starttime, VS_TIME_T *_endtime)
{
	if (NULL == _starttime || NULL == _endtime) {
		return 0;
	}
	unsigned int uiUsageTime = 0x00;
	if (_starttime->time_sec == _endtime->time_sec) {
		uiUsageTime = _endtime->msec - _starttime->msec;
	}
	else {
		uiUsageTime = (_endtime->time_sec - _starttime->time_sec) * 1000 + (1000 - _starttime->msec + _endtime->msec);
	}
	return uiUsageTime;
}


void _VS_BEGIN_TIME_PERIOD(unsigned int _msec)
{
	timeBeginPeriod(_msec);
}

void _VS_END_TIME_PERIOD(unsigned int _msec)
{
	timeEndPeriod(_msec);
}
