#pragma once

#include "stdafx.h"
#include "AppDefinition.h"
#include "FFDecodeUnit.h"

class CFFDecodeEngine
{
private:
	MEDIA_FRAME_INFO m_pMediaFrame;
	RTSP_PLAY_OBJ_T* m_pPlayObjT;
	AUDIO_PLAY_THREAD_OBJ* m_pObjAudioT;
	char* m_pVideoBuffer;
	unsigned char* m_pAudioBuffer;
	int m_nAudioBuffer;

public:
	CFFDecodeEngine(RTSP_PLAY_OBJ_T* play, AUDIO_PLAY_THREAD_OBJ* audio);
	~CFFDecodeEngine();

	void Run();
private:
	bool OnAudioOfDecode();
	bool OnVideoOfDecode();
	void CreateWavePlayer(unsigned int channel, unsigned int samplerate, unsigned int bitpersample);
};

