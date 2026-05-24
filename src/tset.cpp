// tset.cpp
// Author: Anurag Choubey

#include <cstring>
#include "tset.h"


int writeset_init(WriteSet* set, char* slice_base){
    if (!set || !slice_base) return -1;

    set->base = slice_base;
    set->count = 0;

    memset(set->base, 0, WS_SLOTS * sizeof(void*));

    bloom_init2(&set->bf, 1024, 0.01);

    return 0;
}

int writeset_reset(WriteSet* set){
    if (!set) return -1;

    set->count = 0;
    memset(set->base, 0, WS_SLOTS * sizeof(void*));
    bloom_reset(&set->bf);

    return 0;
}