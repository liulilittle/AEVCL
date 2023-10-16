#ifndef __RTP_264P_H__
#define __RTP_264P_H__

#include <stdio.h>
#include "rtp.h"
#include "base64.h"

typedef struct 
{
	int width;
	int height;
} NAL_264_SPS_INFO;

UINT Ue(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
	//计算0bit的个数
	UINT nZeroNum = 0;
	while (nStartBit < nLen * 8)
	{
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
		{
			break;
		}
		nZeroNum++;
		nStartBit++;
	}
	nStartBit++;
	//计算结果
	DWORD dwRet = 0;
	for (UINT i = 0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return (1 << nZeroNum) - 1 + dwRet;
}

int Se(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
	int UeVal = Ue(pBuff, nLen, nStartBit);
	double k = UeVal;
	int nValue = ceil(k / 2); // ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2 == 0)
		nValue = -nValue;
	return nValue;
}

DWORD u(UINT BitCount, BYTE * buf, UINT &nStartBit)
{
	DWORD dwRet = 0;
	for (UINT i = 0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return dwRet;
}

bool h264_decode_seq_parameter_set(BYTE * buf, UINT nLen, int &Width, int &Height)
{
	UINT StartBit = 0;
	int forbidden_zero_bit = u(1, buf, StartBit);
	int nal_ref_idc = u(2, buf, StartBit);
	int nal_unit_type = u(5, buf, StartBit);
	if (nal_unit_type == 7)
	{
		int profile_idc = u(8, buf, StartBit);
		int constraint_set0_flag = u(1, buf, StartBit); // (buf[1] & 0x80)>>7;
		int constraint_set1_flag = u(1, buf, StartBit); // (buf[1] & 0x40)>>6;
		int constraint_set2_flag = u(1, buf, StartBit); // (buf[1] & 0x20)>>5;
		int constraint_set3_flag = u(1, buf, StartBit); // (buf[1] & 0x10)>>4;
		int reserved_zero_4bits = u(4, buf, StartBit);
		int level_idc = u(8, buf, StartBit);

		int seq_parameter_set_id = Ue(buf, nLen, StartBit);

		if (profile_idc == 100 || profile_idc == 110 ||
			profile_idc == 122 || profile_idc == 144)
		{
			int chroma_format_idc = Ue(buf, nLen, StartBit);
			if (chroma_format_idc == 3)
				int residual_colour_transform_flag = u(1, buf, StartBit);
			int bit_depth_luma_minus8 = Ue(buf, nLen, StartBit);
			int bit_depth_chroma_minus8 = Ue(buf, nLen, StartBit);
			int qpprime_y_zero_transform_bypass_flag = u(1, buf, StartBit);
			int seq_scaling_matrix_present_flag = u(1, buf, StartBit);

			int seq_scaling_list_present_flag[8];
			if (seq_scaling_matrix_present_flag)
			{
				for (int i = 0; i < 8; i++) {
					seq_scaling_list_present_flag[i] = u(1, buf, StartBit);
				}
			}
		}
		int log2_max_frame_num_minus4 = Ue(buf, nLen, StartBit);
		int pic_order_cnt_type = Ue(buf, nLen, StartBit);
		if (pic_order_cnt_type == 0)
			int log2_max_pic_order_cnt_lsb_minus4 = Ue(buf, nLen, StartBit);
		else if (pic_order_cnt_type == 1)
		{
			int delta_pic_order_always_zero_flag = u(1, buf, StartBit);
			int offset_for_non_ref_pic = Se(buf, nLen, StartBit);
			int offset_for_top_to_bottom_field = Se(buf, nLen, StartBit);
			int num_ref_frames_in_pic_order_cnt_cycle = Ue(buf, nLen, StartBit);

			int *offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
			for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
				offset_for_ref_frame[i] = Se(buf, nLen, StartBit);
			delete[] offset_for_ref_frame;
		}
		int num_ref_frames = Ue(buf, nLen, StartBit);
		int gaps_in_frame_num_value_allowed_flag = u(1, buf, StartBit);
		int pic_width_in_mbs_minus1 = Ue(buf, nLen, StartBit);
		int pic_height_in_map_units_minus1 = Ue(buf, nLen, StartBit);

		Width = (pic_width_in_mbs_minus1 + 1) * 16;
		Height = (pic_height_in_map_units_minus1 + 1) * 16;

		return true;
	}
	else
		return false;
}

class RTP_264_UNPACK
{
#define RTP_VERSION 2  
#define BUF_SIZE (1024 * 500)  

public:
	RTP_264_UNPACK(unsigned char H264PAYLOADTYPE = 98) // 标准RTP协议定义的的H.264类型为96
		: m_bWaitKeyFrame(true), m_bSPSFound(false), m_bPPSFound(false)
		, m_bPrevFrameEnd(false)
		, m_bAssemblingFrame(false)
		, m_wSeq(1234)
		, m_ssrc(0), m_width(0), m_height(0)
	{

		m_pBuf = new BYTE[BUF_SIZE];
		if (m_pBuf != NULL) {

			m_H264PAYLOADTYPE = H264PAYLOADTYPE;
			m_pEnd = m_pBuf + BUF_SIZE;
			m_pStart = m_pBuf;
			m_dwSize = 0;
		}
		m_bIsKeyFrame = false;
	}

	~RTP_264_UNPACK(void)
	{
		delete[] m_pBuf;
	}

	int getWidth()
	{
		return m_width;
	}
	int getHeight()
	{
		return m_height;
	}
	//pBuf为H264 RTP视频数据包，nSize为RTP视频数据包字节长度，outSize为输出视频数据帧字节长度。  
	//返回值为指向视频数据帧的指针。输入数据可能被破坏。  
//	BYTE* Parse_RTP_Packet(BYTE *pBuf, unsigned short nSize, int *outSize)
	BYTE* Parse_RTP_Packet(RTP_HEADER* rtp, int *outSize)
	{
		m_bIsDstFrame = m_bIsKeyFrame = false;
		BYTE* pPayload = (BYTE*)rtp->body.buf;
		DWORD PayloadSize = rtp->body.size;
		int PayloadType = pPayload[0] & 0x1f;
		int NALType = PayloadType;
		if (NALType == 28) { // FU_A  
			if (PayloadSize < 2)
				return NULL;
			NALType = pPayload[1] & 0x1f;
		}
	
		if (NALType == 0x07 || NALType == 0x08)// || NALType == 0x06) // SPS、PPS 、SEI
		{ 
			if (NALType == 0x07) // SPS  
			{
				m_bSPSFound = true;
				h264_decode_seq_parameter_set((BYTE*)pPayload, PayloadSize, m_width, m_height);
			}
			else if (m_bSPSFound && (NALType == 0x08))
			{ // PPS  
				m_bPPSFound = true;
			}
			else
				return NULL;

			*((DWORD*)(m_pStart)) = 0x01000000;
			m_pStart += 4;
			m_dwSize += 4;
			CopyMemory(m_pStart, pPayload, PayloadSize);
			m_pStart += PayloadSize;
			m_dwSize += PayloadSize;

			m_wSeq = rtp->seq;
			m_bPrevFrameEnd = true;
			return NULL;
		}
		if (m_bSPSFound & m_bPPSFound)
		{

		}else
		{
			return NULL;
		}

		if (m_bWaitKeyFrame)
		{
			if (rtp->m) // frame end  
			{
				m_bPrevFrameEnd = true;
				if (!m_bAssemblingFrame) {
					m_wSeq = rtp->seq;
					return NULL;
				}
			}
			if (!m_bPrevFrameEnd) {
				m_wSeq = rtp->seq;
				return NULL;
			}
			else if(NALType != 0x05) { // KEY FRAME
				m_wSeq = rtp->seq;
				m_bPrevFrameEnd = false;
				return NULL;
			}
		}
		if (rtp->seq != (WORD)(m_wSeq + 1)) { // lost packet  
			m_wSeq = rtp->seq;
			SetLostPacket();
			return NULL;
		}
		else {
			m_wSeq = rtp->seq; // 码流正常  
			m_bAssemblingFrame = true;
			if (PayloadType != 28) { // whole NAL
				*((DWORD*)(m_pStart)) = 0x01000000;
				m_pStart += 4;
				m_dwSize += 4;
			}
			else { // FU_A
				if (pPayload[1] & 0x80) 
				{ // FU_A start
					*((DWORD*)(m_pStart)) = 0x01000000;
					m_pStart += 4;
					m_dwSize += 4;
					pPayload[1] = (pPayload[0] & 0xE0) | NALType;
					pPayload += 1;
					PayloadSize -= 1;
				}
				else {
					pPayload += 2;
					PayloadSize -= 2;
				}
			}
			if (m_pStart + PayloadSize < m_pEnd) {
				CopyMemory(m_pStart, pPayload, PayloadSize);
				m_dwSize += PayloadSize;
				m_pStart += PayloadSize;
			}
			else { // memory overflow 
				SetLostPacket();
				return NULL;
			}
			if (rtp->m) { // frame end  
				*outSize = m_dwSize;
				m_pStart = m_pBuf;
				m_dwSize = 0;
				if (NALType == 0x05) { // KEY FRAME  
					m_bWaitKeyFrame = false;
					m_bIsKeyFrame = true;
				}
				if (NALType == 0x01)
					m_bIsDstFrame = true;
				return m_pBuf;
			}
			else
				return NULL;
		}
	}

	void SetLostPacket()
	{
		m_bIsKeyFrame = false;
		m_bWaitKeyFrame = true;
		m_bPrevFrameEnd = false;
		m_bAssemblingFrame = false;
		m_pStart = m_pBuf;
		m_dwSize = 0;
		m_width = m_height = 0;
	}

	bool isKeyFrame() {
		return m_bIsKeyFrame;
	}

	int isDstFrame() {
		return m_bIsDstFrame;
	}

private:
//	RTP_HEADER m_RTP_Header;

	BYTE *m_pBuf;
	bool m_bSPSFound;
	bool m_bPPSFound;
	bool m_bWaitKeyFrame;
	bool m_bAssemblingFrame;
	bool m_bPrevFrameEnd;
	BYTE *m_pStart;
	BYTE *m_pEnd;
	DWORD m_dwSize;
	int m_width, m_height;
	WORD m_wSeq;

	BYTE m_H264PAYLOADTYPE;
	DWORD m_ssrc;
	bool m_bIsKeyFrame;
	bool m_bIsDstFrame;
};

#endif