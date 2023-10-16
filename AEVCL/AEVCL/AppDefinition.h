#pragma once

#include "ffdecoder\FFDecoderAPI.h"
#include "d3drender\D3DRenderAPI.h"
#include "rtpc\rtpc.h"

#include "ssqueue.h"
#include "SoundPlayer.h"
#include "vstime.h"
#include "trace.h"

#pragma comment(lib, "ffdecoder/FFDecoder.lib")
#pragma comment(lib, "d3drender/D3DRender.lib")

#define		MAX_CHANNEL_NUM		64		// ���Խ�����ʾ�����ͨ����
#define		MAX_DECODER_NUM		5		// һ�������߳���������������
#define		MAX_YUV_FRAME_NUM	3		// ���������YUV֡��
#define		MAX_CACHE_FRAME		30		// ���֡����,������ֵ��ֻ����I֡
#define		MAX_AVQUEUE_SIZE	(1024*1024)	// ���д�С

typedef struct
{
	//Video Codec
	unsigned int	vidCodec;
	int				width;
	int				height;
	int				fps;
	float			bitrate;

	//Audio Codec
	unsigned int	audCodec;
	int				samplerate;
	int				channels;
} CODEC_T;

// ��Ƶ�����߳�
typedef struct __AUDIO_PLAY_THREAD_OBJ
{
	int				channelId;		// ��ǰ����ͨ����

	unsigned int	samplerate;	// ������
	unsigned int	audiochannels;	// ����
	unsigned int	bitpersample;

	//CWaveOut		*pWaveOut;
	CSoundPlayer	*pSoundPlayer;
}AUDIO_PLAY_THREAD_OBJ;

typedef struct
{
	CODEC_T			codec;
	FFD_HANDLE		ffDecoder;
	int				yuv_size;
} DECODER_OBJ;

typedef struct 			//YUV��Ϣ
{
	RTP_FRAME_INFO	frameinfo;
	char	*pYuvBuf;
	int		Yuvsize;
} YUV_FRAME_INFO;

typedef struct __THREAD_OBJ
{
	int			flag; // 00�߳�δ������01�߳��Ѵ���
	HANDLE		hThread;
}THREAD_OBJ;

typedef struct
{
	__THREAD_OBJ		decodeThread;		//�����߳�
	__THREAD_OBJ		displayThread;		//��ʾ�߳�

	HWND			hWnd;				//��ʾ��Ƶ�Ĵ��ھ��
	int				showStatisticalInfo;//��ʾͳ����Ϣ

	int				frameCache;		//֡����(���ڵ���������),���ϲ�Ӧ������
	int				initQueue;		//��ʼ�����б�ʶ
	SS_QUEUE_OBJ_T	*pAVQueue;		//����rtsp��֡����
	int				frameQueue;		//�����е�֡��
	int				findKeyframe;	//�Ƿ���Ҫ���ҹؼ�֡��ʶ
	int				decodeYuvIdx;

	DWORD			dwLosspacketTime;	//����ʱ��
	DWORD			dwDisconnectTime;	//����ʱ��

	DECODER_OBJ		decoderObj[MAX_DECODER_NUM];
	D3D_HANDLE		d3dHandle;		//��ʾ���
	D3D_SUPPORT_FORMAT	renderFormat;	//��ʾ��ʽ
	int				ShownToScale;		//��������ʾ
	int				decodeKeyFrameOnly;	//��������ʾ�ؼ�֡

	unsigned int	rtpTimestamp;
	LARGE_INTEGER	cpuFreq;		//cpuƵ��
	_LARGE_INTEGER	lastRenderTime;	//�����ʾʱ��

	int				yuvFrameNo;		//��ǰ��ʾ��yuv֡��
	YUV_FRAME_INFO	yuvFrame[MAX_YUV_FRAME_NUM];
	CRITICAL_SECTION	crit;
	bool			resetD3d;		//�Ƿ���Ҫ�ؽ�d3dRender
	RECT			rcSrcRender;
	D3D9_LINE		d3d9Line;

	char			manuRecordingFile[MAX_PATH];
	int				manuRecording;
	void*			mp4cHandle;
	int				vidFrameNum;

	void*			pCallback;
	void			*pUserPtr;
} RTSP_PLAY_OBJ_T;

