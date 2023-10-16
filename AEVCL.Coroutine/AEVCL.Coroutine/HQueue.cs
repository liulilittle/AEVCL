namespace AEVCL.Coroutine
{
    using System;
    using System.Collections;
    using System.Collections.Generic;

    public sealed class HQueue<T>
    {
        private T[] g_pBuffer = null;
        private int g_nCapacity = 0;
        private int g_nOffset = -1;

        public HQueue(int capacity = 1000)
        {
            if (capacity <= 0)
            {
                throw new ArgumentOutOfRangeException();
            }
            g_nCapacity = capacity;
            g_pBuffer = new T[capacity];
        }

        public int Capacity
        {
            get
            {
                return g_nCapacity;
            }
        }

        public int Count
        {
            get
            {
                return g_nCapacity - g_nOffset;
            }
        }

        public int Position
        {
            get
            {
                return g_nOffset;
            }
        }

        public void Enqueue(T value)
        {
            lock (this)
            {
                int offset = g_nOffset + 1;
                //if (offset >= g_nCapacity)
                //    throw new ArgumentOutOfRangeException();
                //g_pBuffer[offset] = value;
                //g_nOffset = offset;
                if (offset < g_nCapacity)
                {
                    g_pBuffer[offset] = value;
                    g_nOffset = offset;
                }
            }
        }

        public T Dequeue()
        {
            lock (this)
            {
                if (g_nOffset < 0)
                    return default(T);
                T value = g_pBuffer[g_nOffset];
                g_pBuffer[g_nOffset] = default(T);
                g_nOffset--;
                return value;
            }
        }
    }
}
