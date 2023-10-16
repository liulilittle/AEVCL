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

/* ��Ƶ���� */
#define RTPC_SDK_VIDEO_CODEC_H264	0x1C		/* H264  */
#define RTPC_SDK_VIDEO_CODEC_H265	0x48323635	/* 1211250229 */
#define	RTPC_SDK_VIDEO_CODEC_MJPEG	0x08		/* MJPEG */
#define	RTPC_SDK_VIDEO_CODEC_MPEG4	0x0D		/* MPEG4 */

/* ��Ƶ���� */
#define RTPC_SDK_AUDIO_CODEC_AAC	0x15002		/* AAC */
#define RTPC_SDK_AUDIO_CODEC_G711U	0x10006		/* G711 ulaw*/
#define RTPC_SDK_AUDIO_CODEC_G711A	0x10007		/* G711 alaw*/
#define RTPC_SDK_AUDIO_CODEC_G726	0x1100B		/* G726 */


/* ����Ƶ֡��ʶ */
#define RTPC_SDK_VIDEO_FRAME_FLAG	0x00000001		/* ��Ƶ֡��־ */
#define RTPC_SDK_AUDIO_FRAME_FLAG	0x00000002		/* ��Ƶ֡��־ */
#define RTPC_SDK_EVENT_FRAME_FLAG	0x00000004		/* �¼�֡��־ */
#define RTPC_SDK_RTP_FRAME_FLAG		0x00000008		/* RTP֡��־ */
#define RTPC_SDK_SDP_FRAME_FLAG		0x00000010		/* SDP֡��־ */
#define RTPC_SDK_MEDIA_INFO_FLAG	0x00000020		/* ý�����ͱ�־*/

/* ��Ƶ�ؼ��ֱ�ʶ */
#define RTPC_SDK_VIDEO_FRAME_I		0x01		/* I֡ */
#define RTPC_SDK_VIDEO_FRAME_P		0x02		/* P֡ */
#define RTPC_SDK_VIDEO_FRAME_B		0x03		/* B֡ */
#define RTPC_SDK_VIDEO_FRAME_J		0x04		/* JPEG */

#define AV_CODEC_ID_H264 28

/* ֡��Ϣ */
typedef struct
{
	unsigned int	codec;				/* ����Ƶ��ʽ */

	unsigned int	type;				/* ��Ƶ֡���� */
	unsigned char	fps;				/* ��Ƶ֡�� */
	unsigned short	width;				/* ��Ƶ�� */
	unsigned short  height;				/* ��Ƶ�� */

	unsigned int	reserved1;			/* ��������1 */
	unsigned int	reserved2;			/* ��������2 */

	unsigned int	sample_rate;		/* ��Ƶ������ */
	unsigned int	channels;			/* ��Ƶ������ */
	unsigned int	bits_per_sample;	/* ��Ƶ�������� */

	unsigned int	length;				/* ����Ƶ֡��С */
	unsigned int    timestamp_usec;		/* ʱ���,΢�� */
	unsigned int	timestamp_sec;		/* ʱ��� �� */

	float			bitrate;			/* ������ */
	float			losspacket;			/* ������ */
} RTP_FRAME_INFO;

typedef int(CALLBACK* RTPCOnReceivedProc)(void* state, char* buffer, int size, int media, RTP_FRAME_INFO* frame);

BOOL RTPC_Init(HANDLE* handle, void* state);
BOOL RTPC_Deinit(HANDLE* handle);
BOOL RTPC_OnReceived(HANDLE handle, RTPCOnReceivedProc recevied);
BOOL RTPC_Connect(HANDLE handle, char* server, int port, char* sim);
BOOL RTPC_Enable(HANDLE handle, BOOL enable);
BOOL RTPC_Connect(HANDLE handle, char* server, int port, char* sim, int channel);

#endif