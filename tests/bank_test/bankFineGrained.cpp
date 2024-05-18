// bankSFineGrained.cpp
// This program demonstrates a multi-threaded bank transfer system using fine-grained locks with StmVariable.
// It performs parallel account transfers, ensuring thread safety by explicitly managing locks for each transaction.
// Author: ac2255@g.rit.edu
// Instructor: Dr. Matthew Fluet

#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <cstdlib> 

#define TRANSACTION_H_PATH "/home/ac2255/phpc/weeks_11_to_15/revised_final/new/src/transaction.h"
#include TRANSACTION_H_PATH // Include the transaction handling definitions

std::random_device rd;  // Random device for generating numbers
std::mt19937 gen(rd()); // Random generator

// Function to initialize accounts with a specified number of accounts, each initialized with a given amount
void populateAccounts(std::vector<StmVariable<int>>& accounts, int amount, int entries) {
    for (int i = 0; i < entries; i++) {
        accounts.push_back(StmVariable<int>(amount)); // Initialize each account with a specified amount
    }
}

// Function to simulate transfers between accounts using fine-grained locking
void transfer(std::vector<StmVariable<int>>& accounts,
              std::uniform_int_distribution<>& dis,
              std::uniform_int_distribution<> amount_dis, int num_loops) {
    
    for (long i = 0; i < num_loops; i++) {
        int source = dis(gen); // Select a random source account
        int dest = dis(gen); // Select a random destination account

        if (dest != source) {
            int amount = amount_dis(gen); // Determine the amount to transfer

            accounts[source].acquireLock(std::chrono::milliseconds(100));
            accounts[dest].acquireLock(std::chrono::milliseconds(100));

            if (amount <= accounts[source].loadSTM_Var()) {
                int finalSrcAmount = accounts[source].loadSTM_Var() - amount;
                int finalDestAmount = accounts[dest].loadSTM_Var() + amount;

                accounts[source].storeSTM_Var(finalSrcAmount);
                accounts[dest].storeSTM_Var(finalDestAmount);
            }

            accounts[source].releaseLock();
            accounts[dest].releaseLock();
        }
    }
}

// Function to calculate the total sum of all accounts for validation
int getSumOfArray(const std::vector<StmVariable<int>>& accounts) {
    int sum = 0;
    for (auto& entry : accounts) {
        sum += entry.loadSTM_Var(); // Sum the balance of each account
    }
    return sum;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {  // Ensure the correct number of arguments are provided
        std::cerr << "Usage: " << argv[0] << " [NUM_THREADS] [NUM_LOOPS]" << std::endl;
        return 1;
    }

    int num_threads = std::atoi(argv[1]); // Convert the first argument to integer (number of threads)
    int num_loops = std::atoi(argv[2]); // Convert the second argument to integer (number of loops per thread)

    std::vector<StmVariable<int>> accounts;
    int num_entries = 20000; // Number of accounts
    int amount = 100; // Initial amount in each account

    populateAccounts(accounts, amount, num_entries); // Populate the accounts

    std::vector<std::thread> threads;
    std::uniform_int_distribution<> dis(0, num_entries - 1); // Distribution for account index
    std::uniform_int_distribution<> amount_dis(1, amount); // Distribution for transfer amount

    for (int i = 0; i < num_threads; i++) { // Create and start threads to perform transfers
        threads.push_back(std::thread(transfer, std::ref(accounts), std::ref(dis), std::ref(amount_dis), num_loops));
    }

    for (auto& t : threads) { // Wait for all threads to finish
        t.join();
    }

    // Validate the total amount of money in the system remains constant
    std::cout << "No money created or destroyed: " << ((num_entries * amount) == getSumOfArray(accounts)) << std::endl;

    return 0;
}
