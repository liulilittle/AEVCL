namespace AEVCL.Forwarding
{
    using System;
    using System.Net.Sockets;
    using System.Net;

    /// <summary>
    /// 客户端会话上下文
    /// </summary>
    public sealed class RtpClientSession : EventArgs
    {
        private HQueue<byte[]> g_pBufferPools = null;
        private string g_szTSimNum = null;
        private int g_nTChanelNUm = 0;
        private IRtpClientStream g_pCntStream = null;

        public RtpClientSession(IRtpClientStream stream, string sim, int chanels)
        {
            if (stream == null)
                throw new ArgumentException();
            g_nTChanelNUm = chanels;
            g_szTSimNum = sim;
            g_pCntStream = stream;
        }

        public bool Equals(string sim, int chanels)
        {
            return (sim == g_szTSimNum) && (chanels == g_nTChanelNUm);
        }

        public IRtpClientStream Stream
        {
            get
            {
                return g_pCntStream;
            }
        }

        public string SimCardNumber
        {
            get
            {
                return g_szTSimNum;
            }
        }

        public int ChannelNumber
        {
            get
            {
                return g_nTChanelNUm;
            }
        }

        /// <summary>
        /// 添加会话
        /// </summary>
        /// <param name="buffer"></param>
        public void Wrtie(byte[] buffer)
        {
            lock (this)
            {
                if (g_pBufferPools == null)
                {
                    g_pBufferPools = new HQueue<byte[]>();
                }
            }
            g_pBufferPools.Enqueue(buffer);
        }

        /// <summary>
        /// 执行事务
        /// </summary>
        public bool SendTo()
        {
            lock (this)
            {
                if (g_pBufferPools == null)
                {
                    g_pBufferPools = new HQueue<byte[]>();
                }
            }
            if (g_pCntStream.CanWrite)
            {
                byte[] buffer = g_pBufferPools.Dequeue();
                if (buffer == null)
                {
                    return false;
                }
                g_pCntStream.Write(buffer);
                {
                    return true;
                }
            }
            return false;
        }
    }
}
