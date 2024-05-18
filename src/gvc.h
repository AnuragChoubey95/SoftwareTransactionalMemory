// GLOBAL_VERSION_CLOCK_H
// Defines operations for managing a global version clock using atomic operations,
// which ensures thread-safe incrementing and fetching of the global version number
// as used in the TL2 algorithm. This is crucial for maintaining
// the consistency and order of transactions in concurrent environments.
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#ifndef GLOBAL_VERSION_CLOCK_H
#define GLOBAL_VERSION_CLOCK_H

#include <atomic>  

// Define a global atomic variable for the version clock.
std::atomic<int64_t> globalVersionClock = 0;  // Initialize the global version clock to 0.

// Function to atomically increment the global version clock and return the new value.
int64_t increment_and_fetch_GVC() {
    int64_t expected = globalVersionClock.load();  // Load the current value of the global version clock.
    int64_t desired;
    do {
        desired = expected + 1;  // Calculate the desired new value by adding one.
        // Atomically compare and set the global version clock.
    } while (!globalVersionClock.compare_exchange_strong(expected, desired));
    return desired;  
}

// Function to fetch the current value of the global version clock.
int64_t get_GVC() {
    return globalVersionClock.load();  // Return the current value of the global version clock.
}

#endif // GLOBAL_VERSION_CLOCK_H
