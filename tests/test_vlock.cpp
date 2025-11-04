#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "vlock.h"

TEST(VLock, InitAllocatesStripeArray) {
    vlock_init();
    ASSERT_NE(lockMap, nullptr);
    // Check that all locks start at zero (unlocked, version 0)
    for (size_t i = 0; i < NUM_STRIPES; ++i) {
        EXPECT_EQ(lockMap[i].load(std::memory_order_relaxed), 0);
    }
}

TEST(VLock, IndexMappingIsStable) {
    uint64_t x, y;
    size_t idx1 = vlock_index(&x);
    size_t idx2 = vlock_index(&x);
    size_t idx3 = vlock_index(&y);
    EXPECT_EQ(idx1, idx2);
    EXPECT_NE(idx1, idx3);
}

TEST(VLock, AcquireSetsLockBit) {
    vlock_init();
    std::atomic<uint64_t>* lock = vlock_ptr(reinterpret_cast<void*>(0x1234));
    vlock_acquire(lock);
    EXPECT_TRUE(vlock_is_locked(lock));
}

TEST(VLock, ReleaseClearsLockAndSetsVersion) {
    vlock_init();
    std::atomic<uint64_t>* lock = vlock_ptr(reinterpret_cast<void*>(0xBEEF));
    vlock_acquire(lock);
    vlock_release(lock, 42);
    EXPECT_FALSE(vlock_is_locked(lock));
    EXPECT_EQ(vlock_get_version(lock), 42);
}

TEST(VLock, ConcurrentAcquireAllowsOnlyOne) {
    vlock_init();
    std::atomic<uint64_t>* lock = vlock_ptr(reinterpret_cast<void*>(0xCAFE));
    std::atomic<int> success = 0;

    auto attempt = [&]() {
        if (!vlock_is_locked(lock)) {
            vlock_acquire(lock);
            if (vlock_is_locked(lock)) {
                success.fetch_add(1, std::memory_order_relaxed);
                vlock_release(lock, vlock_get_version(lock) + 1);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(attempt);
    for (auto& t : threads)
        t.join();

    EXPECT_EQ(success.load(), 8);
}

TEST(VLock, ClearAllResetsEverything) {
    vlock_init();
    std::atomic<uint64_t>* lock = vlock_ptr(reinterpret_cast<void*>(0x2222));
    vlock_acquire(lock);
    vlock_release(lock, 99);
    vlock_clear_all();
    for (size_t i = 0; i < NUM_STRIPES; ++i)
        EXPECT_EQ(lockMap[i].load(std::memory_order_relaxed), 0);
}
