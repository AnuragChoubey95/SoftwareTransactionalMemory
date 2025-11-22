// tset.h

#include "bloom.h"

#define R_INIT 16
#define W_INIT 32
#define INLINE_CAP = 32;

struct ReadEntry{
    void* varAddr;
    std::atomic<uint64_t>* lock;
};

struct WriteEntry{
    void* varAddr;
    std::atomic<uint64_t>* lock;
    size_t size;

    char inlineBuf[INLINE_CAP];
    char* buffer;
    size_t bufCapacity;
};

struct ReadSet {
    ReadEntry* entries;
    size_t count;
    size_t capacity;
};

struct WriteSet {
    WriteEntry* entries;
    size_t count;
    size_t capacity;

    struct bloom bf;   
};

int readset_init(ReadSet* rSet);
int writeset_init(WriteSet* wSet);

int readset_reset(ReadSet* rSet);
int writeset_reset(WriteSet* wSet);

int writeset_add(WriteSet* wSet, void* addr, size_t size, void* src);
int writeset_lookup(WriteSet* wSet, void* addr, WriteEntry** out);




