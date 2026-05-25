#include <gtest/gtest.h>
#include <cstring>
#include "tset.h"
#include "vlock.h"
#include "arena.h"

TEST(WriteSet, AddThenLookupRecoversBufferedData) {
    vlock_init();
    char* arena = new char[SLICE_SIZE]();
    WriteSet ws;
    writeset_init(&ws, arena);

    uint64_t vars[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint64_t vals[4] = {111, 222, 333, 444};

    for (int i = 0; i < 4; i++)
        EXPECT_EQ(writeset_add(&ws, &vars[i], &vals[i], sizeof(uint64_t)), 0);

    EXPECT_EQ(ws.count, 4);

    for (int i = 0; i < 4; i++){
        WriteEntry* e = nullptr;
        EXPECT_EQ(writeset_lookup(&ws, &vars[i], &e), 1);
        ASSERT_NE(e, nullptr);
        EXPECT_EQ(e->size, sizeof(uint64_t));

        uint64_t recovered = 0;
        memcpy(&recovered, e->buf, sizeof(uint64_t));
        EXPECT_EQ(recovered, vals[i]);
    }

    uint64_t missing = 0xFF;
    WriteEntry* e = nullptr;
    EXPECT_EQ(writeset_lookup(&ws, &missing, &e), 0);
    EXPECT_EQ(e, nullptr);

    bloom_free(&ws.bf);
    delete[] arena;
}

TEST(ReadSet, ValidateFailsWhenVersionAdvances) {
    vlock_init();
    char* arena = new char[SLICE_SIZE]();
    ReadSet rs;
    readset_init(&rs, arena);

    uint64_t vars[3];
    for (int i = 0; i < 3; i++){
        std::atomic<uint64_t>* lock = vlock_ptr(&vars[i]);
        readset_add(&rs, lock);
    }

    EXPECT_EQ(rs.count, 3);
    EXPECT_EQ(readset_validate(&rs, 100), 1);

    std::atomic<uint64_t>* victim = vlock_ptr(&vars[1]);
    vlock_acquire(victim);
    EXPECT_EQ(readset_validate(&rs, 100), 0);

    vlock_release(victim, 200);
    EXPECT_EQ(readset_validate(&rs, 100), 0);
    EXPECT_EQ(readset_validate(&rs, 200), 1);

    delete[] arena;
}
