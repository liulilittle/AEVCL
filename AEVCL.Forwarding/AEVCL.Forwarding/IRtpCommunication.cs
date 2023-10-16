namespace AEVCL.Forwarding
{
    using System;
    using System.Net.Sockets;

    public partial interface IRtpCommunication
    {
        void Start(string address, int port);
        void Stop();
        event EventHandler<RTP_HEADER> Received;
        event EventHandler<RtpClientSession> Connected;
        event EventHandler<RtpControlMessage> Controled;
        Action<object, IRtpClientStream> Disconnected
        {
            get;
            set;
        }
    }
}
