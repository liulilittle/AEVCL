namespace AEVCL.Forwarding
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Net.Sockets;
    using System.Speech.Synthesis;
    using System.Text;
    using System.Linq;
    using System.Threading;

    public sealed class RtpApplication
    {
        private RtpMappingTable g_pMapTable = null;

        public RtpApplication()
        {
            g_pMapTable = new RtpMappingTable();
            IRtpCommunication rtp = new RtpCommunication();
            rtp.Received += OnReceived;
            rtp.Connected += OnConnected;
            rtp.Controled += OnControled;
            rtp.Disconnected = OnDisconnected;
            rtp.Start("192.168.100.234", 6813); // 192.168.100.112
        }

        private void OnDisconnected(object sender, IRtpClientStream e)
        {
            g_pMapTable.RemoveClient(e);
        }

        private void OnControled(object sender, RtpControlMessage e)
        {
            // TD:OD 
        }

        private void OnConnected(object sender, RtpClientSession e)
        {
            g_pMapTable.AddClient(e);
        }

        static void Main(string[] args)
        {
            RtpApplication application = new RtpApplication();
            Console.ReadLine();
        }

        private void OnReceived(object sender, RTP_HEADER e)
        {
            g_pMapTable.AddStream(e);
        }
    }
}
