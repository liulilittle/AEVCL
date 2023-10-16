namespace AEVCL.Forwarding
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Net.Sockets;
    using System.Text;

    public sealed class RtpMappingContext
    {
        private object g_pLstCntSessionSync = new object();
        private string g_szTSimNum = null;
        private int g_nTChannelNum = 0;
        private IList<RtpClientSession> g_pLstCntSession = null;

        public RtpMappingContext(string sim, int channels)
        {
            g_pLstCntSession = new List<RtpClientSession>();
            g_szTSimNum = sim;
            g_nTChannelNum = channels;
        }

        public bool Equals(string sim, int channels)
        {
            return (sim == g_szTSimNum) && (channels == g_nTChannelNum);
        }

        private RtpClientSession GetClient(string sim, int channels)
        {
            lock (g_pLstCntSessionSync)
            {
                for (int i = 0; i < g_pLstCntSession.Count; i++)
                {
                    RtpClientSession client = g_pLstCntSession[i];
                    if (client.Equals(sim, channels))
                        return client;
                }
                return null;
            }
        }

        public void AddClient(RtpClientSession session)
        {
            lock (g_pLstCntSessionSync)
            {
                //RtpClientSession client = GetClient(session.SimCardNumber, session.ChannelNumber);
                //if (client == null)
                //{
                //    g_pLstCntSession.Add(session);
                //}
                g_pLstCntSession.Add(session);
            }
        }

        public void AddStream(RTP_HEADER rtp)
        {
            RTP_BODY body = rtp.body;
            RTP_BODY_BUFFER data = body.buf;
            for (int i = 0; i < g_pLstCntSession.Count; i++)
            {
                RtpClientSession session = g_pLstCntSession[i];
                session.Wrtie(data.ToArray());
            }
        }

        public void SendTo()
        {
            if (g_pLstCntSession != null)
            {
                lock (g_pLstCntSessionSync)
                {
                    for (int i = 0; i < g_pLstCntSession.Count; i++)
                    {
                        RtpClientSession client = g_pLstCntSession[i];
                        client.SendTo();
                    }
                }
            }
        }

        public void RemoveClient(IRtpClientStream client)
        {
            lock (g_pLstCntSessionSync)
            {
                for (int i = 0; i < g_pLstCntSession.Count; i++)
                {
                    RtpClientSession session = g_pLstCntSession[i];
                    if (session.Stream == client)
                    {
                        g_pLstCntSession.RemoveAt(i);
                        break;
                    }
                }
            }
        }
    }
}
