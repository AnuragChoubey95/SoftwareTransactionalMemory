// vlock.h

// Author: Anurag Choubey

#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

constexpr size_t NUM_STRIPES = 1 << 20;
constexpr size_t LOCK_SIZE = sizeof(uint64_t);

extern std::atomic<uint64_t>* lockMap;

void vlock_acquire(std::atomic<uint64_t>* lock);
void vlock_release(std::atomic<uint64_t>* lock, uint64_t new_version);
size_t vlock_index(void* addr);
std::atomic<uint64_t>* vlock_ptr(void* addr);
uint64_t vlock_get_version(const std::atomic<uint64_t>* lock);
bool vlock_is_locked(const std::atomic<uint64_t>* lock);
void vlock_clear_all();
void vlock_init();
void vlock_reset();