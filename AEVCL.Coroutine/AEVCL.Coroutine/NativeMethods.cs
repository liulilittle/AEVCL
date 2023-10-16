namespace AEVCL.Coroutine
{
    using System.Runtime.InteropServices;
    using System.Threading;

    public static class NativeMethods
    {
        [DllImport("kernel32.dll", SetLastError = false)]
        private static extern int CreateWaitableTimer(int lpTimerAttributes, bool bManualReset, int lpTimerName);

        [DllImport("kernel32.dll", SetLastError = false)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetWaitableTimer(int hTimer, ref long pDueTime, int lPeriod, int pfnCompletionRoutine, int lpArgToCompletionRoutine, bool fResume);

        [DllImport("user32.dll", SetLastError = false)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool MsgWaitForMultipleObjects(uint nCount, ref int pHandles, bool bWaitAll, int dwMilliseconds, uint dwWakeMask);

        [DllImport("winmm", SetLastError = false)]
        private static extern uint timeGetTime();

        [DllImport("winmm", SetLastError = false)]
        private static extern void timeBeginPeriod(int t);

        [DllImport("winmm", SetLastError = false)]
        private static extern void timeEndPeriod(int t);

        private const int NULL = 0;
        private const int QS_TIMER = 0x10;

        /// <summary>
        /// 延迟代码执行时间
        /// </summary>
        /// <param name="us">以微秒作为延迟单位，避免纳秒级对处理器与内核的负载过高</param>
        public static void usleep(int us)
        {
            NativeMethods.WaitForMultipleObjects(us * 10);
        }

        /// <summary>
        /// 等待内核发出信号
        /// </summary>
        /// <param name="ns">以100纳秒为一基准单位</param>
        private static void WaitForMultipleObjects(int ns)
        {
            long duetime = -1 * ns;
            int hWaitTimer = NativeMethods.CreateWaitableTimer(NativeMethods.NULL, true, NativeMethods.NULL);
            NativeMethods.SetWaitableTimer(hWaitTimer, ref duetime, 0, NativeMethods.NULL, NativeMethods.NULL, false);
            while (NativeMethods.MsgWaitForMultipleObjects(1, ref hWaitTimer, false, Timeout.Infinite, NativeMethods.QS_TIMER)) ;
        }

        /// <summary>
        /// 等待内核发出信号
        /// </summary>
        /// <param name="ns">以100纳秒为一基准单位</param>
        public static void nanosleep(int ns)
        {
            NativeMethods.WaitForMultipleObjects(ns);
        }

        /// <summary>
        /// 获取高精度的时间
        /// </summary>
        /// <returns></returns>
        public static uint time()
        {
            uint time = 0;
            NativeMethods.timeBeginPeriod(1);
            time = NativeMethods.timeGetTime();
            NativeMethods.timeEndPeriod(1);
            return time;
        }
    }
}
