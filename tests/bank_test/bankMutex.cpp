// bankMutex.cpp
// This program simulates a multi-threaded bank account transfer system using Software Transactional Memory (STM)
// variables to ensure thread-safe operations. 
// A global locking mechanism is employed to maintaining read-write safety.
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

// Define the path to the necessary headers for transaction handling and macros
#define TRANSACTION_H_PATH "/home/ac2255/phpc/weeks_11_to_15/revised_final/new/src/transaction.h"
#define MACRO_H_PATH "/home/ac2255/phpc/weeks_11_to_15/revised_final/new/src/macro.h"

#include TRANSACTION_H_PATH // Include the transaction system definitions
#include MACRO_H_PATH // Include predefined macros for transaction handling

#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <cstdlib> 

std::random_device rd;  // Random device for generating numbers
std::mt19937 gen(rd()); // Random generator

std::mutex account_mutex; // Mutex for locking account updates to prevent race conditions

// Function to populate a vector of accounts with a given initial amount
void populateAccounts(std::vector<StmVariable<int>>& accounts, int amount, int entries) {
    for (int i = 0; i < entries; i++) {
        accounts.push_back(StmVariable<int>(amount)); // Initialize accounts with the specified amount
    }
}

// Function to perform transfers between accounts in a thread-safe manner using STM Variables
void transfer(std::vector<StmVariable<int>>& accounts,
                std::uniform_int_distribution<>& dis,
                    std::uniform_int_distribution<> amount_dis, int NUM_LOOPS) {
    
    for(long i = 0; i < NUM_LOOPS; i++){
        int source = dis(gen); // Randomly select source account index
        int dest = dis(gen); // Randomly select destination account index
        int amount = amount_dis(gen); // Randomly determine the amount to transfer

        if (dest != source) {
            std::lock_guard<std::mutex> lock(account_mutex); // Lock the accounts during update

            if (amount <= accounts[source].loadSTM_Var()) { // Check if the source account has enough funds
                int finalSrcAmount = accounts[source].loadSTM_Var() - amount; // Calculate new source balance
                int finalDestAmount = accounts[dest].loadSTM_Var() + amount; // Calculate new destination balance
                accounts[source].storeSTM_Var(finalSrcAmount); // Update source account
                accounts[dest].storeSTM_Var(finalDestAmount); // Update destination account
            }
        }
    }
}

// Function to calculate the total sum of all account balances for verification
int getSumOfArray(std::vector<StmVariable<int>>& accounts){
    int sum = 0;

    for(auto& entry : accounts){
        sum += entry.loadSTM_Var(); 
    }

    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <NUM_THREADS> <NUM_LOOPS>" << std::endl;
        return 1;
    }

    int NUM_THREADS = std::atoi(argv[1]);
    int NUM_LOOPS = std::atoi(argv[2]);

    std::vector<StmVariable<int>> accounts; // Vector to hold all accounts
    int num_entries = 20000; // Number of accounts
    int amount = 100; // Initial amount for each account

    populateAccounts(accounts, amount, num_entries); // Populate accounts with initial funds

    std::vector<std::thread> threads; // Vector to hold all threads
    
    std::uniform_int_distribution<> dis(0, num_entries - 1); // Distribution for account indices
    std::uniform_int_distribution<> amount_dis(1, amount); // Distribution for transfer amounts

    // Create and launch threads
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(transfer, std::ref(accounts), std::ref(dis), std::ref(amount_dis), NUM_LOOPS));
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Check that no money was created or destroyed during the transactions
    std::cout << "No money created or destroyed: " << ((num_entries * amount) == getSumOfArray(accounts)) << std::endl;

    return 0;
}
