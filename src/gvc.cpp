#include "gvc.h"

std::atomic<uint64_t> gvc{0};

void gvc_init(){
    gvc = 0;
} 

uint64_t gvc_get(){
    return gvc.load(std::memory_order_relaxed);
}

uint64_t gvc_read(){
    return gvc.load(std::memory_order_acquire);
} 

uint64_t gvc_inc(){
    return gvc.fetch_add(1, std::memory_order_release);
}

