// gvc.h

#pragma once
#include <atomic>

extern std::atomic<uint64_t> gvc;

void gvc_init();
uint64_t gvc_get();
uint64_t gvc_read();
uint64_t gvc_inc();
