// MPEG2RTP.h
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include <winsock2.h>


//#include "mem.h"

//


#define PACKET_BUFFER_END            (unsigned int)0x00000000


#define MAX_RTP_PKT_LENGTH     1400

#define DEST_IP                "192.168.100.114"
#define DEST_PORT            6813

#define H264                    98

// typedef struct 
// {
//     /**//* byte 0 */
//     unsigned char csrc_len:4;        /**//* expect 0 */
//     unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
//     unsigned char padding:1;        /**//* expect 0 */
//     unsigned char version:2;        /**//* expect 2 */
//     /**//* byte 1 */
//     unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
//     unsigned char marker:1;        /**//* expect 1 */
//     /**//* bytes 2, 3 */
//     unsigned short seq_no;            
//     /**//* bytes 4-7 */
//     unsigned  long timestamp;        
//     /**//* bytes 8-11 */
//     unsigned long ssrc;            /**//* stream number is used here. */
// } RTP_FIXED_HEADER;
#pragma pack(1)
typedef struct
{
	//LITTLE_ENDIAN  

	unsigned long    protocol;
	unsigned short   cc : 4;      /* CSRC count                 */
	unsigned short   x : 1;       /* header extension flag      */
	unsigned short   p : 1;       /* padding flag               */
	unsigned short   v : 2;       /* packet type                */
	unsigned short   pt : 7;      /* payload type               */
	unsigned short   m : 1;       /* marker bit                 */

	unsigned short   seq;         /* sequence number            */
	unsigned char    sim[6];	  // 终端设备SIM卡号
	unsigned char    channel;	  // 终端设备通道号
	unsigned char    datatype: 4;	  // 数据类型（0000、I帧，0001、P帧率，0010、B帧，0011、音频帧，0100、透传数据）
	unsigned char    split: 4;		  // 分包处理标记（0000、原子包，0001、分包时第一个包，0010、分布处理时最后一包，0011、分包处理时的中间包）
	//unsigned char    ts[8];         /* timestamp                  */
	UINT64    ts;         /* timestamp                  */
	unsigned short   lifi;		  // 缩写，与上一关键帧的时间间隔（不是视频帧时则没有该字段）
	unsigned short   lfi;		  // 与上一帧的时间间隔（不是视频帧时则没有该字段）
	unsigned short   len;	 // NAL码流数据体长度

} RTP_FIXED_HEADER;

#pragma pack()

typedef struct {
    //byte 0
	unsigned char TYPE:5;
    unsigned char NRI:2;
	unsigned char F:1;    
         
} NALU_HEADER; /**//* 1 BYTES */

typedef struct {
    //byte 0
    unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;    
            
             
} FU_INDICATOR; /**//* 1 BYTES */

typedef struct {
    //byte 0
    unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;    
} FU_HEADER; /**//* 1 BYTES */




BOOL InitWinsock();
