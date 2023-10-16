#include "stdafx.h"
#include "FFDecodeUnit.h"
#include "vstime.h"

void CDecodeUnit::ZeroYuvFrame(YUV_FRAME_INFO* ptr, int len)
{
	if (ptr != nullptr) {
		for (int i = 0; i < len; i++) {
			memset(&ptr[i].frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
		}
	}
}

void CDecodeUnit::Delay(int begin, int delay, int end)
{
	_VS_BEGIN_TIME_PERIOD(begin);
	__VS_Delay(delay);
	_VS_END_TIME_PERIOD(end);
}
