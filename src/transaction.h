// Transaction.h provides a framework for managing software transactional memory (STM).
// It defines the Transaction class which supports methods for starting, executing, 
// and managing the lifecycle of transactions involving STM variables.
// The class manages read-write operations with concurrency control.

// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <set>
#include <map>
#include <any>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <stdexcept>

#include "gvc.h"
#include "stmVar.h"
#include "stats.h"
#include "custom_types/ll.h"

#include "gvc.h"          // Include the global version clock utilities.
#include "stmVar.h"       // Include the STM variable definitions.
#include "stats.h"        // Include the statistics tracking utilities.
#include "custom_types/ll.h"  // Include custom linked list type definitions.

class Transaction {
private:
    std::thread::id transactionId;  // Unique identifier for the transaction based on thread ID.
    std::set<StmVariableBase *> readLog;  // Set of variables read during the transaction.
    std::map<StmVariableBase *, std::any *> writeLog;  // Map of variables written to during the transaction.
    int64_t readVersion;  // The version of the read view of the data.
    int64_t writeVersion;  // The version of the write view of the data.
    bool readOnly;  // Flag to indicate if the transaction is read-only.
    int status;  // Current status of the transaction, 0 for active, -1 for aborted.
    bool collectStats;  // Flag to determine if statistics should be collected.
    Statistics& statCollector;  // Reference to statistics collector to record transaction metrics.

    // Enable write operations on the transaction by setting readOnly to false.
    inline void writeModeOn() {
        readOnly = false;
    }

    // Abort the transaction, releasing all locks and clearing logs.
    inline void abortTransaction() {

        // Loop through each entry in the writeLog to release locks and deallocate memory
        for (auto& var : writeLog) {
            var.first->releaseLock();   // Release lock on each variable
            delete var.second;          // Delete dynamically allocated memory to prevent leaks
            var.second = nullptr;       // Nullify pointer to indicate it is no longer valid
        }

        // Clear readLog and writeLog to reset transaction state
        readLog.clear();               // Clear all entries from readLog
        writeLog.clear();              // Clear all entries from writeLog

        // If statistics are being collected, record this abort
        if(collectStats) statCollector.increment_aborts();  // Increment the count of aborted transactions

        // Throw an exception to indicate that the transaction has been aborted
        throw std::runtime_error("Transaction aborted");    // Signal the abort to calling processes
    }


    // Attempt to lock all variables in the write set.
    bool lockWriteSet() {
        for (auto &var : writeLog) {
            if (!var.first->acquireLock(std::chrono::milliseconds(100))) {
                return false;
            }
        }
        return true;
    }

    // Store a value in the STM variable, with type checks and conversions.
    inline void storeSingleEntry(StmVariableBase *key, std::any* &value) {
        // Extensive type checking to ensure correct type casting before storing.
        // This block covers a variety of types that might be stored in an STM variable.
        if (auto intVal = std::any_cast<int>(value))
        {
            key->storeSTM_Var(*intVal);
        }
        else if (auto doubleVal = std::any_cast<double>(value))
        {
            key->storeSTM_Var(*doubleVal);
        }
        else if (auto floatVal = std::any_cast<float>(value))
        {
            key->storeSTM_Var(*floatVal);
        }
        else if (auto charVal = std::any_cast<char>(value))
        {
            key->storeSTM_Var(*charVal);
        }
        else if (auto boolVal = std::any_cast<bool>(value))
        {
            key->storeSTM_Var(*boolVal);
        }
        else if (auto shortVal = std::any_cast<short>(value))
        {
            key->storeSTM_Var(*shortVal);
        }
        else if (auto longVal = std::any_cast<long>(value))
        {
            key->storeSTM_Var(*longVal);
        }
        else if (auto longLongVal = std::any_cast<long long>(value))
        {
            key->storeSTM_Var(*longLongVal);
        }
        else if (auto unsignedCharVal = std::any_cast<unsigned char>(value))
        {
            key->storeSTM_Var(*unsignedCharVal);
        }
        else if (auto unsignedShortVal = std::any_cast<unsigned short>(value))
        {
            key->storeSTM_Var(*unsignedShortVal);
        }
        else if (auto unsignedIntVal = std::any_cast<unsigned int>(value))
        {
            key->storeSTM_Var(*unsignedIntVal);
        }
        else if (auto unsignedLongVal = std::any_cast<unsigned long>(value))
        {
            key->storeSTM_Var(*unsignedLongVal);
        }
        else if (auto unsignedLongLongVal = std::any_cast<unsigned long long>(value))
        {
            key->storeSTM_Var(*unsignedLongLongVal);
        }
        else if (auto stringVal = std::any_cast<std::string>(value))
        {
            key->storeSTM_Var(*stringVal);
        }
        else if (auto nodeVal = std::any_cast<Node<int> *>(value))
        {
            key->storeSTM_Var(*nodeVal);
        }
        else if (auto llVal = std::any_cast<LinkedList<int>>(value))
        {
            key->storeSTM_Var(*llVal);
        }
        else
        {
            key->storeSTM_Var(nullptr);
        }
    }

    // Restart the transaction, reinitializing and resetting statistics.
    inline void restartTransaction() {
        initTransaction();
        if(collectStats) statCollector.increment_restarts();
    }

    static void restartTransaction(Transaction *t_ptr) {
        t_ptr->restartTransaction();
    }

public:
    // Constructor to initialize a transaction with given parameters.
    Transaction(std::thread::id id, bool collectStats, Statistics& statCollector) 
        : transactionId(id), collectStats(collectStats), statCollector(statCollector), 
          readVersion(0), writeVersion(0), readOnly(true) {
        initTransaction();
    }

    // Initialize transaction properties and fetch the current global version clock.
    inline void initTransaction() {
        status = 0;
        readVersion = get_GVC();
        if(collectStats) statCollector.increment_inits();
    }

    static void initTransaction(Transaction *t_ptr) {
        t_ptr->initTransaction();
    }

    // Destructor to clean up any dynamically allocated memory in writeLog.
    ~Transaction() {
        for (auto &entry : writeLog) {
            delete entry.second;
        }
        readLog.clear();
        writeLog.clear();
    }

    // Complete the transaction, releasing resources and clearing logs.
    void endTransaction() {
        for (auto& entry : writeLog) {
            delete entry.second;
        }
        writeLog.clear();
        readLog.clear();
    }

    static void endTransaction(Transaction *t_ptr) {
        t_ptr->endTransaction();
    }

    // Methods to handle reading and writing within the transaction
    // Include templates for handling different types of STM variables.
    // These methods ensure type safety and manage concurrency controls.

    template <typename T>
    inline void readTransactional(T *dest, StmVariable<T> *toRead)
    {
        // Check if the transaction is read-only. Read-only transactions do not modify the readLog.
        if (!readOnly)
        {
            // Add the STM variable address to the readLog to track all read operations for possible validation at commit time.
            readLog.insert(static_cast<StmVariableBase *>(toRead));

            // Check if the variable to read is also in the writeLog, indicating it has been written to during this transaction.
            auto castReadLoc = static_cast<StmVariableBase *>(toRead);
            if (writeLog.find(castReadLoc) != writeLog.end())
            {
                try
                {
                    // If present in the writeLog, load the last written value from the writeLog into the destination variable.
                    *dest = std::any_cast<T>(*(writeLog[castReadLoc]));
                }
                catch (const std::bad_any_cast &e)
                {
                    // Handle type casting errors by logging and rethrowing, ensuring type safety.
                    std::cerr << "Failed to cast in readTransactional: " << e.what() << std::endl;
                    throw;
                }
                return;
            }
        }
        // Sample the lock version before reading the variable to handle concurrent modifications.
        int lockNum;
        toRead->getLockVersion(&lockNum);

        // Perform the read operation to get the current value of the STM variable.
        *dest = toRead->loadSTM_Var();

        // Post-validation step: Sample the lock version again after reading.
        int postValidLockVersion;
        toRead->getLockVersion(&postValidLockVersion);

        // Check if the lock version has changed during the read, or if the version is higher than the read version,
        // or if the variable is locked by another thread, indicating a potential conflict or inconsistency.
        if (!((postValidLockVersion == lockNum) // Ensures no change in lock version during the read.
            && (postValidLockVersion <= readVersion) // Ensures the read version is consistent or earlier.
            && !toRead->isLocked())) // Ensures the variable is not locked by another thread.
        {
            // If any of the conditions fail, set the transaction status to aborted and abort the transaction.
            // This ensures the transaction remains consistent and isolated.
            status = -1;
            abortTransaction();
        }
    }

    template <typename T>
    static inline void readTransactional(Transaction *t_ptr, T *dest, StmVariable<T> *toRead)
    {
        t_ptr->readTransactional(dest, toRead);
    }

    template <typename T>
    bool writeTransactional(StmVariable<T> *dest, T *toWrite)
    {
        // Check if the transaction is currently marked as read-only. If it is, any write operation
        // should convert the transaction to a write-enabled state before proceeding.
        if (readOnly)
        {
            writeModeOn(); // Switch transaction to write mode if it was previously read-only.
            status = -1;   // Mark the transaction status as aborted since its state is changing unexpectedly.
            abortTransaction(); // Abort the transaction to ensure data consistency and isolation.
            return false;  // Return false to indicate that the write operation was not successful.
        }

        // Cast the destination STM variable to its base type for compatibility with the write log map.
        auto castWriteLoc = static_cast<StmVariableBase *>(dest);

        // Check if the pointer to the data to write is null. If it is, simply record a null in the write log.
        if (!toWrite)
        {
            writeLog[castWriteLoc] = nullptr; // Record a null entry indicating that no data should be written.
            return true; // Return true indicating the write operation was accepted (even though it writes a null).
        }

        // Create a new std::any object to encapsulate the value to be written.
        std::any *tempWrite = new std::any(*toWrite);

        // Check if the destination is already present in the write log.
        auto it = writeLog.find(castWriteLoc);
        if (it != writeLog.end())
        {
            delete it->second; // If present, delete the old value to prevent memory leaks.
            it->second = tempWrite; // Replace the old value with the new encapsulated value.
        }
        else
        {
            // If not present in the write log, add the new encapsulated value to the log.
            writeLog[castWriteLoc] = tempWrite;
        }

        // Return true to indicate that the write operation was successful.
        return true;
    }


    template <typename T>
    static inline void writeTransactional(Transaction *t_ptr, StmVariable<T> *dest, T *toWrite)
    {
        t_ptr->writeTransactional(dest, toWrite);
    }

    // These methods are core components of the transaction handling process in a software transactional memory (STM) system.
    // tryCommitTransaction attempts to finalize changes by locking necessary resources and verifying the transaction's integrity.
    // If it encounters any conflicts or issues, it aborts the transaction. 
    // The commitTransaction method orchestrates the commit attempt and manages any errors or restart needs based on the transaction's state.


    // Attempt to commit the current transaction. This includes locking the write set, 
    // incrementing the version clock, validating the read set, and updating the write set.
    inline void tryCommitTransaction()
    {
        if (!readOnly)
        {
            // Attempt to acquire locks on all resources in the write set.
            bool acquisition = lockWriteSet();
            if (!acquisition)
            {
                // If locks cannot be acquired, abort the transaction and exit the function.
                abortTransaction();
                return;
            }

            // Increment the global version clock to reflect the new version for this transaction.
            writeVersion = increment_and_fetch_GVC();

            // Validate the read set to ensure no read values have been modified externally since read.
            for (auto &loc : readLog)
            {
                int localLockVers;
                loc->getLockVersion(&localLockVers);
                if (localLockVers > readVersion || loc->isLocked())
                {
                    // If validation fails, abort the transaction and exit.
                    abortTransaction();
                    return;
                }
            }

            // Commit all writes from the write log to the STM variables.
            for (auto &var : writeLog)
            {
                storeSingleEntry(var.first, var.second); // Perform the write operation.
                var.first->setLockVersion(writeVersion); // Update the lock version to the new global version.
                var.first->releaseLock(); // Release all locks held by this transaction.
            }
        }
    }

    static inline void tryCommitTransaction(Transaction *t_ptr)
    {
        t_ptr->tryCommitTransaction();
    }

    // Control the commit process of a transaction, handle exceptions, and manage transaction restarts.
    bool commitTransaction()
    {
        if (status == -1) // Check if the transaction is marked as needing to be aborted.
        {
            restartTransaction(); // Restart the transaction in response to previous aborts.
            return false; // Indicate that the commit was not successful.
        }
        else
        {
            try
            {
                tryCommitTransaction(); // Attempt to commit the transaction.
                return true; // Return true if commit is successful.
            }
            catch (const std::exception &e)
            {
                restartTransaction(); // Restart the transaction if an exception is thrown during commit.
                return false; // Return false indicating the commit was not successful.
            }
        }
    }
};

#endif
