// STATS_H
// Defines the Statistics class which provides atomic operations for tracking transaction-related statistics.
// This class uses atomic integers to ensure thread-safe increments of counters for transaction aborts,
// initializations, and restarts.
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#ifndef STATS_H
#define STATS_H

#include <atomic>  

class Statistics {
public:
    std::atomic<int> num_aborts;    // Counter for the number of transaction aborts
    std::atomic<int> num_inits;     // Counter for the number of transaction initializations
    std::atomic<int> num_restarts;  // Counter for the number of transaction restarts

    // Constructor initializes all counters to zero
    Statistics() : num_aborts(0), num_inits(0), num_restarts(0) {}

    // Increment the abort counter by one in a thread-safe manner
    void increment_aborts() {
        num_aborts.fetch_add(1, std::memory_order_relaxed);
    }

    // Increment the initialization counter by one in a thread-safe manner
    void increment_inits() {
        num_inits.fetch_add(1, std::memory_order_relaxed);
    }

    // Increment the restart counter by one in a thread-safe manner
    void increment_restarts() {
        num_restarts.fetch_add(1, std::memory_order_relaxed);
    }
};

#endif // STATS_H
