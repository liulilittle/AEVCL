#include "stdafx.h"
#include "RtpClient.h"
#include "RtpMessage.h"
#include "FFDecodeUnit.h"

CRtpClient::CRtpClient()
{
	m_audio = new AUDIO_PLAY_THREAD_OBJ;
	m_playobj = new RTSP_PLAY_OBJ_T;
	m_ffdecode = new CFFDecodeEngine(m_playobj, m_audio);
	m_d3ddraw = new CD3DDrawEngine(m_playobj);
	memset(m_audio, 0x00, sizeof(AUDIO_PLAY_THREAD_OBJ));
	memset(m_playobj, 0x00, sizeof(RTSP_PLAY_OBJ_T));
	m_channel = 0;
	m_handle = NULL;
	InitializeCriticalSection(&m_playobj->crit);
	//
	RTPC_Init(&m_handle, this);
	RTPC_OnReceived(m_handle, CRtpClient::OnReceived);
}

CRtpClient::~CRtpClient()
{
	m_selflook.Look();
	{
		TerminateThread(m_playobj->decodeThread.hThread, 0);
		TerminateThread(m_playobj->displayThread.hThread, 0);
		DeleteCriticalSection(&m_playobj->crit);
		RTPC_Deinit(&m_handle);
		delete m_playobj;
		delete m_audio;
		delete m_d3ddraw;
		delete m_ffdecode;
		m_playobj = nullptr;
		m_audio = nullptr;
		m_ffdecode = nullptr;
		m_d3ddraw = nullptr;
	}
	m_selflook.Unlook();
}

bool CRtpClient::Connect(int channelId, const char* server, const int port, const char* sim, int channel)
{
	m_selflook.Look();
	__try {
		int media = MEDIA_TYPE_VIDEO | MEDIA_TYPE_AUDIO;
		void* state = static_cast<void*>(this);
		this->m_channel = channelId;
		return RTPC_Connect(m_handle, (char*)server, (int)port, (char*)sim, channel);
	}
	__finally {
		m_selflook.Unlook();
	}
}

bool CRtpClient::Close()
{
	m_selflook.Look();
	__try {
		RTSP_PLAY_OBJ_T* play = this->m_playobj;
		if (play->decodeThread.flag != 0x00) {
			TerminateThread(play->decodeThread.hThread, 0);
			play->decodeThread.flag = 0x00;
		}
		if (play->displayThread.flag != 0x00) {
			TerminateThread(play->displayThread.hThread, 0);
			play->displayThread.flag = 0x00;
		}
		RTPC_Deinit(&m_handle);
	}
	__finally {
		m_selflook.Unlook();
	}
	return false;
}

bool CRtpClient::Play(HWND hWnd)
{
	m_selflook.Look();
	__try {
		RTSP_PLAY_OBJ_T* play = this->m_playobj;
		play->hWnd = hWnd;
		play->renderFormat = D3D_SUPPORT_FORMAT::D3D_FORMAT_YV12;
		play->frameCache = 3;

		if (play->displayThread.flag == 0x00) {
			play->displayThread.flag = 0x01;
			play->displayThread.hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CRtpClient::OnRendering, this, 0, nullptr);
		}
		if (play->decodeThread.flag == 0x00) {
			play->decodeThread.flag = 0x01;
			play->decodeThread.hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CRtpClient::OnDecoding, this, 0, nullptr);
		}
		// 音视频解码与图形渲染对资源消耗很大（它需求大量的GPU、CPU复合运算）指定最高线程级别
		return SetThreadPriority(play->decodeThread.hThread, THREAD_PRIORITY_HIGHEST) && SetThreadPriority(play->displayThread.hThread, THREAD_PRIORITY_HIGHEST);
	}
	__finally {
		m_selflook.Unlook();
	}
}

void CRtpClient::Enable(bool enable)
{
	if (RTPC_Enable(m_handle, enable)) {
		RTSP_PLAY_OBJ_T* play = this->m_playobj;
		if (enable) {
			ResumeThread(play->displayThread.hThread);
			ResumeThread(play->decodeThread.hThread);
		}
		else {
			SuspendThread(play->displayThread.hThread);
			SuspendThread(play->decodeThread.hThread);
		}
	}
}

bool CRtpClient::Reading()
{
	RTSP_PLAY_OBJ_T* play = m_playobj;
	if (play == NULL)
		return FALSE;
	return WaitForSingleObject(play->decodeThread.hThread, 0) == WAIT_TIMEOUT && m_d3ddraw->HasFrame();
}

int CALLBACK CRtpClient::OnReceived(void* state, char* buffer, int size, int media, RTP_FRAME_INFO* frame)
{
	CRtpClient* client = static_cast<CRtpClient*>(state);
	if (frame != nullptr) // 调整帧的高度差异大小
	{
		if (frame->height == 3008) 
			frame->height = 3000;
		else if (frame->height == 1088)
			frame->height = 1080;
		else if (frame->height == 544) 
			frame->height = 540;
	}
	CRtpMessage* message = new CRtpMessage(client->m_playobj, client->m_channel, media, buffer, frame);
	{
		message->Handle();
		delete message; // 回收垃圾资源
	}
	return 0;
}

void CALLBACK CRtpClient::OnDecoding(object state)
{
	CRtpClient* client = (CRtpClient*)state;
	client->m_ffdecode->Run();
}

void CALLBACK CRtpClient::OnRendering(object state)
{
	CRtpClient* client = (CRtpClient*)state;
	client->m_d3ddraw->Run();
}
