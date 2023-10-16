#ifndef __RTPC_H__
#define __RTPC_H__

#include "sockc.h"

#ifndef MEDIA_TYPE_VIDEO
#define	MEDIA_TYPE_VIDEO	0x01
#endif
#ifndef MEDIA_TYPE_AUDIO
#define	MEDIA_TYPE_AUDIO	0x02
#endif
#ifndef MEDIA_TYPE_EVENT
#define	MEDIA_TYPE_EVENT	0x04
#endif

/* 视频编码 */
#define RTPC_SDK_VIDEO_CODEC_H264	0x1C		/* H264  */
#define RTPC_SDK_VIDEO_CODEC_H265	0x48323635	/* 1211250229 */
#define	RTPC_SDK_VIDEO_CODEC_MJPEG	0x08		/* MJPEG */
#define	RTPC_SDK_VIDEO_CODEC_MPEG4	0x0D		/* MPEG4 */

/* 音频编码 */
#define RTPC_SDK_AUDIO_CODEC_AAC	0x15002		/* AAC */
#define RTPC_SDK_AUDIO_CODEC_G711U	0x10006		/* G711 ulaw*/
#define RTPC_SDK_AUDIO_CODEC_G711A	0x10007		/* G711 alaw*/
#define RTPC_SDK_AUDIO_CODEC_G726	0x1100B		/* G726 */


/* 音视频帧标识 */
#define RTPC_SDK_VIDEO_FRAME_FLAG	0x00000001		/* 视频帧标志 */
#define RTPC_SDK_AUDIO_FRAME_FLAG	0x00000002		/* 音频帧标志 */
#define RTPC_SDK_EVENT_FRAME_FLAG	0x00000004		/* 事件帧标志 */
#define RTPC_SDK_RTP_FRAME_FLAG		0x00000008		/* RTP帧标志 */
#define RTPC_SDK_SDP_FRAME_FLAG		0x00000010		/* SDP帧标志 */
#define RTPC_SDK_MEDIA_INFO_FLAG	0x00000020		/* 媒体类型标志*/

/* 视频关键字标识 */
#define RTPC_SDK_VIDEO_FRAME_I		0x01		/* I帧 */
#define RTPC_SDK_VIDEO_FRAME_P		0x02		/* P帧 */
#define RTPC_SDK_VIDEO_FRAME_B		0x03		/* B帧 */
#define RTPC_SDK_VIDEO_FRAME_J		0x04		/* JPEG */

#define AV_CODEC_ID_H264 28

/* 帧信息 */
typedef struct
{
	unsigned int	codec;				/* 音视频格式 */

	unsigned int	type;				/* 视频帧类型 */
	unsigned char	fps;				/* 视频帧率 */
	unsigned short	width;				/* 视频宽 */
	unsigned short  height;				/* 视频高 */

	unsigned int	reserved1;			/* 保留参数1 */
	unsigned int	reserved2;			/* 保留参数2 */

	unsigned int	sample_rate;		/* 音频采样率 */
	unsigned int	channels;			/* 音频声道数 */
	unsigned int	bits_per_sample;	/* 音频采样精度 */

	unsigned int	length;				/* 音视频帧大小 */
	unsigned int    timestamp_usec;		/* 时间戳,微妙 */
	unsigned int	timestamp_sec;		/* 时间戳 秒 */

	float			bitrate;			/* 比特率 */
	float			losspacket;			/* 丢包率 */
} RTP_FRAME_INFO;

typedef int(CALLBACK* RTPCOnReceivedProc)(void* state, char* buffer, int size, int media, RTP_FRAME_INFO* frame);

BOOL RTPC_Init(HANDLE* handle, void* state);
BOOL RTPC_Deinit(HANDLE* handle);
BOOL RTPC_OnReceived(HANDLE handle, RTPCOnReceivedProc recevied);
BOOL RTPC_Connect(HANDLE handle, char* server, int port, char* sim);
BOOL RTPC_Enable(HANDLE handle, BOOL enable);
BOOL RTPC_Connect(HANDLE handle, char* server, int port, char* sim, int channel);

#endif