namespace AEVCL.Forwarding
{
    using System;
    using System.Net.Sockets;
    using System.Runtime.CompilerServices;

    public class RtpClientStream : IRtpClientStream
    {
        private SocketAsyncEventArgs g_pCntSendEvt = null;
        private bool g_bSendCompleted = true;
        private Socket g_pCntSocket = null;
        private IRtpCommunication g_pRtpCommunication = null;
        private bool g_bCleanedUp = false;
        private object g_pCntSendCompleted = new object();

        public RtpClientStream(Socket socket, IRtpCommunication communication)
        {
            if (socket == null || communication == null)
                throw new ArgumentException();
            g_pCntSocket = socket;
            g_pRtpCommunication = communication;
        }

        private void ProcessSend(object sender, SocketAsyncEventArgs e)
        {
            if (e.SocketError == SocketError.Success)
                g_bSendCompleted = true;
            else
            {
                IRtpClientStream g_pCntStream = this;
                g_pCntStream.Close();
            }
        }

        void IRtpClientStream.Write(byte[] buffer)
        {
            IRtpClientStream g_pCntStream = this;
            lock (g_pCntSendCompleted)
            {
                try
                {
                    if (buffer != null && g_bSendCompleted)
                    {
                        if (g_pCntSendEvt == null)
                        {
                            g_pCntSendEvt = new SocketAsyncEventArgs();
                            g_pCntSendEvt.Completed += ProcessSend;
                            g_pCntSendEvt.AcceptSocket = g_pCntSocket;
                        }
                        g_pCntSendEvt.SetBuffer(buffer, 0, buffer.Length);
                        g_bSendCompleted = false;
                        if (!g_pCntSocket.SendAsync(g_pCntSendEvt))
                        {
                            ProcessSend(g_pRtpCommunication, g_pCntSendEvt);
                        }
                    }
                }
                catch
                {
                    g_pCntStream.Close();
                }
            }
        }

        public override bool Equals(object obj)
        {
            RtpClientStream stream = obj as RtpClientStream;
            if (stream == null)
                return false;
            return stream.g_pCntSocket == this.g_pCntSocket;
        }

        public override int GetHashCode()
        {
            return RuntimeHelpers.GetHashCode(g_pCntSocket);
        }

        bool IRtpClientStream.CanWrite
        {
            get
            {
                return g_bSendCompleted;
            }
        }

        void IRtpClientStream.Close()
        {
            lock (g_pCntSendCompleted)
            {
                if (!g_bCleanedUp)
                {
                    if (g_pCntSocket != null)
                        g_pCntSocket.Close();
                    if (g_pRtpCommunication != null && g_pRtpCommunication.Disconnected != null)
                        g_pRtpCommunication.Disconnected(g_pRtpCommunication, this);
                    g_bCleanedUp = true;
                }
            }
        }
    }
}
