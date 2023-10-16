namespace AEVCL.Coroutine
{
    using System;
    using System.Collections;
    using System.IO;
    using System.Threading;

    class Program
    {
        static IEnumerator SuspendCoutine(int from, int to)
        {
            Console.WriteLine("挂起开始 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            yield return Task.Sleep(5000); 
            int num = 0;
            for (int i = from; i <= to; i++)
                num += i;
            yield return num;
            Console.WriteLine("测试打印：{0}", num);
            Console.WriteLine("挂起结束 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
        }

        static IEnumerator AbortCoutine()
        {
            Console.WriteLine("中断 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            yield return Task.Abort(0); // 中断协程
            Console.WriteLine("中断 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
        }

        static IEnumerator SleepCoutine()
        {
            Console.WriteLine("睡眠开始 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            Task.Factory.StartNew(AbortCoutine());
            yield return Task.Sleep(3000); // 睡眠协程
            Console.WriteLine("唤醒 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            Task task = Task.Factory.StartNew(SuspendCoutine(1, 50));
            task.Suspend();
            yield return Task.Sleep(2000);
            task.Resume();
            Console.WriteLine("睡眠结束 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            yield return Path.GetRandomFileName();
        }

        static IEnumerator WhileCoutine()
        {
            Console.WriteLine("死循环 {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            int seed = 0;
            while (true)
            {
                Console.WriteLine((seed = new Random(seed).Next()));
                yield return Task.Sleep(1);
            }
        }

        static IEnumerator WaitAllCoutine()
        {
            yield return Task.Sleep(7000);
        }

        static IEnumerator MainCoutine()
        {
            Thread.CurrentThread.Name = "C/T";
            Console.WriteLine("Main begin {0} {1} {2}", DateTime.Now, Thread.CurrentThread.Name, Task.Current.Elapsed);
            yield return Task.Sleep(500);
            Task sleep = Task.Factory.StartNew(SleepCoutine());
            sleep.Name = "SleepCoutine";
            Task wait = Task.Factory.StartNew(WaitAllCoutine());
            wait.Name = "WaitCoutine";

            sleep.WaitAsync((token, state) =>
            {
                Console.WriteLine("WaitAsync {0} 返回值 {1} {2}", state.Name, state.Result, DateTime.Now);
            });
            yield return Task.WaitAny(new Task[] { sleep }, -1, (index, state) =>
            {
                Console.WriteLine("WaitAny {0} 索引 {2} 返回值 {1} {3}", state.Name, state.Result, index, DateTime.Now);
            });
            yield return Task.WaitAll(new Task[] { sleep, wait }, -1);
            Console.WriteLine("WaitAll {0} 返回值 {1} {2}", sleep.Name, sleep.Result, DateTime.Now);

            Console.WriteLine("Main exit {0} {1}", DateTime.Now, Thread.CurrentThread.Name);
            
            // Task.Factory.StartNew(WhileCoutine());
            // Parallel.ForEach(s, (i) => Console.WriteLine(i));
            //Parallel.For(0, 5000, (index) =>
            //{
            //    Console.WriteLine(index);
            //});
            Console.WriteLine("OK...............");
        }

        [STAThread]
        static void Main(string[] args)
        {
            Console.Title = "基于.NET运行时轻量级协程库（liulilittle & 知否知否）";
            TaskScheduler.Run(MainCoutine(), ApartmentState.MTA);
        }
    }
}
