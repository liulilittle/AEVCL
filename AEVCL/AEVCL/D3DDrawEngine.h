#pragma once

#include "stdafx.h"
#include "AppDefinition.h"
#include "FFDecodeUnit.h"

class CD3DDrawEngine
{
private:
	RTSP_PLAY_OBJ_T* m_pPlayObjT;
	MEDIA_FRAME_INFO m_pLastFrame;
	YUV_FRAME_INFO m_pLastYuvFrame;

public:
	CD3DDrawEngine(RTSP_PLAY_OBJ_T* play);

	void Run();
	bool HasFrame();

private:
	bool D3D_Prepare(int& iDispalyYuvIdx, int& iLastDisplayYuvIdx, int& iYuvFrameNum, int& iDropFrame, unsigned int& deviceLostTime);
	D3D_OSD* D3D_OBSCreate(YUV_FRAME_INFO* g_pYuvFrame, RTP_FRAME_INFO* g_pRtpFrame, int width, int height, unsigned int& deviceLostTime);
	void D3D_AVOnPaint(unsigned int& deviceLostTime);
};

