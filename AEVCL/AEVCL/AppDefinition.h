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

#define		MAX_CHANNEL_NUM		64		// 可以解码显示的最大通道数
#define		MAX_DECODER_NUM		5		// 一个播放线程中最大解码器个数
#define		MAX_YUV_FRAME_NUM	3		// 解码后的最大YUV帧数
#define		MAX_CACHE_FRAME		30		// 最大帧缓存,超过该值将只播放I帧
#define		MAX_AVQUEUE_SIZE	(1024*1024)	// 队列大小

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

// 音频播放线程
typedef struct __AUDIO_PLAY_THREAD_OBJ
{
	int				channelId;		// 当前播放通道号

	unsigned int	samplerate;	// 采样率
	unsigned int	audiochannels;	// 声道
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

typedef struct 			//YUV信息
{
	RTP_FRAME_INFO	frameinfo;
	char	*pYuvBuf;
	int		Yuvsize;
} YUV_FRAME_INFO;

typedef struct __THREAD_OBJ
{
	int			flag; // 00线程未创建、01线程已创建
	HANDLE		hThread;
}THREAD_OBJ;

typedef struct
{
	__THREAD_OBJ		decodeThread;		//解码线程
	__THREAD_OBJ		displayThread;		//显示线程

	HWND			hWnd;				//显示视频的窗口句柄
	int				showStatisticalInfo;//显示统计信息

	int				frameCache;		//帧缓存(用于调整流畅度),由上层应用设置
	int				initQueue;		//初始化队列标识
	SS_QUEUE_OBJ_T	*pAVQueue;		//接收rtsp的帧队列
	int				frameQueue;		//队列中的帧数
	int				findKeyframe;	//是否需要查找关键帧标识
	int				decodeYuvIdx;

	DWORD			dwLosspacketTime;	//丢包时间
	DWORD			dwDisconnectTime;	//断线时间

	DECODER_OBJ		decoderObj[MAX_DECODER_NUM];
	D3D_HANDLE		d3dHandle;		//显示句柄
	D3D_SUPPORT_FORMAT	renderFormat;	//显示格式
	int				ShownToScale;		//按比例显示
	int				decodeKeyFrameOnly;	//仅解码显示关键帧

	unsigned int	rtpTimestamp;
	LARGE_INTEGER	cpuFreq;		//cpu频率
	_LARGE_INTEGER	lastRenderTime;	//最后显示时间

	int				yuvFrameNo;		//当前显示的yuv帧号
	YUV_FRAME_INFO	yuvFrame[MAX_YUV_FRAME_NUM];
	CRITICAL_SECTION	crit;
	bool			resetD3d;		//是否需要重建d3dRender
	RECT			rcSrcRender;
	D3D9_LINE		d3d9Line;

	char			manuRecordingFile[MAX_PATH];
	int				manuRecording;
	void*			mp4cHandle;
	int				vidFrameNum;

	void*			pCallback;
	void			*pUserPtr;
} RTSP_PLAY_OBJ_T;

