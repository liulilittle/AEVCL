#if !EDIAN_ENABLE_SETTING
#define EDIAN_ENABLE_SETTING
#endif

namespace AEVCL.Forwarding
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;

    public unsafe sealed class RTP_HEADER : EventArgs
    {
        public const int RTP_HEADER_SIZE = 30;
        public const int RTP_HEADER_STX_FLAGS = 0x64633130;
        public const int NULL = 0;

        public uint stx;		// 帧头
        public byte cc;      // 固定为1
        public bool x;       // RTP头是否需要扩展位，固定为0
        public bool p;       // 固定为0
        public byte v;       // 固定为2
        public byte pt;      // 负载类型
        public bool m;       // 完整数据帧的边界
        public ushort seq;		// 包的序列化

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 6)]
        public string sim;		// 终端设备SIM卡号
        public byte channel; // 按照JT/T 1076~2016中的表2
        public byte type;	// 数据类型（0000、I帧，0001、P帧率，0010、B帧，0011、音频帧，0100、透传数据）
        public byte pack;	// 分表处理标记（0000、原子包，0001、分包时第一个包，0010、分布处理时最后一包，0011、分包处理时的中间包）
        //
        public bool pack_a;   // 原子包
        public bool pack_f;  // 第一包
        public bool pack_l;   // 最后一包
        public bool pack_m;    // 中间包
        //
        public bool type_i;		// 视频I帧
        public bool type_p;		// 视频P帧
        public bool type_b;		// 视频B帧
        public bool type_a;		// 音频帧
        public bool type_t;		// 透传数据
        //
        public long ts;       // 时间戳
        //LONG   ssrc;     // 同步源
        public ushort invl_i;	 // 与上一关键帧的时间间隔（不是视频帧时则没有该字段）
        public ushort invl_p;	 // 与上一帧的时间间隔（不是视频帧时则没有该字段）
        public ushort size;	 // NAL码流数据体长度
        public RTP_BODY body;   // 包体缓冲区数据指针

        [DllImport("msvcrt.dll", EntryPoint = "memcpy", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        public static extern void* memcpy(void* dest, void* src, int count);

        private static long edian_btol(byte* buf, int ofs, int size)
        {
#if EDIAN_ENABLE_SETTING
            long value = 0;
            if (buf != null)
            {
                byte* ptr = (byte*)&value;
                for (int i = 0; i < size; i++)
                    ptr[i] = buf[i];
            }
            return value;
#else
            long num = 0;
            if (buf != null)
            {
                byte* ptr = (byte*)&num;
                size--;
                for (int i = 0; i <= size; i++)
                    ptr[i] = buf[size - i];
            }
            return num;
#endif
        }

        private static char* edian_ltob(long num, int size)
        {
            if (size <= 0)
                return null;
            char* buf = (char*)Marshal.UnsafeAddrOfPinnedArrayElement(new char[size], 0);
            char* ptr = (char*)&num;
#if EDIAN_ENABLE_SETTING
            for (int i = 0; i < size; i++)
                buf[i] = ptr[i];
            return buf;
#else
            size--;
            for (int i = 0; i <= size; i++)
                buf[size - i] = ptr[i];
            return buf;
#endif
        }

        private static long edian_flipbit(long num, int start, int end)
        {
#if EDIAN_ENABLE_SETTING
            return num;
#else
            long value = 0;
            for (int i = start; i < end; i++)
            {
                value <<= 1;
                value |= (num >> i) & 1;
            }
            return value;
#endif
        }

        public static RTP_HEADER Parse(byte* buf, int size)
        {
            if (buf == null || size < RTP_HEADER_SIZE)
                return null;
            byte* low = buf;
            long stx = edian_btol(buf, 0, 4);
            if (stx != RTP_HEADER_STX_FLAGS)
                return null;
            RTP_HEADER header = new RTP_HEADER();
            //
            header.stx = (uint)stx;
            buf += 4;
            int eax = (int)edian_flipbit(*buf, 0, 8);
            header.cc = (byte)(eax & 0x0F); // 低四位
            //
            eax = (eax & 0xF0) >> 4; // 高四位
            header.x = (eax & 1) > 0;
            header.p = ((eax >> 2) & 1) > 0;
            header.v = (byte)(eax >> 2);
            buf += 1;
            //
            eax = (int)edian_flipbit(*buf, 0, 8);
            header.pt = (byte)(eax & 0x7F);
            header.m = ((eax >> 7) & 1) > 0;
            buf += 1;
            //
            header.seq = (ushort)edian_btol(buf, 0, 2);
            buf += 2;
            header.sim = new string((sbyte*)buf, 0, 6);
            // memcpy(header->sim, buf, 6);
            buf += 6;
            header.channel = (byte)*buf;
            buf += 1;
            //
            eax = (int)edian_flipbit(*buf, 0, 8);
            header.pack = (byte)(eax & 0x0F);
            header.type = (byte)((eax & 0xF0) >> 4);
            buf += 1;
            //
            header.type_i = (header.type == 0);
            header.type_p = (header.type == 1);
            header.type_b = (header.type == 2);
            header.type_a = (header.type == 3);
            header.type_t = (header.type == 4);
            //
            header.ts = edian_btol(buf, 0, 8);
            buf += 8;

            header.invl_i = (ushort)edian_btol(buf, 0, 2);
            buf += 2;
            header.invl_p = (ushort)edian_btol(buf, 0, 2);
            buf += 2;

            header.pack_a = (header.pack == 0);
            header.pack_f = (header.pack == 1);
            header.pack_l = (header.pack == 2);
            header.pack_m = (header.pack == 3);
            //
            header.size = (ushort)edian_btol(buf, 0, 2);
            buf += 2;
            //
            RTP_BODY body = new RTP_BODY();
            header.body = body;
            body.size = (int)(size - (buf - low));
            //
            int ofs = (int)(buf - low);
            RTP_BODY_BUFFER buffer = new RTP_BODY_BUFFER(header.size + ofs);
            (body.buf = buffer).Write(low, ofs);
            if (body.size > 0)
            {
                buffer.Write(buf, body.size);
            }
            return header;
        }
    }
}
