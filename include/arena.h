// arena.h
// Author: Anurag Choubey

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

#define MAX_THREADS    64
#define PAGE_SIZE      4096 
#define HUGE_PAGE_SIZE (2 * 1024 * 1024)

#define WS_KEYS_OFFSET   0
#define WS_KEYS_BYTES    (2048 * sizeof(void*))

#define WS_VALUES_OFFSET (WS_KEYS_OFFSET + WS_KEYS_BYTES)
#define WS_VALUES_BYTES  (2048 * 144)

#define RS_OFFSET        (WS_VALUES_OFFSET + WS_VALUES_BYTES)
#define RS_BYTES         (2048 * sizeof(void*))

#define BLOOM_OFFSET     (RS_OFFSET + RS_BYTES)
#define BLOOM_BYTES_SZ   1224

#define SLICE_RAW        (BLOOM_OFFSET + BLOOM_BYTES_SZ)
#define SLICE_SIZE       (((SLICE_RAW + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE)
#define ARENA_RAW        (MAX_THREADS * SLICE_SIZE)
#define ARENA_SIZE       (((ARENA_RAW + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE) * HUGE_PAGE_SIZE)

extern char* arena_base;
extern std::atomic<uint32_t> arena_next_slot;
extern int arena_uses_huge_pages;

int   arena_init(int try_huge_pages);
void  arena_destroy();
char* arena_register_thread();
int   arena_slot_count();
