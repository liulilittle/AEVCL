namespace AEVCL.Forwarding
{
    using System;
    using System.Collections.Generic;
    using System.Net;
    using System.Net.Sockets;
    using System.Runtime.InteropServices;
    using System.Linq;
    using System.Threading;

    public unsafe sealed class RtpCommunication : IRtpCommunication
    {
        private sealed class PrivateUserToken
        {
            public int recv_size;
            public byte[] recv_buf;
            public int recv_offset;
            public RTP_HEADER recv_rtp;
            public RtpClientSession rtp_client;
        }

        private IList<EventHandler<RtpControlMessage>> g_pCtrlEvt = null;
        private IList<EventHandler<RTP_HEADER>> g_pReceivedEvt = null;
        private IList<EventHandler<RtpClientSession>> g_pConnEvt = null;
        private Socket g_pRtpSocket = null;

        public RtpCommunication()
        {
            g_pCtrlEvt = new List<EventHandler<RtpControlMessage>>();
            g_pReceivedEvt = new List<EventHandler<RTP_HEADER>>();
            g_pConnEvt = new List<EventHandler<RtpClientSession>>();
        }

        private void StartAccept(SocketAsyncEventArgs e)
        {
            if (e == null)
            {
                e = new SocketAsyncEventArgs();
                e.Completed += OnAcceptCompleted;
            }
            e.AcceptSocket = null;
            if (!g_pRtpSocket.AcceptAsync(e))
            {
                ProcessAccept(e);
            }
        }

        private void ProcessAccept(SocketAsyncEventArgs e)
        {
            if (e.SocketError == SocketError.Success)
            {
                Socket client = e.AcceptSocket;
                if (client.Connected)
                {
                    // this.OnConnected(new RtpClientSession(new RtpClientStream(client, this), "123456", 5));
                    StartReceive(client, null);
                }
                StartAccept(e);
            }
        }

        private void StartReceive(Socket client, SocketAsyncEventArgs e)
        {
            try
            {
                if (client == null && e == null)
                    throw new ArgumentNullException();
                if (e == null)
                {
                    e = new SocketAsyncEventArgs();
                    e.AcceptSocket = client;
                    e.Completed += OnReceiveCompleted;
                    e.UserToken = new PrivateUserToken
                    {
                        recv_buf = new byte[1500]
                    };
                }
                if (client == null)
                {
                    client = e.AcceptSocket;
                }
                PrivateUserToken token = e.UserToken as PrivateUserToken;
                if (token.recv_size <= 0)
                {
                    token.recv_size = 30;
                }
                e.SetBuffer(token.recv_buf, token.recv_offset, token.recv_size);
                if (!client.ReceiveAsync(e))
                {
                    ProcessReceive(e);
                }
            }
            catch
            {
                this.OnDisconnected(client);
            }
        }

        private void OnDisconnected(Socket socket)
        {
            if (socket != null)
            {
                IRtpClientStream stream = new RtpClientStream(socket, this);
                this.OnDisconnected(stream);
            }
        }

        private void ProcessReceive(SocketAsyncEventArgs e)
        {
            Socket client = e.AcceptSocket;
            if (e.SocketError == SocketError.Success)
            {
                int size = e.BytesTransferred;
                if (size <= 0)
                {
                    this.OnDisconnected(client);
                }
                PrivateUserToken token = e.UserToken as PrivateUserToken;
                if (token.recv_rtp == null)
                {
                    RTP_HEADER header = RTP_HEADER.Parse((byte*)Marshal.UnsafeAddrOfPinnedArrayElement(e.Buffer, 0), size);
                    if (header != null)
                    {
                        token.recv_offset = header.body.size;
                        token.recv_rtp = header;
                        token.recv_size = header.size - token.recv_offset;
                        this.OnProcessEvent(client, token);
                    }
                }
                else
                {
                    RTP_HEADER header = token.recv_rtp;
                    RTP_BODY body = header.body;
                    RTP_BODY_BUFFER buffer = body.buf;
                    buffer.Write(e.Buffer, size);
                    token.recv_offset += size;
                    //
                    this.OnProcessEvent(client, token);
                }
                StartReceive(client, e);
            }
            else
            {
                this.OnDisconnected(client);
            }
        }

        private void OnProcessEvent(Socket client, PrivateUserToken token) // 基于事件驱动的状态机
        {
            if (token.recv_offset >= token.recv_size)
            {
                RTP_HEADER header = token.recv_rtp;
                token.recv_size = 0;
                token.recv_rtp = null;
                token.recv_offset = 0;
                //
                if (header.type == 0x0F)
                {
                    IRtpClientStream g_pCntStream = new RtpClientStream(client, this);
                    token.rtp_client = new RtpClientSession(g_pCntStream, header.sim, header.channel);
                    this.OnConnected(token.rtp_client);
                }
                else if (header.type == 0x08)
                {
                    RtpControlMessage message = new RtpControlMessage(token.rtp_client, header);
                    this.OnControled(message);
                }
                else
                {
                    this.OnReceived(header);
                }
            }
        }

        private void OnReceiveCompleted(object sender, SocketAsyncEventArgs e)
        {
            ProcessReceive(e);
        }

        private void OnAcceptCompleted(object sender, SocketAsyncEventArgs e)
        {
            ProcessAccept(e);
        }

        public void Stop()
        {
            
        }

        void IRtpCommunication.Start(string address, int port)
        {
            lock (this)
            {
                if (g_pRtpSocket == null)
                {
                    g_pRtpSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                    g_pRtpSocket.Bind(new IPEndPoint(IPAddress.Parse(address), port));
                    g_pRtpSocket.Listen(300);
                    StartAccept(null);
                }
            }
        }

        void IRtpCommunication.Stop()
        {

        }

        private void OnControled(RtpControlMessage e)
        {
            if (g_pCtrlEvt != null)
            {
                for (int i = 0; i < g_pCtrlEvt.Count; i++)
                {
                    EventHandler<RtpControlMessage> g_pEvt = g_pCtrlEvt[i];
                    g_pEvt(this, e);
                }
            }
        }

        private void OnConnected(RtpClientSession e)
        {
            if (g_pConnEvt != null)
            {
                for (int i = 0; i < g_pConnEvt.Count; i++)
                {
                    EventHandler<RtpClientSession> g_pEvt = g_pConnEvt[i];
                    g_pEvt(this, e);
                }
            }
        }

        private void AddEvent<T>(IList<EventHandler<T>> x, EventHandler<T> y) where T : EventArgs
        {
            if (g_pReceivedEvt == null)
            {
                throw new InvalidOperationException();
            }
            if (x == null)
            {
                throw new ArgumentNullException();
            }
            EventHandler<T> g_pEvt = x.FirstOrDefault(i => i.Method != y.Method);
            if (g_pEvt == null)
            {
                x.Add(y);
            }
        }

        private void RemoveEvent<T>(IList<EventHandler<T>> x, EventHandler<T> y) where T : EventArgs
        {
            if (x == null)
            {
                throw new InvalidOperationException();
            }
            if (y == null)
            {
                throw new ArgumentNullException();
            }
            EventHandler<T> g_pEvt = x.FirstOrDefault(i => i.Method != y.Method);
            if (g_pEvt != null)
            {
                x.Remove(g_pEvt);
            }
        }

        private void OnReceived(RTP_HEADER e)
        {
            if (g_pReceivedEvt != null)
            {
                for (int i = 0; i < g_pReceivedEvt.Count; i++)
                {
                    EventHandler<RTP_HEADER> g_pEvt = g_pReceivedEvt[i];
                    g_pEvt(this, e);
                }
            }
        }

        private void OnDisconnected(IRtpClientStream e)
        {
            IRtpCommunication communication = this;
            if (e != null)
            {
                e.Close();
            }
            if (communication.Disconnected != null)
            {
                communication.Disconnected(this, e);
            }
        }

        event EventHandler<RTP_HEADER> IRtpCommunication.Received
        {
            add
            {
                AddEvent(g_pReceivedEvt, value);
            }
            remove
            {
                RemoveEvent(g_pReceivedEvt, value);
            }
        }

        event EventHandler<RtpClientSession> IRtpCommunication.Connected
        {
            add
            {
                AddEvent(g_pConnEvt, value);
            }
            remove
            {
                RemoveEvent(g_pConnEvt, value);
            }
        }

        event EventHandler<RtpControlMessage> IRtpCommunication.Controled
        {
            add
            {
                AddEvent(g_pCtrlEvt, value);
            }
            remove
            {
                RemoveEvent(g_pCtrlEvt, value);
            }
        }

        Action<object, IRtpClientStream> IRtpCommunication.Disconnected
        {
            get;
            set;
        }
    }
}
