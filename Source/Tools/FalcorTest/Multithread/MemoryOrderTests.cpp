/**
 * Unit tests for C++11 atomic memory order semantics.
 * Covers memory_order_relaxed, memory_order_acquire/release, and memory_order_seq_cst.
 * See doc/Falcor/Test.md and memory order documentation for background.
 */
#include "Testing/UnitTest.h"

#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace Falcor
{

// --- memory_order_relaxed: 简单计数器，只保证原子性，不保证跨线程顺序 ---
CPU_TEST(MemoryOrder_RelaxedCounter)
{
    std::atomic<int> counter(0);
    const int kIncrementsPerThread = 100000;
    const int kNumThreads = 10;

    auto incrementRelaxed = [&counter, kIncrementsPerThread]()
    {
        for (int i = 0; i < kIncrementsPerThread; ++i)
        {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);
    for (int i = 0; i < kNumThreads; ++i)
    {
        threads.emplace_back(incrementRelaxed);
    }
    for (auto& t : threads)
    {
        t.join();
    }

    const int expected = kNumThreads * kIncrementsPerThread;
    EXPECT_EQ(counter.load(std::memory_order_relaxed), expected);
}

// --- memory_order_acquire/release: 生产者-消费者，保证 data 在 ready_flag 之后可见 ---
CPU_TEST(MemoryOrder_AcquireRelease)
{
    std::string data;
    std::atomic<bool> readyFlag(false);

    std::thread producer([&]()
    {
        data = "Hello, C++ Memory Order!";
        readyFlag.store(true, std::memory_order_release);
    });

    std::thread consumer([&]()
    {
        while (!readyFlag.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
        EXPECT_EQ(data, "Hello, C++ Memory Order!");
    });

    producer.join();
    consumer.join();
}

// --- memory_order_seq_cst: 全局一致顺序，观察者看到 x,y 为 true 时 a,b 必已写入 ---
CPU_TEST(MemoryOrder_SeqCst)
{
    std::atomic<bool> x(false);
    std::atomic<bool> y(false);
    int a = 0;
    int b = 0;

    std::thread t1([&]()
    {
        a = 1;
        x.store(true, std::memory_order_seq_cst);
    });

    std::thread t2([&]()
    {
        b = 1;
        y.store(true, std::memory_order_seq_cst);
    });

    std::thread t3([&]()
    {
        while (!x.load(std::memory_order_seq_cst) || !y.load(std::memory_order_seq_cst))
        {
            std::this_thread::yield();
        }
        EXPECT_EQ(a, 1);
        EXPECT_EQ(b, 1);
    });

    t1.join();
    t2.join();
    t3.join();
}

} // namespace Falcor
