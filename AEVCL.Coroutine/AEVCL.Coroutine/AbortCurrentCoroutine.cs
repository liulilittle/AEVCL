namespace AEVCL.Coroutine
{
    using System.Threading;

    sealed class AbortCurrentCoroutine
    {
        private int g_exit = 0;

        public AbortCurrentCoroutine(int exit)
        {
            g_exit = exit;
        }

        public void Handle()
        {
            if (g_exit != 0)
                throw new CoroutineAbortException();
        }
    }
}
