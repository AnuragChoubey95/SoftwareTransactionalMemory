#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <set>
#include <cstdint>
#include <cstring>
#include "arena.h"

TEST(Arena, ConcurrentRegistrationProducesUniqueNonOverlappingSlices) {
    arena_destroy();
    ASSERT_EQ(arena_init(0), 0);

    std::vector<std::thread> threads;
    std::vector<char*> results(MAX_THREADS, nullptr);

    for (int i = 0; i < MAX_THREADS; i++){
        threads.emplace_back([i, &results](){
            results[i] = arena_register_thread();
        });
    }
    for (auto& t : threads) t.join();

    std::set<char*> unique;
    for (int i = 0; i < MAX_THREADS; i++){
        ASSERT_NE(results[i], nullptr);
        ASSERT_GE(results[i], arena_base);
        ASSERT_LT(results[i], arena_base + ARENA_SIZE);
        EXPECT_EQ(((uintptr_t)results[i] - (uintptr_t)arena_base) % SLICE_SIZE, 0u);
        unique.insert(results[i]);
    }
    EXPECT_EQ(unique.size(), (size_t)MAX_THREADS);

    char* overflow = arena_register_thread();
    EXPECT_EQ(overflow, nullptr);
    EXPECT_EQ(arena_slot_count(), MAX_THREADS);

    arena_destroy();
}

TEST(Arena, LifecycleIsolationAndReuse) {
    arena_destroy();
    EXPECT_EQ(arena_register_thread(), nullptr);

    arena_destroy();

    ASSERT_EQ(arena_init(0), 0);
    EXPECT_EQ(arena_init(0), -1);
    EXPECT_EQ(arena_init(1), -1);

    std::vector<char*> slices;
    for (int i = 0; i < MAX_THREADS; i++){
        char* s = arena_register_thread();
        ASSERT_NE(s, nullptr);
        slices.push_back(s);
    }

    for (int i = 0; i < MAX_THREADS; i++){
        memset(slices[i], (char)(i + 1), SLICE_RAW);
    }
    for (int i = 0; i < MAX_THREADS; i++){
        for (size_t off = 0; off < SLICE_RAW; off += 4096){
            EXPECT_EQ((unsigned char)slices[i][off], (unsigned char)(i + 1));
        }
        EXPECT_EQ((unsigned char)slices[i][SLICE_RAW - 1], (unsigned char)(i + 1));
    }

    arena_destroy();
    EXPECT_EQ(arena_slot_count(), 0);
    EXPECT_EQ(arena_register_thread(), nullptr);

    ASSERT_EQ(arena_init(0), 0);
    EXPECT_EQ(arena_slot_count(), 0);

    char* fresh = arena_register_thread();
    ASSERT_NE(fresh, nullptr);
    EXPECT_EQ(fresh, arena_base);

    std::vector<std::thread> threads;
    std::atomic<int> nulls{0};
    for (int i = 0; i < 200; i++){
        threads.emplace_back([&nulls](){
            if (arena_register_thread() == nullptr)
                nulls.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads) t.join();

    EXPECT_EQ(arena_slot_count(), MAX_THREADS);
    EXPECT_GE(nulls.load(), 200 - (MAX_THREADS - 1));

    arena_destroy();
}
