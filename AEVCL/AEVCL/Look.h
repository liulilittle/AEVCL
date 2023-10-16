#pragma once

#include "stdafx.h"
#include <windows.h>

class CLook
{
private:
	CRITICAL_SECTION m_cs;

public:
	CLook()
	{
		InitializeCriticalSection(&m_cs);
	}
	~CLook()
	{
		DeleteCriticalSection(&m_cs);
	}

public:
	void Look()
	{
		EnterCriticalSection(&m_cs);
	}

	void Unlook()
	{
		LeaveCriticalSection(&m_cs);
	}
};

