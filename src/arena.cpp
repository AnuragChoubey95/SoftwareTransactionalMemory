// arena.cpp
// Author: Anurag Choubey

#include <sys/mman.h>
#include <cstddef>
#include <cstdint>
#include "arena.h"

#ifdef __APPLE__
#include <mach/vm_statistics.h>
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

char* arena_base = nullptr;
std::atomic<uint32_t> arena_next_slot{0};
int arena_uses_huge_pages = 0;

int arena_init(int try_huge_pages){
    if (arena_base != nullptr) return -1;

    void* mem = MAP_FAILED;

    if (try_huge_pages){
#if defined(MAP_HUGETLB)
        mem = mmap(nullptr, ARENA_SIZE,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                   -1, 0);
        if (mem != MAP_FAILED){
            arena_uses_huge_pages = 1;
        }
#elif defined(VM_FLAGS_SUPERPAGE_SIZE_2MB)
        mem = mmap(nullptr, ARENA_SIZE,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
        if (mem != MAP_FAILED){
            arena_uses_huge_pages = 1;
        }
#endif
    }

    if (mem == MAP_FAILED){
        mem = mmap(nullptr, ARENA_SIZE,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);
        if (mem == MAP_FAILED) return -1;

#ifdef MADV_HUGEPAGE
        if (try_huge_pages){
            madvise(mem, ARENA_SIZE, MADV_HUGEPAGE);
        }
#endif
        arena_uses_huge_pages = 0;
    }

    arena_base = (char*)mem;
    arena_next_slot.store(0, std::memory_order_release);
    return 0;
}

void arena_destroy(){
    if (arena_base == nullptr) return;

    munmap(arena_base, ARENA_SIZE);

    arena_base = nullptr;
    arena_next_slot.store(0, std::memory_order_relaxed);
    arena_uses_huge_pages = 0;
}

char* arena_register_thread(){
    if (arena_base == nullptr) return nullptr;

    uint32_t slot = arena_next_slot.fetch_add(1, std::memory_order_acq_rel);

    if (slot >= MAX_THREADS){
        arena_next_slot.fetch_sub(1, std::memory_order_relaxed);
        return nullptr;
    }

    return arena_base + ((size_t)slot * SLICE_SIZE);
}

int arena_slot_count(){
    uint32_t n = arena_next_slot.load(std::memory_order_acquire);
    if (n > MAX_THREADS) return MAX_THREADS;
    return (int)n;
}
