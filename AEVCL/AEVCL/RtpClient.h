#pragma once

#include "stdafx.h"
#include <exception>

#include "AppDefinition.h"
#include "Look.h"
#include "FFDecodeEngine.h"
#include "D3DDrawEngine.h"

using namespace std;

#ifndef MEDIA_TYPE_VIDEO
#define	MEDIA_TYPE_VIDEO	0x01
#endif
#ifndef MEDIA_TYPE_AUDIO
#define	MEDIA_TYPE_AUDIO	0x02
#endif
#ifndef MEDIA_TYPE_EVENT
#define	MEDIA_TYPE_EVENT	0x04
#endif

class CRtpClient
{
private:
	HANDLE m_handle;
	RTSP_PLAY_OBJ_T* m_playobj;
	CLook m_selflook;
	int m_channel;
	AUDIO_PLAY_THREAD_OBJ* m_audio;
	CFFDecodeEngine* m_ffdecode;
	CD3DDrawEngine* m_d3ddraw;

public:
	CRtpClient();
	~CRtpClient();

public:
	bool Connect(int channelId, const char* server, const int port, const char* sim, int channel);
	bool Close();
	bool Play(HWND hWnd);
	void Enable(bool enable);
	bool Reading();

private:
	static int CALLBACK OnReceived(void* state, char* buffer, int size, int media, RTP_FRAME_INFO* frame);
	static void CALLBACK OnDecoding(object state);
	static void CALLBACK OnRendering(object state);
};

