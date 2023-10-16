#ifndef __RTP_H__
#define __RTP_H__

#include <stdio.h>
#include <windows.h>

#define RTP_HEADER_SIZE 30
// 0x30 0x31 0x63 0x64
#define RTP_HEADER_STX_FLAGS 0x64633130

typedef struct 
{
	int size;
	char* buf;
} RTP_BODY;

typedef struct
{
	DWORD  stx;		// 帧头
	BYTE   cc;      // 固定为1
	BOOL   x;       // RTP头是否需要扩展位，固定为0
	BOOL   p;       // 固定为0
	BYTE   v;       // 固定为2
	BYTE   pt;      // 负载类型
	BOOL   m;       // 完整数据帧的边界
	WORD   seq;		// 包的序列化
	CHAR   sim[6];		// 终端设备SIM卡号
	BYTE   channel; // 按照JT/T 1076~2016中的表2
	BYTE   type;	// 数据类型（0000、I帧，0001、P帧率，0010、B帧，0011、音频帧，0100、透传数据）
	BYTE   pack;	// 分表处理标记（0000、原子包，0001、分包时第一个包，0010、分布处理时最后一包，0011、分包处理时的中间包）
	//
	BOOL   pack_a;   // 原子包
	BOOL   pack_f;  // 第一包
	BOOL   pack_l;   // 最后一包
	BOOL   pack_m;    // 中间包
	//
	BOOL   type_i;		// 视频I帧
	BOOL   type_p;		// 视频P帧
	BOOL   type_b;		// 视频B帧
	BOOL   type_a;		// 音频帧
	BOOL   type_t;		// 透传数据
	//
	LONG   ts;       // 时间戳
	//LONG   ssrc;     // 同步源
	WORD   invl_i;	 // 与上一关键帧的时间间隔（不是视频帧时则没有该字段）
	WORD   invl_p;	 // 与上一帧的时间间隔（不是视频帧时则没有该字段）
	WORD   size;	 // NAL码流数据体长度
	RTP_BODY body;   // 包体缓冲区数据指针
} RTP_HEADER;

long long edian_btol(char* buf, int ofs, int size);
char* edian_ltob(long num, int size);
long edian_flipbit(long num, int start, int end);
RTP_HEADER* rtp_header_parse(char* buffer, int size);
char* rtp_header_to(RTP_HEADER* header, int* len);
bool rtp_header_default(RTP_HEADER* header);

#endif