// Transaction Macros Definition
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet
// Provides macros to simplify transaction management in STM systems.

// Include the Transaction class definition.
#include "/home/ac2255/phpc/weeks_11_to_15/revised_final/new/src/transaction.h"  

// Define a macro to start a transaction with specified statistics and collector.
#define START_TX(tr, collectStats, statCollector) \
    Transaction tr(std::this_thread::get_id(), collectStats, statCollector); \
    do { try  // Begin a transaction block with a try statement to handle exceptions.

// Define a macro to end a transaction and handle exceptions silently.
#define END_TX(tr) \
    catch (const std::exception& e) {} \
    } while (!tr.commitTransaction());  // Continue the transaction until it successfully commits.
