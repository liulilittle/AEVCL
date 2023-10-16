namespace AEVCL.Forwarding
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;

    public unsafe sealed class RTP_BODY_BUFFER
    {
        [DllImport("msvcrt.dll", EntryPoint = "memcpy", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        public static extern void* memcpy(void* dest, void* src, int count);

        private byte[] buf = null;
        private int ofs = 0;
        private int size = 0;

        public RTP_BODY_BUFFER(int capacity)
        {
            if (capacity < 0)
                throw new ArgumentOutOfRangeException();
            size = capacity;
            buf = new byte[capacity];
        }

        public byte* GetBuffer()
        {
            fixed (byte* p = &buf[ofs])
                return p;
        }

        public byte[] ToArray()
        {
            return buf;
        }

        public int Capacity
        {
            get
            {
                return size - ofs;
            }
        }

        public int Length
        {
            get
            {
                return size;
            }
        }

        public int Position
        {
            get
            {
                return ofs;
            }
        }

        public void Seek(int offset, SeekOrigin loc)
        {
            if (offset < 0 || offset >= size)
                throw new ArgumentOutOfRangeException();
            if (loc == SeekOrigin.Begin)
                ofs = offset;
            else if (loc == SeekOrigin.Current)
                ofs += offset;
            else if (loc == SeekOrigin.End)
                if (offset < 0)
                    ofs = Math.Abs(offset);
                else
                    ofs = size - offset;
        }

        public void Write(byte[] src, int len)
        {
            if (src != null && src.Length > 0)
            {
                if (src.Length < len)
                    throw new ArgumentOutOfRangeException();
                fixed (byte* p = src)
                    this.Write(p, len);
            }
        }

        public void Write(byte* src, int len)
        {
            if (len < 0 || this.Capacity <= 0)
                throw new ArgumentOutOfRangeException();
            byte* dst = this.GetBuffer();
            memcpy(dst, src, len);
            ofs += len;
        }
    }
}
