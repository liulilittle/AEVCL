namespace AEVCL.Forwarding
{
    using System;

    public sealed class RtpControlMessage : EventArgs
    {
        public RtpControlMessage(RtpClientSession session, RTP_HEADER message)
        {
            this.Session = session;
            this.Message = message;
        }

        public RtpClientSession Session
        {
            get;
            private set;
        }

        public RTP_HEADER Message
        {
            get;
            private set;
        }
    }
}
