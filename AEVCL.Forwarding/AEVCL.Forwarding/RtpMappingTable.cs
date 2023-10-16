namespace AEVCL.Forwarding
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using System.Threading;

    public sealed class RtpMappingTable
    {
        private IList<RtpMappingContext> g_pLstMapContext = null;
        private object g_pLstMapCtxSync = new object();
        private Thread g_pThrSendRtpWork = null;

        private static class NativeMethods
        {
            [DllImport("kernel32.dll", SetLastError = false)]
            private static extern IntPtr CreateWaitableTimer(int lpTimerAttributes, bool bManualReset, int lpTimerName);

            [DllImport("kernel32.dll", SetLastError = false)]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool SetWaitableTimer(IntPtr hTimer, ref long pDueTime, int lPeriod, int pfnCompletionRoutine, int lpArgToCompletionRoutine, bool fResume);

            [DllImport("user32.dll", SetLastError = false)]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool MsgWaitForMultipleObjects(uint nCount, ref IntPtr pHandles, bool bWaitAll, int dwMilliseconds, uint dwWakeMask);

            [DllImport("kernel32.dll", SetLastError = false)]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool CloseHandle(IntPtr hWnd);

            private const int NULL = 0;
            private const int QS_TIMER = 0x10;

            public static void Sleep(int us)
            {
                long duetime = -10 * us;
                IntPtr hWaitTimer = CreateWaitableTimer(NULL, true, NULL);
                SetWaitableTimer(hWaitTimer, ref duetime, 0, NULL, NULL, false);
                while (MsgWaitForMultipleObjects(1, ref hWaitTimer, false, Timeout.Infinite, QS_TIMER)) ;
                CloseHandle(hWaitTimer);
            }
        }

        public RtpMappingTable()
        {
            g_pLstMapContext = new List<RtpMappingContext>();
            g_pThrSendRtpWork = new Thread(delegate(object state)
            {
                while (true)
                {
                    if (g_pLstMapContext != null)
                    {
                        lock (g_pLstMapCtxSync)
                        {
                            for (int i = 0; i < g_pLstMapContext.Count; i++)
                            {
                                RtpMappingContext context = g_pLstMapContext[i];
                                context.SendTo();
                            }
                        }
                    }
                    NativeMethods.Sleep(25);
                }
            }) { IsBackground = true, Priority = ThreadPriority.Highest };
            g_pThrSendRtpWork.Start();
        }

        private RtpMappingContext GetContext(RtpClientSession session)
        {
            if (session == null)
                return null;
            return GetContext(session.SimCardNumber, session.ChannelNumber);
        }

        private RtpMappingContext GetContext(string sim, int channels)
        {
            lock (g_pLstMapCtxSync)
            {
                for (int i = 0; i < g_pLstMapContext.Count; i++)
                {
                    RtpMappingContext context = g_pLstMapContext[i];
                    if (context.Equals(sim, channels))
                    {
                        return context;
                    }
                }
                return null;
            }
        }

        private RtpMappingContext GetContext(RTP_HEADER rtp)
        {
            if (rtp == null)
                return null;
            return GetContext(rtp.sim, rtp.channel);
        }

        public void AddStream(RTP_HEADER rtp)
        {
            if (rtp != null)
            {
                lock (g_pLstMapContext)
                {
                    RtpMappingContext context = GetContext(rtp);
                    if (context == null)
                    {
                        context = new RtpMappingContext(rtp.sim, rtp.channel);
                        g_pLstMapContext.Add(context);
                    }
                    context.AddStream(rtp);
                }
            }
        }

        public void AddClient(RtpClientSession session)
        {
            if (session != null)
            {
                lock (g_pLstMapCtxSync)
                {
                    RtpMappingContext context = GetContext(session);
                    if (context == null)
                    {
                        context = new RtpMappingContext(session.SimCardNumber, session.ChannelNumber);
                        g_pLstMapContext.Add(context);
                    }
                    context.AddClient(session);
                }
            }
        }

        public void RemoveClient(IRtpClientStream client)
        {
            if (client != null)
            {
                lock (g_pLstMapCtxSync)
                {
                    for (int i = 0; i < g_pLstMapContext.Count; i++)
                    {
                        RtpMappingContext context = g_pLstMapContext[i];
                        context.RemoveClient(client);
                    }
                }
            }
        }
    }
}
