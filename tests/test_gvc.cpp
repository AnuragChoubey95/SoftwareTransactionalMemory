#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "gvc.h"

TEST(GVC, InitResetsToZero) {
    gvc_init();
    EXPECT_EQ(gvc_get(), 0);
    EXPECT_EQ(gvc_read(), 0);
}

TEST(GVC, IncrementIncreasesValue) {
    gvc_init();
    uint64_t v1 = gvc_inc();
    uint64_t v2 = gvc_inc();
    EXPECT_LT(v1, v2);
    EXPECT_EQ(gvc_get(), 2);
}

TEST(GVC, RelaxedVsAcquireRelease) {
    gvc_init();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    gvc_inc();
    EXPECT_EQ(gvc_read(), 1);
}

TEST(GVC, ConcurrentIncrementsAreAtomic) {
    gvc_init();
    const int num_threads = 8;
    const int iters = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iters; ++j)
                gvc_inc();
        });
    }
    for (auto &t : threads) t.join();
    EXPECT_EQ(gvc_get(), num_threads * iters);
}

TEST(GVC, AcquireReleaseSemantics) {
    gvc_init();
    std::atomic<bool> done = false;
    std::thread writer([&]() {
        gvc_inc(); // release
        done.store(true, std::memory_order_release);
    });
    std::thread reader([&]() {
        while (!done.load(std::memory_order_acquire)) {}
        EXPECT_GE(gvc_read(), 1);
    });
    writer.join();
    reader.join();
}
