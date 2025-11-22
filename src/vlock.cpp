// vlock.cpp

#include "vlock.h"

std::atomic<uint64_t>* lockMap = new std::atomic<uint64_t>[NUM_STRIPES]{}; // remember to cleanup upon program exit!!

void vlock_acquire(std::atomic<uint64_t>* lock){
    uint64_t curr = lock->load(std::memory_order_acquire);

    while(true){
        if (curr & 1ULL){
            curr = lock->load(std::memory_order_acquire);
            continue;
        }
        uint64_t desired = curr | 1ULL;
        if (lock->compare_exchange_weak(curr, desired,
                                        std::memory_order_acquire,
                                        std::memory_order_relaxed)){
            break;
        }
    }  
}

void vlock_release(std::atomic<uint64_t>* lock, uint64_t new_version){
    uint64_t new_val = new_version << 1;
    lock->store(new_val, std::memory_order_release);
}

size_t vlock_index(void* addr){
    size_t idx = ((uintptr_t)addr >> 3) & ((1ULL << 20) - 1);
    return idx;
}

std::atomic<uint64_t>* vlock_ptr(void* addr) {
    return &lockMap[vlock_index(addr)];
}

uint64_t vlock_get_version(const std::atomic<uint64_t>* lock) {
    return lock->load(std::memory_order_relaxed) >> 1;
}

bool vlock_is_locked(const std::atomic<uint64_t>* lock) {
    return (lock->load(std::memory_order_relaxed) & 1ULL);
}

void vlock_clear_all() {
    for (size_t i = 0; i < NUM_STRIPES; ++i)
        lockMap[i].store(0, std::memory_order_relaxed);
}

void vlock_init()  { vlock_clear_all(); }

void vlock_reset() { vlock_clear_all(); }
