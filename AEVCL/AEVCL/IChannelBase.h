#pragma once 

#include "stdafx.h"

__interface __declspec(uuid("90A65085-8205-4E71-B86C-C398A25E02EB")) IChannelBase
{
	HWND HWnd(HWND value = NULL);
	BOOL Connect(const char* url);
};