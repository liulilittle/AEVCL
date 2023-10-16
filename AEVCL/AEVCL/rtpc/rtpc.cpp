#include <stdio.h>
#include <vector>
#include <queue>
#include "rtpc.h"
#include "rtp.h"
#include "rtp_264p.h"
#include <assert.h>
#include <time.h>
#include "..\mempool\mempool.h"

#if defined(__linux__)
#include <unistd.h>
#else
#define sleep(ms) Sleep(ms)
#endif

typedef struct
{
	RTP_FRAME_INFO* frame;
	int size;
	char* buffer;
} RTPC_WORK_FRAME_T;

typedef struct
{
	RTPCOnReceivedProc recevied; // 数据到达
	HANDLE socket; // 关联的套接字
	char recoverystate; // 0、无故障，1、重连
	BOOL enable;
	RTP_HEADER* rtp; // RTP数据包
	int recvsize; // 收取大小
	RTP_264_UNPACK* pack; // 264解包工具
	HANDLE callback; // 回调工作线程
	void* state;
	std::queue<RTP_HEADER*>* frames;
	char* sim;
	int channel;
} RTPC_WORK_OBJ_T;

typedef struct
{
	std::vector<RTPC_WORK_OBJ_T*> vtWork;
	HANDLE htReceived; // 工作收取线程
	HANDLE htRecovery; // 工作恢复线程
	HANDLE hExitMutex;
	CRITICAL_SECTION hInitMutex;
	HANDLE hRecvMutex;
} RTPC_RUNTIME_OBJ_T;

static RTPC_RUNTIME_OBJ_T* rtpcWorkRuntime = NULL;

BOOL RTPC_Authentication(RTPC_WORK_OBJ_T* work) {
	RTP_HEADER rtp;
	rtp_header_default(&rtp);
	if (work->sim != NULL)
		memcpy(rtp.sim, work->sim, 6);
	rtp.type = 0x0F;
	rtp.channel = work->channel;
	int len = 0;
	char* buf = rtp_header_to(&rtp, &len);
	if (!sock_write(work->socket, buf, len))
		return FALSE;
	mempool_free(buf);
	return TRUE;
}

void RTPC_ProcessEvent(RTPC_WORK_OBJ_T* work)
{
	RTP_HEADER* rtp = work->rtp;
	RTP_BODY* body = &rtp->body;
	if (body->size >= rtp->size) {
		work->recvsize = 0;
		work->rtp = NULL;
		//
		work->frames->push(rtp);
	}
}

void RTPC_ReceivedWork(void* state)
{
	char* buffer = (char*)mempool_alloc(1500);
	while (TRUE)
	{
		if (rtpcWorkRuntime == NULL)
			sleep(500);
		EnterCriticalSection(&rtpcWorkRuntime->hInitMutex);
		ResetEvent(rtpcWorkRuntime->hRecvMutex);
		int untreated = 0;
		for (int i = 0; i < rtpcWorkRuntime->vtWork.size(); i++) {
			RTPC_WORK_OBJ_T* work = rtpcWorkRuntime->vtWork[i];
			if (!sock_inited(work->socket) || !work->enable) {
				untreated++;
				continue;
			}
			int len = 0;
			if (work->recvsize <= NULL)
				work->recvsize = 30;
			int error = sock_read(work->socket, buffer, work->recvsize, &len, FALSE);
			if (error < 0 && work->recoverystate == 0)
				work->recoverystate = 1;
			if (error == 0)
				continue;
			if (error > 0) {
				RTP_HEADER* rtp = work->rtp;
				if (rtp == NULL) {
					rtp = rtp_header_parse(buffer, len);
					if (rtp != NULL) {
						RTP_BODY* body = &rtp->body;
						work->rtp = rtp;
						work->recvsize = (rtp->size - body->size);
						RTPC_ProcessEvent(work);
					}
				}
				else {
					RTP_BODY* body = &rtp->body;
					char* offset = (body->buf + body->size);
					body->size += len;
					memcpy(offset, buffer, len);
					RTPC_ProcessEvent(work);
				}
			}
		}
		int size = rtpcWorkRuntime->vtWork.size();
		SetEvent(rtpcWorkRuntime->hRecvMutex);
		LeaveCriticalSection(&rtpcWorkRuntime->hInitMutex);
		if (untreated >= size)
			sleep(1);
	}
	mempool_free(buffer);
}

void RTPC_CallbackWork(void* state)
{
	RTPC_WORK_OBJ_T* work = (RTPC_WORK_OBJ_T*)state;
	while (TRUE)
	{
		if (work->frames->empty())
			sleep(1);
		else {
			RTP_HEADER* rtp = work->frames->front();
			work->frames->pop();
			if (rtp != NULL && rtp->body.buf != NULL) {
				RTP_264_UNPACK* pack = work->pack;
				if (pack == NULL) {
					pack = new RTP_264_UNPACK;
					work->pack = pack;
				}
				int len = 0;
				BYTE* f = pack->Parse_RTP_Packet(rtp, &len);
				if (f != NULL) {
					// printf(" pack addr: %p,len:%d,seq:%d\r\n  ", pack, len, rtp->seq);
					RTPC_WORK_FRAME_T* ff = (RTPC_WORK_FRAME_T*)mempool_alloc(sizeof(RTPC_WORK_FRAME_T));
					ff->buffer = (char*)mempool_alloc(len);
					memcpy(ff->buffer, f, len);
					ff->size = len;
					//
					ff->frame = (RTP_FRAME_INFO*)mempool_alloc(sizeof(RTP_FRAME_INFO));
					memset(ff->frame, 0x00, sizeof(RTP_FRAME_INFO));
					ff->frame->codec = AV_CODEC_ID_H264;
					ff->frame->width = pack->getWidth();
					ff->frame->height = pack->getHeight();
					ff->frame->channels = 2;
					ff->frame->bits_per_sample = 16;
					ff->frame->sample_rate = 48000;
					ff->frame->length = len;
					if (pack->isKeyFrame())
						ff->frame->type = RTPC_SDK_VIDEO_FRAME_I;
					else if (pack->isDstFrame())
						ff->frame->type = RTPC_SDK_VIDEO_FRAME_P;
					else
						ff->frame->type = RTPC_SDK_VIDEO_FRAME_B;
					if (ff && work->recevied != NULL)
						work->recevied(work->state, ff->buffer, ff->size, RTPC_SDK_VIDEO_FRAME_FLAG, ff->frame);
					mempool_free(ff->frame);
					mempool_free(ff->buffer);
					mempool_free(ff);
				}
			}
			if (rtp != NULL) {
				if (rtp->body.buf != NULL) {
					mempool_free(rtp->body.buf);
					rtp->body.buf = NULL;
				}
				mempool_free(rtp);
			}
		}
	}
}

void RTPC_RecoveryWork(void* state)
{
	while (TRUE)
	{
		if (rtpcWorkRuntime != NULL) {
			WaitForSingleObject(rtpcWorkRuntime->hExitMutex, INFINITE);
			std::vector<RTPC_WORK_OBJ_T*>* vt = &rtpcWorkRuntime->vtWork;
			for (int i = 0; i < vt->size(); i++)
			{
				RTPC_WORK_OBJ_T* work = vt->at(i);
				if (!work->enable)
					continue;
				if (work->recoverystate == 1) {
					work->recvsize = 0;
					work->rtp = NULL;
					//
					if (work->pack != NULL)
						delete work->pack;
					if (sock_recovery(work->socket) && RTPC_Authentication(work))
						work->recoverystate = 0;
				}
			}
		}
		sleep(5000);
	}
}

char* RTPC_TextRandom(int length)
{
	char* string = NULL;
	int flag, i;
	timeBeginPeriod(1);
	srand(timeGetTime());
	timeEndPeriod(1);
	if ((string = (char*)mempool_alloc(length)) == NULL)
		return NULL;
	for (i = 0; i < length - 1; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			string[i] = 'A' + rand() % 26;
			break;
		case 1:
			string[i] = 'a' + rand() % 26;
			break;
		case 2:
			string[i] = '0' + rand() % 10;
			break;
		default:
			string[i] = 'x';
			break;
		}
	}
	string[length - 1] = '\0';
	return string;
}

HANDLE RTPC_CreateEvent()
{
	char* str = RTPC_TextRandom(8);
	HANDLE h = CreateEventA(NULL, FALSE, FALSE, str);
	mempool_free(str);
	return h;
}

BOOL RTPC_Init(HANDLE* handle, void* state)
{
	if (handle == NULL)
		return FALSE;
	RTPC_WORK_OBJ_T* work = (RTPC_WORK_OBJ_T*)mempool_alloc(sizeof(RTPC_WORK_OBJ_T));
	memset(work, 0x00, sizeof(RTPC_WORK_OBJ_T));
	if (rtpcWorkRuntime == NULL)
	{
		rtpcWorkRuntime = new RTPC_RUNTIME_OBJ_T;
		//
		memset(&rtpcWorkRuntime->hInitMutex, 0x00, sizeof(CRITICAL_SECTION));
		InitializeCriticalSection(&rtpcWorkRuntime->hInitMutex);
		//
		rtpcWorkRuntime->htReceived = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&RTPC_ReceivedWork, NULL, 0, 0);
		SetThreadPriority(rtpcWorkRuntime->htReceived, THREAD_PRIORITY_HIGHEST);

		rtpcWorkRuntime->htRecovery = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&RTPC_RecoveryWork, NULL, 0, 0);
		SetThreadPriority(rtpcWorkRuntime->htRecovery, THREAD_PRIORITY_LOWEST);

		rtpcWorkRuntime->hRecvMutex = RTPC_CreateEvent();
	}
	EnterCriticalSection(&rtpcWorkRuntime->hInitMutex);
	if (work->frames == NULL)
		work->frames = new std::queue<RTP_HEADER*>;
	*handle = work;
	work->state = state;
	RTPC_Enable(work, TRUE);
	if (work->callback == NULL) {
		work->callback = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&RTPC_CallbackWork, work, 0, 0);
		SetThreadPriority(work->callback, THREAD_PRIORITY_LOWEST);
	}
	rtpcWorkRuntime->vtWork.push_back(work);
	LeaveCriticalSection(&rtpcWorkRuntime->hInitMutex);
	return TRUE;
}

BOOL RTPC_Deinit(HANDLE* handle)
{
	if (handle == NULL)
		return FALSE;
	rtpcWorkRuntime->hExitMutex = RTPC_CreateEvent();
	WaitForSingleObject(rtpcWorkRuntime->hRecvMutex, INFINITE);
	RTPC_WORK_OBJ_T* work = NULL;
	if (*handle != NULL) {
		work = (RTPC_WORK_OBJ_T*)(*handle);
		RTP_HEADER* rtp = work->rtp;
		if (work->callback != NULL)
			TerminateThread(work->callback, 0);
		if (work->pack != NULL) {
			mempool_free(work->pack);
			work->pack = NULL;
		}
		if (rtp != NULL) {
			if (rtp->body.buf != NULL)
				mempool_free(rtp->body.buf);
			mempool_free(rtp);
			work->rtp = NULL;
		}
		if (sock_inited(work->socket))
			sock_deinit(&work->socket);
		work->callback = NULL;
		std::vector<RTPC_WORK_OBJ_T*>* vt = &rtpcWorkRuntime->vtWork;
		auto i = std::find(vt->cbegin(), vt->cend(), work);
		if (i != vt->cend())
			vt->erase(i);
		*handle = NULL;
		// mempool_free(work);
	}
	SetEvent(rtpcWorkRuntime->hExitMutex);
	CloseHandle(rtpcWorkRuntime->hExitMutex);
	//
	return TRUE;
}

BOOL RTPC_Enable(HANDLE handle, BOOL enable)
{
	RTPC_WORK_OBJ_T* work = (RTPC_WORK_OBJ_T*)handle;
	if (work == NULL)
		return FALSE;
	work->enable = enable;
	return TRUE;
}

BOOL RTPC_OnReceived(HANDLE handle, RTPCOnReceivedProc recevied)
{
	if (handle == NULL || recevied == NULL)
		return FALSE;
	RTPC_WORK_OBJ_T* work = (RTPC_WORK_OBJ_T*)handle;
	work->recevied = recevied;
	return TRUE;
}

BOOL RTPC_Control(HANDLE handle, char* data, int size)
{
	RTPC_WORK_OBJ_T* work = (RTPC_WORK_OBJ_T*)handle;
	if (work == NULL || size < 0 || (data == NULL && size > 0))
		return FALSE;
	if (!sock_inited(work->socket))
		sock_init(&work->socket, TRUE);
	RTP_HEADER rtp;
	rtp_header_default(&rtp);
	if (work->sim != NULL)
		memcpy(rtp.sim, work->sim, 6);
	rtp.channel = work->channel;
	rtp.type = 0x08;
	rtp.size = size;
	if (data != NULL) {
		RTP_BODY body = rtp.body;
		body.buf = data;
	}
	int len = 0;
	char* buf = rtp_header_to(&rtp, &len);
	if (!sock_write(work->socket, buf, len))
		return FALSE;
	mempool_free(buf);
	return TRUE;
}

BOOL RTPC_Connect(HANDLE handle, char* server, int port, char* sim, int channel)
{
	RTPC_WORK_OBJ_T* work = (RTPC_WORK_OBJ_T*)handle;
	if (work == NULL)
		return FALSE;
	else {
		work->sim = sim;
		work->channel = channel;
	}
	if (!sock_inited(work->socket))
		sock_init(&work->socket, TRUE);
	if (sock_connect(work->socket, server, port))
		return RTPC_Authentication(work);
	return FALSE;
}
