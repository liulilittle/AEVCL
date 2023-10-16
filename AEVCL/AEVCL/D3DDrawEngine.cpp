#include "stdafx.h"
#include "D3DDrawEngine.h"
#include "mempool\mempool.h"

CD3DDrawEngine::CD3DDrawEngine(RTSP_PLAY_OBJ_T* play)
{
	m_pPlayObjT = play;
	memset(&m_pLastYuvFrame, 0x00, sizeof(MEDIA_FRAME_INFO));
	memset(&m_pLastFrame, 0x00, sizeof(MEDIA_FRAME_INFO));
}

void CD3DDrawEngine::Run()
{
	unsigned int deviceLostTime = (unsigned int)time(NULL) - 2;
	m_pPlayObjT->rtpTimestamp = 0;
	int	iDropFrame = 0;	// 丢帧机制
	int iLastDisplayYuvIdx = -1;
	m_pPlayObjT->displayThread.flag = 0x02;
	while (m_pPlayObjT->displayThread.flag != 0x03)
	{
		unsigned int rtpTimestamp = m_pPlayObjT->rtpTimestamp;
		int iDispalyYuvIdx = -1;
		int iYuvFrameNum = 0;
		unsigned int getNextFrame = 0;
		do
		{
			for (int iYuvIdx = 0; iYuvIdx < MAX_YUV_FRAME_NUM; iYuvIdx++)
			{
				if (m_pPlayObjT->yuvFrame[iYuvIdx].frameinfo.length > 0)
				{
					unsigned int newTimestamp = m_pPlayObjT->yuvFrame[iYuvIdx].frameinfo.timestamp_sec * 1000 +
						m_pPlayObjT->yuvFrame[iYuvIdx].frameinfo.timestamp_usec / 1000;
					if (newTimestamp > rtpTimestamp && getNextFrame == 0)
					{
						rtpTimestamp = newTimestamp;
						iDispalyYuvIdx = iYuvIdx;
						getNextFrame = 1;
					}
					else if (newTimestamp <= rtpTimestamp)
					{
						rtpTimestamp = newTimestamp;
						iDispalyYuvIdx = iYuvIdx;
					}
					iYuvFrameNum++;
				}
			}
			Sleep(1000.0f / 30);
			if (iDispalyYuvIdx == -1 && m_pLastYuvFrame.Yuvsize > 0)
				this->D3D_AVOnPaint(deviceLostTime);
		} while (iDispalyYuvIdx == -1);
		m_pPlayObjT->dwDisconnectTime = 0;
		this->D3D_Prepare(iDispalyYuvIdx, iLastDisplayYuvIdx, iYuvFrameNum, iDropFrame, deviceLostTime);
		this->D3D_AVOnPaint(deviceLostTime);
	}
	D3D_Release(&m_pPlayObjT->d3dHandle);
	m_pPlayObjT->rtpTimestamp = 0;
	m_pPlayObjT->displayThread.flag = 0x00;
}

bool CD3DDrawEngine::HasFrame()
{
	return m_pLastYuvFrame.Yuvsize > 0;
}

bool CD3DDrawEngine::D3D_Prepare(int& iDispalyYuvIdx, int& iLastDisplayYuvIdx, int& iYuvFrameNum, int& iDropFrame, unsigned int& deviceLostTime)
{
	if (iDispalyYuvIdx < 0 || iDispalyYuvIdx >= MAX_YUV_FRAME_NUM)
		return FALSE;
	YUV_FRAME_INFO* g_pYuvFrame = &m_pPlayObjT->yuvFrame[iDispalyYuvIdx];
	RTP_FRAME_INFO* g_pRtpFrame = &g_pYuvFrame->frameinfo;
	if (g_pRtpFrame->width <= 0 || g_pRtpFrame->height <= 0)
		return FALSE;
	if ((NULL == m_pPlayObjT->hWnd) || (NULL != m_pPlayObjT->hWnd && (!IsWindow(m_pPlayObjT->hWnd))) || 
		(NULL != m_pPlayObjT->hWnd && (!IsWindowVisible(m_pPlayObjT->hWnd))))
	{
		m_pPlayObjT->rtpTimestamp = g_pRtpFrame->timestamp_sec * 1000 + g_pRtpFrame->timestamp_usec / 1000;
		memset(&g_pYuvFrame->frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
		return FALSE;
	}
	iLastDisplayYuvIdx = iDispalyYuvIdx;
	m_pPlayObjT->frameQueue = 0;
	SS_QUEUE_OBJ_T* g_pAVQueue = m_pPlayObjT->pAVQueue;
	SS_HEADER_T* g_pAVQHeader = NULL;
	if (NULL != g_pAVQueue && NULL != (g_pAVQHeader = g_pAVQueue->pQueHeader))
		m_pPlayObjT->frameQueue = g_pAVQHeader->videoframes;
	int iQue1_DecodeQueue = m_pPlayObjT->frameQueue; // 未解码的帧数
	int iQue2_DisplayQueue = iYuvFrameNum; // 已解码的帧数
	int nQueueFrame = iQue1_DecodeQueue + iQue2_DisplayQueue;
	int iCache = 0;
	if (NULL != m_pPlayObjT->frameCache)
		iCache = m_pPlayObjT->frameCache;
	memcpy(&m_pLastFrame, &g_pYuvFrame->frameinfo, sizeof(MEDIA_FRAME_INFO)); 
	if (nQueueFrame > iCache * 2)
		iDropFrame++;
	else
		iDropFrame = 0;
	if (iDropFrame < 0x02)
		memcpy(&m_pLastYuvFrame, g_pYuvFrame, sizeof(YUV_FRAME_INFO));
	return memset(&g_pYuvFrame->frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO)) != NULL;
}

D3D_OSD* CD3DDrawEngine::D3D_OBSCreate(YUV_FRAME_INFO* g_pYuvFrame, RTP_FRAME_INFO* g_pRtpFrame, int width, int height, unsigned int& deviceLostTime)
{
	if ((NULL == m_pPlayObjT->d3dHandle) && ((unsigned int)time(NULL) - deviceLostTime >= 2))
	{
		D3D_FONT font;
		memset(&font, 0x00, sizeof(D3D_FONT));
		font.bold = 0x00;
		wcscpy(font.name, TEXT("Arial Black"));
		font.size = (int)(float)((width)*0.02f);// 32;
		font.width = (int)(float)((font.size) / 2.5f);//13;
		if (NULL != m_pPlayObjT->hWnd && (IsWindow(m_pPlayObjT->hWnd)))
			D3D_Initial(&m_pPlayObjT->d3dHandle, m_pPlayObjT->hWnd, width, height, 0, 1, m_pPlayObjT->renderFormat, &font);
		if (NULL == m_pPlayObjT->d3dHandle) {
			deviceLostTime = (unsigned int)time(NULL);
			// 如果d3d 初始化失败,则清空帧头信息,以便解码线程继续解码下一帧
			m_pPlayObjT->rtpTimestamp = g_pRtpFrame->timestamp_sec * 1000 + g_pRtpFrame->timestamp_usec / 1000;
			memset(&g_pYuvFrame->frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
			Sleep(3);
			return FALSE;
		}
	}
	int fps = g_pRtpFrame->fps;
	int iOneFrameUsec = 1000 / 30;	// normal
	if (fps > 0)
		iOneFrameUsec = 1000 / fps;
	m_pPlayObjT->rtpTimestamp = g_pRtpFrame->timestamp_sec * 1000 + g_pRtpFrame->timestamp_usec / 1000;
	// 统计信息:  编码格式 分辨率 帧率 帧类型  码流  缓存帧数
	char sztmp[128] = { 0, };
	D3D_OSD* g_pD3dOsd = (D3D_OSD*)mempool_alloc(sizeof(D3D_OSD));
	memset(g_pD3dOsd, 0x00, sizeof(D3D_OSD));
	MByteToWChar(sztmp, g_pD3dOsd->string, sizeof(g_pD3dOsd->string) / sizeof(g_pD3dOsd->string[0]));
	SetRect(&g_pD3dOsd->rect, 2, 2, (int)wcslen(g_pD3dOsd->string) * 20, (int)(float)((height)*0.046f));
	g_pD3dOsd->color = RGB(0xff, 0xff, 0x00);
	g_pD3dOsd->shadowcolor = RGB(0x15, 0x15, 0x15);
	g_pD3dOsd->alpha = 180;
	return g_pD3dOsd;
}

void CD3DDrawEngine::D3D_AVOnPaint(unsigned int& deviceLostTime)
{
	RTP_FRAME_INFO* g_pRtpFrame = (RTP_FRAME_INFO*)&m_pLastFrame;
	int height = 0;
	int width = 0;
	RECT rcSrc;
	RECT rcDst;
	if (!IsRectEmpty(&m_pPlayObjT->rcSrcRender))
		CopyRect(&rcSrc, &m_pPlayObjT->rcSrcRender);
	else
		SetRect(&rcSrc, 0, 0, width, height);
	if (NULL != m_pPlayObjT->hWnd && (IsWindow(m_pPlayObjT->hWnd)))
		GetClientRect(m_pPlayObjT->hWnd, &rcDst);
	// 如果当前分辨率和之前的不同,则重新初始化d3d
	if (m_pLastFrame.width != g_pRtpFrame->width || m_pLastFrame.height != g_pRtpFrame->height)
		m_pPlayObjT->resetD3d = true;
	EnterCriticalSection(&m_pPlayObjT->crit);			// Lock
	{
		if (m_pPlayObjT->resetD3d)
		{
			m_pPlayObjT->resetD3d = false;
			D3D_Release(&m_pPlayObjT->d3dHandle);
		}
		width = g_pRtpFrame->width;
		height = g_pRtpFrame->height;
	}
	LeaveCriticalSection(&m_pPlayObjT->crit);			// Unlock
	D3D_OSD* g_pD3dOsd = this->D3D_OBSCreate(&m_pLastYuvFrame, g_pRtpFrame, width, height, deviceLostTime);
	D3D_UpdateData(m_pPlayObjT->d3dHandle, 0, (unsigned char*)m_pLastYuvFrame.pYuvBuf, width, height, &rcSrc, NULL,
		m_pPlayObjT->showStatisticalInfo, g_pD3dOsd);
	if (D3D_Render(m_pPlayObjT->d3dHandle, m_pPlayObjT->hWnd, m_pPlayObjT->ShownToScale, &rcDst) < 0)
	{
		deviceLostTime = (unsigned int)time(NULL);
		m_pPlayObjT->resetD3d = true;						// 需要重建D3D
	}
	mempool_free(g_pD3dOsd);
}
