#include "SoundPlayer.h"
#include "trace.h"
#include "vstime.h"
#include <time.h>
#include <math.h>

DWORD WINAPI __SOUND_PLAY_THREAD(LPVOID lpParam);

CSoundPlayer::CSoundPlayer(void)
{
	memset(&soundObj, 0x00, sizeof(SOUND_OBJ_T));


	soundObj.waveHdrNum = MAX_WAVEHDR_BUFFER_COUNT;
	soundObj.pWaveHdr = new WAVEHDR[soundObj.waveHdrNum];
	memset(soundObj.pWaveHdr, 0x00, sizeof(WAVEHDR) * soundObj.waveHdrNum);

	for (unsigned int i = 0; i < soundObj.waveHdrNum; i++)
	{
		soundObj.pWaveHdr[i].lpData = (LPSTR)new BYTE[MAX_AUDIO_BUFFER_LENGTH];
		soundObj.pWaveHdr[i].dwBufferLength = MAX_AUDIO_BUFFER_LENGTH;
	}

	soundObj.hNotify = CreateEvent(NULL, FALSE, FALSE, 0);

	InitializeCriticalSection(&crit);
}

CSoundPlayer::~CSoundPlayer(void)
{
	if (NULL != soundObj.hThread)
	{
		if (soundObj.flag != 0x00)	soundObj.flag = 0x03;
		while (soundObj.flag != 0x00)	{ Sleep(100); }

		CloseHandle(soundObj.hThread);
		soundObj.hThread = NULL;
	}
	if (NULL != soundObj.pWaveHdr)
	{
		for (unsigned int i = 0; i < soundObj.waveHdrNum; i++)
		{
			if (NULL != soundObj.pWaveHdr[i].lpData)
			{
				delete[]soundObj.pWaveHdr[i].lpData;
				soundObj.pWaveHdr[i].lpData = NULL;
			}
			soundObj.pWaveHdr[i].dwBufferLength = 0;
		}
		delete[]soundObj.pWaveHdr;
		soundObj.pWaveHdr = NULL;
	}
	if (soundObj.hNotify) 
		CloseHandle(soundObj.hNotify);
	soundObj.hNotify = NULL;
	DeleteCriticalSection(&crit);
}

int CSoundPlayer::Open(WAVEFORMATEX _tOutWFX)
{
	DWORD dwFlag = 0;
	HRESULT	hr = NOERROR;
	MMRESULT	mmhr;

	if (NULL != soundObj.hWaveOut)	
		return -1;

	// Open WaveOut Device for _tOutWFX
	dwFlag = WAVE_FORMAT_DIRECT;
	dwFlag |= CALLBACK_EVENT;
	mmhr = waveOutOpen(&soundObj.hWaveOut, WAVE_MAPPER, &_tOutWFX, (DWORD)soundObj.hNotify, (DWORD_PTR)0, dwFlag);
	if (MMSYSERR_NOERROR != mmhr) {
		return -4;
	}
	for (unsigned int i = 0; i < soundObj.waveHdrNum; i++)
	{
		soundObj.pWaveHdr[i].dwFlags = 0;
		soundObj.pWaveHdr[i].dwBufferLength = MAX_AUDIO_BUFFER_LENGTH;
		mmhr = waveOutPrepareHeader(soundObj.hWaveOut, &soundObj.pWaveHdr[i], sizeof(WAVEHDR));
		soundObj.pWaveHdr[i].dwBufferLength = 0;
	}
	soundObj.initWaveHdr = 0x01;

	soundObj.samplerate = _tOutWFX.nSamplesPerSec;
	soundObj.channel = _tOutWFX.nChannels;
	soundObj.bitpersec = _tOutWFX.wBitsPerSample;

	ResetData();

	if (NULL == soundObj.hThread)
	{
		soundObj.flag = 0x01;
		soundObj.pEx = this;
		soundObj.hThread = CreateThread(NULL, 0, __SOUND_PLAY_THREAD, &soundObj, 0, NULL);
		while (soundObj.flag != 0x02 && soundObj.flag != 0x00)	{ Sleep(100); }
	}
	soundObj.waveHdrWriteIdx = 0;
	return 0;
}

void CSoundPlayer::Close()
{
	if (NULL != soundObj.pWaveHdr && soundObj.initWaveHdr == 0x01)
	{
		MMRESULT mmhr;
		for (unsigned int i = 0; i < soundObj.waveHdrNum; i++)
		{
			soundObj.pWaveHdr[i].dwFlags = 0;
			mmhr = waveOutUnprepareHeader(soundObj.hWaveOut, &soundObj.pWaveHdr[i], sizeof(WAVEHDR));
			soundObj.pWaveHdr[i].dwBufferLength = 0;
		}
	}
	soundObj.initWaveHdr = 0x00;
	if (NULL != soundObj.hWaveOut)
	{
		waveOutClose(soundObj.hWaveOut);
		soundObj.hWaveOut = NULL;
	}
}

int	CSoundPlayer::Write(char *pbuf, int bufsize)//, unsigned int _timestamp)
{
	if (NULL == soundObj.pWaveHdr)	
		return -1;
	int idx = soundObj.waveHdrWriteIdx;
	if (soundObj.clearFlag == 0x01)		
		return -2; // 正在清空中...
	while ((NULL != soundObj.pWaveHdr) && (soundObj.pWaveHdr[idx].dwBufferLength > 0))
		Sleep(2);
	if (NULL == soundObj.pWaveHdr)	
		return -1;
	memcpy(soundObj.pWaveHdr[idx].lpData, pbuf, bufsize);
	soundObj.pWaveHdr[idx].dwBufferLength = bufsize;
	soundObj.waveHdrWriteIdx++;
	if (soundObj.waveHdrWriteIdx >= soundObj.waveHdrNum)
		soundObj.waveHdrWriteIdx = 0;

	//float atom = bufsize * 1024.0f / soundObj.samplerate; // 波形元素因子
	//atom /= (soundObj.channel * soundObj.bitpersec);
	//atom *= 8;
	//soundObj.sleeptimes = atom;
		
	float atom = soundObj.channel * soundObj.samplerate; // 波形元素因子
	atom /= bufsize; // soundObj.bitpersec
	soundObj.sleeptimes = atom;

	soundObj.pWaveHdr[idx].dwUser = soundObj.sleeptimes;
	soundObj.framenum++;
	return 0;
}

void CSoundPlayer::Clear()
{
	soundObj.clearFlag = 0x01;
	soundObj.waveHdrWriteIdx = 0x00;
}

void CSoundPlayer::ResetData()
{
	for (unsigned int i = 0; i < soundObj.waveHdrNum; i++) {
		memset(soundObj.pWaveHdr[i].lpData, 0x00, MAX_AUDIO_BUFFER_LENGTH);
		soundObj.pWaveHdr[i].dwBufferLength = 0;
		soundObj.framenum = 0;
	}
	soundObj.waveHdrWriteIdx = 0x00;
	soundObj.clearFlag = 0x00;
}

DWORD WINAPI __SOUND_PLAY_THREAD(LPVOID lpParam)
{
	SOUND_OBJ_T *pSoundObj = (SOUND_OBJ_T *)lpParam;
	if (NULL == pSoundObj) {
		return -1;
	}
	CSoundPlayer *pThis = (CSoundPlayer *)pSoundObj->pEx;
	pSoundObj->flag = 0x02;
	unsigned int playIdx = 0;
	MMRESULT mmhr;
	while (true)
	{
		if (pSoundObj->flag == 0x03)
			break;
		if (NULL == pSoundObj->pWaveHdr)
		{
			Sleep(100);
			continue;
		}
		if (pSoundObj->clearFlag == 0x01)
		{
			pThis->ResetData();
			playIdx = 0;
		}
		if (pSoundObj->pWaveHdr[playIdx].dwBufferLength < 1)
		{
			Sleep(1);
			continue;
		}
		EnterCriticalSection(&pThis->crit);
		mmhr = waveOutWrite(pSoundObj->hWaveOut, &pSoundObj->pWaveHdr[playIdx], sizeof(WAVEHDR));
		LeaveCriticalSection(&pThis->crit);
		if (MMSYSERR_NOERROR != mmhr)
			Sleep(10);
		int msecs = pSoundObj->pWaveHdr[playIdx].dwUser; // pSoundObj->sleeptimes;
		if (msecs > 500)	msecs = 500;
		if ((pSoundObj->framenum > ((float)pSoundObj->waveHdrNum*0.7)) && (msecs > 0))
			msecs = (int)((float)msecs * (float)0.7f);
		if (msecs > 0)
			Sleep(msecs);
		pSoundObj->pWaveHdr[playIdx].dwBufferLength = 0;
		playIdx++;
		if (playIdx >= pSoundObj->waveHdrNum)
			playIdx = 0;
		pSoundObj->framenum--;
	}
	pSoundObj->flag = 0x00;
	return 0;
}
