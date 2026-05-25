// tset.cpp
// Author: Anurag Choubey

#include <cstring>
#include "tset.h"
#include "arena.h"


int writeset_init(WriteSet* set, char* slice_base){
    if (!set || !slice_base) return -1;

    set->base = slice_base;
    set->count = 0;

    memset(set->base + WS_KEYS_OFFSET, 0, WS_KEYS_BYTES);

    bloom_init2(&set->bf, 1024, 0.01);

    return 0;
}

int writeset_reset(WriteSet* set){
    if (!set) return -1;

    set->count = 0;
    memset(set->base + WS_KEYS_OFFSET, 0, WS_KEYS_BYTES);
    bloom_reset(&set->bf);

    return 0;
}

int writeset_add(WriteSet* set, void* addr, void* src, size_t size){
    if (!set || !addr || !src || size == 0 || size > INLINE_CAP) return -1;
    if (set->count >= WS_SLOTS) return -1;

    void** keys = (void**)(set->base + WS_KEYS_OFFSET);
    keys[set->count] = addr;

    WriteEntry* entries = (WriteEntry*)(set->base + WS_VALUES_OFFSET);
    WriteEntry* e = &entries[set->count];

    e->lock = vlock_ptr(addr);
    e->size = size;
    memcpy(e->buf, src, size);

    bloom_add(&set->bf, &addr, sizeof(addr));

    set->count++;
    return 0;
}

int writeset_lookup(WriteSet* set, void* addr, WriteEntry** entry){
    if (!set || !addr || !entry) return -1;
    *entry = nullptr;

    if (bloom_check(&set->bf, &addr, sizeof(addr)) == 0) return 0;

    void** keys = (void**)(set->base + WS_KEYS_OFFSET);
    WriteEntry* entries = (WriteEntry*)(set->base + WS_VALUES_OFFSET);

    for (uint16_t i = 0; i < set->count; i++){
        if (keys[i] == addr){
            *entry = &entries[i];
            return 1;
        }
    }

    return 0;
}

void** writeset_keys(WriteSet* set){
    if (!set) return nullptr;
    return (void**)(set->base + WS_KEYS_OFFSET);
}

WriteEntry* writeset_values(WriteSet* set){
    if (!set) return nullptr;
    return (WriteEntry*)(set->base + WS_VALUES_OFFSET);
}

int readset_init(ReadSet* set, char* slice_base){
    if (!set || !slice_base) return -1;

    set->base = slice_base;
    set->count = 0;

    return 0;
}

int readset_reset(ReadSet* set){
    if (!set) return -1;

    set->count = 0;

    return 0;
}

int readset_add(ReadSet* set, std::atomic<uint64_t>* lock){
    if (!set || !lock) return -1;
    if (set->count >= RS_MAX) return -1;

    ReadEntry* entries = (ReadEntry*)(set->base + RS_OFFSET);
    entries[set->count].lock = lock;

    set->count++;
    return 0;
}

int readset_validate(ReadSet* set, uint64_t rv){
    if (!set) return -1;

    ReadEntry* entries = (ReadEntry*)(set->base + RS_OFFSET);

    for (uint16_t i = 0; i < set->count; i++){
        std::atomic<uint64_t>* lock = entries[i].lock;

        if (vlock_is_locked(lock)) return 0;
        if (vlock_get_version(lock) > rv) return 0;
    }

    return 1;
}