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
	DWORD  stx;		// ֡ͷ
	BYTE   cc;      // �̶�Ϊ1
	BOOL   x;       // RTPͷ�Ƿ���Ҫ��չλ���̶�Ϊ0
	BOOL   p;       // �̶�Ϊ0
	BYTE   v;       // �̶�Ϊ2
	BYTE   pt;      // ��������
	BOOL   m;       // ��������֡�ı߽�
	WORD   seq;		// �������л�
	CHAR   sim[6];		// �ն��豸SIM����
	BYTE   channel; // ����JT/T 1076~2016�еı�2
	BYTE   type;	// �������ͣ�0000��I֡��0001��P֡�ʣ�0010��B֡��0011����Ƶ֡��0100��͸�����ݣ�
	BYTE   pack;	// �ֱ����ǣ�0000��ԭ�Ӱ���0001���ְ�ʱ��һ������0010���ֲ�����ʱ���һ����0011���ְ�����ʱ���м����
	//
	BOOL   pack_a;   // ԭ�Ӱ�
	BOOL   pack_f;  // ��һ��
	BOOL   pack_l;   // ���һ��
	BOOL   pack_m;    // �м��
	//
	BOOL   type_i;		// ��ƵI֡
	BOOL   type_p;		// ��ƵP֡
	BOOL   type_b;		// ��ƵB֡
	BOOL   type_a;		// ��Ƶ֡
	BOOL   type_t;		// ͸������
	//
	LONG   ts;       // ʱ���
	//LONG   ssrc;     // ͬ��Դ
	WORD   invl_i;	 // ����һ�ؼ�֡��ʱ������������Ƶ֡ʱ��û�и��ֶΣ�
	WORD   invl_p;	 // ����һ֡��ʱ������������Ƶ֡ʱ��û�и��ֶΣ�
	WORD   size;	 // NAL���������峤��
	RTP_BODY body;   // ���建��������ָ��
} RTP_HEADER;

long long edian_btol(char* buf, int ofs, int size);
char* edian_ltob(long num, int size);
long edian_flipbit(long num, int start, int end);
RTP_HEADER* rtp_header_parse(char* buffer, int size);
char* rtp_header_to(RTP_HEADER* header, int* len);
bool rtp_header_default(RTP_HEADER* header);

#endif