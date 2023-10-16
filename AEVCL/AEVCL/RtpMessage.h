#pragma once

#include "stdafx.h"
#include "ssqueue.h"
#include "AppDefinition.h"

#ifndef MAX_AVQUEUE_SIZE
	#define	MAX_AVQUEUE_SIZE (1024 * 1024)	// ���д�С
#endif

class __declspec(uuid("032207E4-179D-40DA-8BAC-8B2EDC28BAAE")) CRtpMessage
{
private:
	RTP_FRAME_INFO* m_frameInfo;
 	char* m_buffer;
	int m_mediaType;
	bool m_connecting;
	bool m_loosPacket;
	float m_loosPacketRate;
	RTSP_PLAY_OBJ_T* m_avQueue;
	int m_channelId;
	bool m_audioPacket;

public:
	CRtpMessage(RTSP_PLAY_OBJ_T* avQueue, int channelId, int mediaType, char *pBuf, RTP_FRAME_INFO *frameInfo);

public:
	bool __declspec(property(get = get_Connecting)) Connecting; // ������
	bool __declspec(property(get = get_LoosPacket)) LossPacket; // ����
	float __declspec(property(get = get_LoosPacketRate)) LossPacketRate; // ������
	bool __declspec(property(get = get_AudioPacket)) AudioPacket; // ��Ƶ��

public:
	void Handle();

private:
	bool get_Connecting();
	bool get_LoosPacket();
	float get_LoosPacketRate();
	bool get_AudioPacket();

private:
	void OnAudioMessage();
	void OnEventMessage();
	void OnFrameMessage();
};

