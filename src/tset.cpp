// tset.cpp

#include "tset.h"
#include <cstdlib>

int readset_init(ReadSet* rset) {
    if (!rset) return 0;

    rset->entries = (ReadEntry*) std::malloc(R_INIT * sizeof(ReadEntry));
    if (!rset->entries) return 0;

    rset->count = 0;
    rset->capacity = initial;

    return 1;
}

int writeset_init(WriteSet* wset) {
    if (!wset) return 0;

    size_t initial = 32;
    wset->entries = (WriteEntry*) std::malloc(W_INIT * sizeof(WriteEntry));
    if (!wset->entries) return 0;

    wset->count = 0;
    wset->capacity = initial;

    if (bloom_init2(&wset->bf, 128, 0.01) != 0) return 0;

    return 1;
}

int readset_reset(ReadSet* rset) {
    if (!rset || !rset->entries) return 0;

    rset->count = 0;
    return 1;
}

int writeset_reset(WriteSet* wset) {
    if (!wset || !wset->entries) return 0;

    for (size_t i = 0; i < wset->count; ++i) {
        WriteEntry* e = &wset->entries[i];
        if (e->buffer && e->buffer != e->inlineBuf)
            std::free(e->buffer);
    }

    std::free(wset->entries);
    wset->entries = nullptr;
    wset->count = 0;
    wset->capacity = 0;

    if (bloom_reset(&wset->bf) != 0) // look this up in bloom.c
        return 0;

    return 1;
}

int writeset_lookup(WriteSet* wSet, void* addr, WriteEntry** out){
    if (!out) return 0;
    *out = nullptr;

    if (!wset || !wset->entries || !addr) return 0;
   
    for (size_t i = 0; i < wset->count; ++i){
        WriteEntry* e = &wset->entries[i];
        if (e->varAddr == addr){
            *out = e;
            return 1; 
        }
    }
    return 0;
}


// Write unit tests for 
// defensive props,
// type heterogenity, does it even fucking work?
int writeset_add(WriteSet* wSet, void* addr, size_t size, void* src){
   if (!wset || !wset->entries || 
        !addr || !src || size == 0) return 0;

    int bloom_res = bloom_check(&wset->bf, &addr, sizeof(addr)); // look this up in bloom.c
    if (bloom_res < 0) return 0;

    if (bloom_res == 1){
        WriteEntry* e = nullptr;
        if (writeset_lookup(wSet, addr, size, src) == 1){
            e->size = size;
            
        }
    }



}   

