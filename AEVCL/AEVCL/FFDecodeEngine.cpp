#include "stdafx.h"
#include "FFDecodeEngine.h"
#include "AppDefinition.h"

CFFDecodeEngine::CFFDecodeEngine(RTSP_PLAY_OBJ_T* play, AUDIO_PLAY_THREAD_OBJ* audio)
{
	CDecodeUnit::ZeroYuvFrame((this->m_pPlayObjT = play)->yuvFrame, MAX_YUV_FRAME_NUM);
	this->m_pObjAudioT = audio;
	int m_nVideoBuffer = 1024 * 1024;
	m_pVideoBuffer = new char[m_nVideoBuffer];
	if (NULL == m_pVideoBuffer)
		m_pPlayObjT->decodeThread.flag = 0x00;
	else {
		memset(m_pVideoBuffer, 0x00, m_nVideoBuffer);
		m_pPlayObjT->decodeYuvIdx = 0;
		memset(&m_pMediaFrame, 0x00, sizeof(MEDIA_FRAME_INFO));
		m_nAudioBuffer = (64000 * 3) / 2;
		m_pAudioBuffer = new unsigned char[m_nAudioBuffer + 1];
		memset(m_pAudioBuffer, 0x00, m_nAudioBuffer);
	}
}

CFFDecodeEngine::~CFFDecodeEngine()
{
	delete[] m_pAudioBuffer;
	delete[] m_pVideoBuffer;
	m_pVideoBuffer = NULL;
	m_pAudioBuffer = NULL;
}

void CFFDecodeEngine::Run()
{
	while (true)
	{
		unsigned int channelid = 0, mediatype = 0; // 定义堆栈临时变量
		if (m_pPlayObjT->decodeThread.flag == 0x03)
			break;
		if (m_pPlayObjT->initQueue == 0x00 || NULL == m_pPlayObjT->pAVQueue) {
			Sleep(10); continue;
		}
		if (SSQ_GetData(m_pPlayObjT->pAVQueue, &channelid, &mediatype, &m_pMediaFrame, m_pVideoBuffer) < 0) {
			CDecodeUnit::Delay(1, 1, 1); continue;
		}
		if (mediatype == MEDIA_TYPE_VIDEO) {
			if (!OnVideoOfDecode())
				continue;
		}
		else if (MEDIA_TYPE_AUDIO == mediatype) { // 音频 
			if (!OnAudioOfDecode())
				continue;;
		}
		else if (MEDIA_TYPE_EVENT == mediatype) {
			if (m_pMediaFrame.type == 0xF1)		// Loss Packet
				m_pPlayObjT->dwLosspacketTime = GetTickCount();
			else if (m_pMediaFrame.type == 0xFF)	// Disconnect
				m_pPlayObjT->dwDisconnectTime = GetTickCount();
		}
	}
	m_pPlayObjT->decodeThread.flag = 0x00;
}

void CFFDecodeEngine::CreateWavePlayer(unsigned int channel, unsigned int samplerate, unsigned int bitpersample)
{
	auto pAudioPlayThread = m_pObjAudioT;
	if (NULL != pAudioPlayThread) {
		WAVEFORMATEX wfx = { 0, };
		wfx.cbSize = sizeof(WAVEFORMATEX);
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec = samplerate;
		wfx.wBitsPerSample = bitpersample;
		wfx.nChannels = channel;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
		if (NULL == pAudioPlayThread->pSoundPlayer)
			pAudioPlayThread->pSoundPlayer = new CSoundPlayer();
		if (NULL != pAudioPlayThread->pSoundPlayer) {
			pAudioPlayThread->pSoundPlayer->Close();
			pAudioPlayThread->pSoundPlayer->Open(wfx);
		}
		pAudioPlayThread->audiochannels = channel;
	}
}

bool CFFDecodeEngine::OnAudioOfDecode()
{
	DECODER_OBJ* decode = CDecodeUnit::GetDecoder(m_pPlayObjT, MEDIA_TYPE_AUDIO, &m_pMediaFrame);
	if (NULL == decode)
		return FALSE;
	memset(m_pAudioBuffer, 0x00, m_nAudioBuffer);
	int pcm_data_size = 0; // 音频解码(支持g711(ulaw)和AAC)
	if (FFD_DecodeAudio(decode->ffDecoder, (char*)m_pVideoBuffer, m_pMediaFrame.length, (char *)m_pAudioBuffer, &pcm_data_size) == 0) {
		AUDIO_PLAY_THREAD_OBJ*  audio = m_pObjAudioT;
		if (audio->audiochannels == 0)
			CreateWavePlayer(decode->codec.channels, decode->codec.samplerate, 16);
		if (audio->audiochannels != 0 && NULL != audio->pSoundPlayer)
			audio->pSoundPlayer->Write((char *)m_pAudioBuffer, pcm_data_size);
	}
	return TRUE;
}

bool CFFDecodeEngine::OnVideoOfDecode()
{
	if (m_pPlayObjT->frameQueue > MAX_CACHE_FRAME) {
		SSQ_Clear(m_pPlayObjT->pAVQueue);
		m_pPlayObjT->findKeyframe = 0x01;
		m_pPlayObjT->frameQueue = m_pPlayObjT->pAVQueue->pQueHeader->videoframes;
		return FALSE;
	}
	if ((m_pPlayObjT->findKeyframe == 0x01) && (m_pMediaFrame.type == RTPC_SDK_VIDEO_FRAME_I))
		m_pPlayObjT->findKeyframe = 0x00;
	else if (m_pPlayObjT->findKeyframe == 0x01)
		return FALSE;
	DECODER_OBJ* decode = CDecodeUnit::GetDecoder(m_pPlayObjT, MEDIA_TYPE_VIDEO, &m_pMediaFrame); // 获取相应的解码器
	if (NULL == decode) {
		CDecodeUnit::Delay(1, 1, 1); return FALSE;
	}
	if (NULL == m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].pYuvBuf) {
		m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].Yuvsize = decode->yuv_size;
		m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].pYuvBuf = new char[m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].Yuvsize];
	}
	if (NULL == m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].pYuvBuf)
		return FALSE;
	while (m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].frameinfo.length > 0) { // 等待该帧显示完成
		if (m_pPlayObjT->decodeThread.flag == 0x03)
			break;
		CDecodeUnit::Delay(1, 1, 1);
	}
	if (m_pPlayObjT->decodeThread.flag == 0x03)
		return FALSE;
	if (m_pPlayObjT->decodeKeyFrameOnly == 0x01)
		if (m_pMediaFrame.type != RTPC_SDK_VIDEO_FRAME_I) {
			m_pPlayObjT->findKeyframe = 0x01; return FALSE;
		}
	EnterCriticalSection(&m_pPlayObjT->crit); // 解码
	if (0 != FFD_DecodeVideo3(decode->ffDecoder, m_pVideoBuffer, m_pMediaFrame.length, m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].pYuvBuf, m_pMediaFrame.width, m_pMediaFrame.height))
		m_pPlayObjT->findKeyframe = 0x01;
	else {
		memcpy(&m_pPlayObjT->yuvFrame[m_pPlayObjT->decodeYuvIdx].frameinfo, &m_pMediaFrame, sizeof(MEDIA_FRAME_INFO));
		m_pPlayObjT->decodeYuvIdx++;
		if (m_pPlayObjT->decodeYuvIdx >= MAX_YUV_FRAME_NUM)
			m_pPlayObjT->decodeYuvIdx = 0;
	}
	LeaveCriticalSection(&m_pPlayObjT->crit);
	return TRUE;
}
