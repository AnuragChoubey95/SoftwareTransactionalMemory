// STM_VARIABLE_H defines the interface and implementation for StmVariableBase and StmVariable.
// These classes provide a framework for managing software transactional memory (STM) variables,
// supporting operations such as lock management, value storage, and concurrency control.
// They utilize modern C++ features including std::any for type-safe variable storage.

// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#ifndef STM_VARIABLE_H
#define STM_VARIABLE_H

#include <any>
#include <atomic>
#include <iostream>
#include <chrono>
#include <memory>
#include <mutex>
#include <typeinfo>
#include <variant>
#include <string>
#include <thread>

class StmVariableBase {
public:
    virtual ~StmVariableBase() = default;  

    // Store a value in the STM variable. The actual type of value is managed using std::any.
    virtual void storeSTM_Var(const std::any& value) = 0;

    // Check if the variable is currently locked.
    virtual bool isLocked() = 0;

    // Attempt to acquire a lock on this variable within the specified timeout period.
    virtual bool acquireLock(std::chrono::milliseconds timeout) = 0;

    // Release the lock held by the current thread, if it holds it.
    virtual bool releaseLock() = 0;

    // Retrieve the current lock version.
    virtual void getLockVersion(int* lockVal) = 0;

    // Set the lock version to a specified number.
    virtual void setLockVersion(int num) = 0;
};


template<typename T>
class StmVariable : public StmVariableBase {
private:
    T val;  // The value of the STM variable, generic type T.
    int lockVersion;  // Version number of the lock, used for concurrency control.
    std::recursive_timed_mutex mtx;  // Mutex that allows recursive locking with timeout.
    std::thread::id lastLockedBy;  // ID of the thread that last locked this variable.

public:
    StmVariable() = default;  // Default constructor.
    StmVariable(const T& value) : val(value), lockVersion(0) {}  // Constructor initializing the variable with a value.

    // Copy constructor to ensure correct copying behavior.
    StmVariable(const StmVariable& other) : val(other.val) {}

    // Overload the output stream operator to print the variable's value.
    friend std::ostream& operator<<(std::ostream& os, const StmVariable<T>& var) {
        os << var.val;
        return os;
    }

    // Store a new value in this variable, ensuring type safety via std::any_cast.
    inline void storeSTM_Var(const std::any& value) override {
        try {
            val = std::any_cast<T>(value);
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Bad cast: " << e.what() << std::endl;
            throw;
        }
    }

    // Load the current value of the variable.
    T loadSTM_Var() const {
        return val;
    }

    // Check if the variable is locked by trying to acquire and immediately release the lock.
    bool isLocked() override {
        if (mtx.try_lock()) {
            mtx.unlock();
            return false;
        }
        return true;
    }

    // Attempt to acquire the lock within a specified timeout. Store the locking thread's ID.
    bool acquireLock(std::chrono::milliseconds timeout) override {
        if (mtx.try_lock_for(timeout)) {
            lastLockedBy = std::this_thread::get_id();
            return true;
        }
        return false;
    }

    // Release the lock only if it is held by the current thread.
    bool releaseLock() override {
        if (lastLockedBy == std::this_thread::get_id()) {
            mtx.unlock();
            return true;
        }
        return false;
    }

    // Retrieve the lock version into a provided integer pointer.
    inline void getLockVersion(int* lockVal) override {
        *lockVal = lockVersion;
    }

    // Set the lock version to a new value.
    inline void setLockVersion(int num) override {
        lockVersion = num;
    }
};

#endif