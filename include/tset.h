// tset.h
// Author: Anurag Choubey

#pragma once 

#include <atomic>
#include "bloom.h"
#include <cstddef>
#include <cstdint>

#define WS_SLOTS 2048
#define RS_MAX 2048
#define INLINE_CAP 128
#define BLOOM_BYTES 1224 //(1024 entries, 1% FPR, rounded to 8-byte boundary)

struct WriteEntry{
    std::atomic<uint64_t>* lock;
    size_t size;
    char buf[INLINE_CAP];
};

struct WriteSet{
    char* base;
    uint16_t count;
    struct bloom bf;
};

struct ReadEntry{
    std::atomic<uint64_t>* lock;
};

struct ReadSet{
    char* base;
    uint16_t count;
};

int writeset_init(WriteSet* set, char* slice_base);
int writeset_reset(WriteSet* set);
int writeset_add(WriteSet* set, void* addr, void* src, size_t size);
int writeset_lookup(WriteSet* set, void* addr, WriteEntry** entry);
void** writeset_keys(WriteSet* set);
WriteEntry* writeset_values(WriteSet* set);

int readset_init(ReadSet* set, char* slice_base);
int readset_reset(ReadSet* set);
int readset_add(ReadSet* set, std::atomic<uint64_t>* lock);
int readset_validate(ReadSet* set, uint64_t rv);



