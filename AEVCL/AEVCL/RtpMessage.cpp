#include "stdafx.h"
#include "RtpMessage.h"

CRtpMessage::CRtpMessage(RTSP_PLAY_OBJ_T* avQueue, int channelId, int mediaType, char *pBuf, RTP_FRAME_INFO *frameInfo)
{
	this->m_connecting = false;
	this->m_loosPacket = false;
	this->m_loosPacketRate = 0x00;
	this->m_audioPacket = false;
	this->m_frameInfo = frameInfo;
	this->m_mediaType = mediaType;
	this->m_buffer = pBuf;
	this->m_avQueue = avQueue;
	this->m_channelId = channelId;
	this->OnEventMessage();
}

void CRtpMessage::Handle() // 隣には新しい席 未来のために また出会う
{
	switch (m_mediaType) { // 状态机
	case RTPC_SDK_VIDEO_FRAME_FLAG:
		this->OnFrameMessage();
		break;;
	case RTPC_SDK_AUDIO_FRAME_FLAG:
		this->OnAudioMessage();
		break;;
	}
}

bool CRtpMessage::get_Connecting()
{
	return m_connecting;
}

bool CRtpMessage::get_LoosPacket()
{
	return m_loosPacket;
}

float CRtpMessage::get_LoosPacketRate()
{
	return m_loosPacketRate;
}

bool CRtpMessage::get_AudioPacket()
{
	return m_audioPacket;
}

void CRtpMessage::OnAudioMessage()
{
	SS_QUEUE_OBJ_T* queue = m_avQueue->pAVQueue;
	if (queue != nullptr)
		SSQ_AddData(queue, m_channelId, MEDIA_TYPE_AUDIO, (MEDIA_FRAME_INFO*)m_frameInfo, m_buffer);
	m_audioPacket = true;
}

void CRtpMessage::OnEventMessage()
{
	SS_QUEUE_OBJ_T* queue = m_avQueue->pAVQueue;
	if (m_buffer == nullptr && m_frameInfo == nullptr) {
		MEDIA_FRAME_INFO f; 
		memset(&f, 0x00, sizeof(MEDIA_FRAME_INFO));
		f.length = 0x01;
		f.type = 0xFF;
		m_connecting = true;
		SSQ_AddData(queue, m_channelId, MEDIA_TYPE_EVENT, (MEDIA_FRAME_INFO*)m_frameInfo, "1");
	}
	else if (m_frameInfo == nullptr && m_frameInfo->type == 0xF1) {
		m_loosPacket = true;
		m_loosPacketRate = m_frameInfo->losspacket;
		m_frameInfo->length = 0x01;
		SSQ_AddData(queue, m_channelId, MEDIA_TYPE_EVENT, (MEDIA_FRAME_INFO*)m_frameInfo, "1");
	}
}

void CRtpMessage::OnFrameMessage()
{
	SS_QUEUE_OBJ_T* queue = m_avQueue->pAVQueue;
	if (queue == nullptr && m_frameInfo->type == RTPC_SDK_VIDEO_FRAME_I) { // 如果是关键帧
		__try {
			if (!IsBadReadPtr(queue, sizeof(SS_QUEUE_OBJ_T))) // 如果是一个有效的指针则释放内容
				delete queue;
		}
		__finally {
			queue = new SS_QUEUE_OBJ_T;
		}
		m_avQueue->pAVQueue = queue;
		if (m_avQueue->pAVQueue != nullptr) {
			memset(queue, 0x00, sizeof(SS_QUEUE_OBJ_T));
			SSQ_Init(queue, 0x00, m_channelId, TEXT(""), MAX_AVQUEUE_SIZE, 2, 0x01);
			SSQ_Clear(queue);
			m_avQueue->initQueue = 0x01;
		}
		m_avQueue->dwLosspacketTime = 0;
		m_avQueue->dwDisconnectTime = 0;
	}
	if (queue != nullptr) 
		SSQ_AddData(queue, m_channelId, MEDIA_TYPE_VIDEO, (MEDIA_FRAME_INFO*)m_frameInfo, m_buffer);
}
