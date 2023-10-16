namespace AEVCL.Forwarding
{
    using System;
    using System.Net.Sockets;

    public interface IRtpClientStream
    {
        bool CanWrite
        {
            get;
        }

        void Close();

        void Write(byte[] buffer);
    }
}
