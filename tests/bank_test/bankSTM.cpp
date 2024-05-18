// bankSTM.cpp
// This program simulates a multi-threaded bank transfer system using STM (Software Transactional Memory).
// This version employs the use of our transaction logic to place concurrent operations in a transactional scope
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#define MACRO_H_PATH "/home/ac2255/phpc/weeks_11_to_15/revised_final/new/src/macro.h"
#include MACRO_H_PATH  // Include predefined STM macros for transaction handling

#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <cstdlib>  

std::random_device rd;  // Random device for generating numbers
std::mt19937 gen(rd()); // Random generator
Statistics myStatCollector;  // Global statistics collector to track transactional stats 

// Function to initialize accounts with a specified amount
void populateAccounts(std::vector<StmVariable<int>>& accounts, int amount, int entries) {
    for (int i = 0; i < entries; i++) {
        accounts.push_back(StmVariable<int>(amount));  // Initialize accounts with the specified amount
    }
}

// Function to simulate transfers between accounts
void transfer(bool collectStats, std::vector<StmVariable<int>>& accounts,
              std::uniform_int_distribution<>& dis,
              std::uniform_int_distribution<> amount_dis, int num_loops) {

    // Using the STM macro to start a transactional scope
    START_TX(tr, collectStats, myStatCollector) {
        for (long i = 0; i < num_loops; i++) {
            int source = dis(gen);  // Randomly select source account
            int dest = dis(gen);  // Randomly select destination account
            int amount = amount_dis(gen);  // Randomly determine the amount to transfer

            if (dest != source) {  // Ensure source and destination are not the same
                int tempDest, tempSrc;

                // Transactionally read the balance of destination and source accounts
                tr.readTransactional(&tempDest, &accounts.at(dest));
                tr.readTransactional(&tempSrc, &accounts.at(source));

                // Check if the source account has enough balance to transfer
                if (amount <= tempSrc) {
                    tempSrc -= amount;  // Subtract the transfer amount from the source account
                    tempDest += amount;  // Add the transfer amount to the destination account

                    // Transactionally write the updated balances back to the accounts
                    tr.writeTransactional(&accounts.at(source), &tempSrc);
                    tr.writeTransactional(&accounts.at(dest), &tempDest);
                }
            }
        }
    } END_TX(tr)  // Using the STM macro to end the transactional scope
}

// Function to calculate the total sum of all account balances
int getSumOfArray(const std::vector<StmVariable<int>>& accounts) {
    int sum = 0;
    for (const auto& entry : accounts) {
        sum += entry.loadSTM_Var();  
    }
    return sum;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {  
        std::cerr << "Usage: " << argv[0] << " [collectStats: 0 or 1] [NUM_THREADS] [NUM_LOOPS]" << std::endl;
        return 1;
    }

    bool collectStats = std::atoi(argv[1]) != 0;  // Convert the first argument to boolean
    int num_threads = std::atoi(argv[2]);  
    int num_loops = std::atoi(argv[3]);  
    std::vector<StmVariable<int>> accounts;
    int num_entries = 20000;  // Number of accounts
    int initial_amount = 100;  // Initial amount in each account

    populateAccounts(accounts, initial_amount, num_entries);  // Initialize the accounts with money

    std::vector<std::thread> threads;  // Vector to hold threads
    std::uniform_int_distribution<> dis(0, num_entries - 1);  // Distribution for account indices
    std::uniform_int_distribution<> amount_dis(1, initial_amount);  // Distribution for transfer amounts

    for (int i = 0; i < num_threads; i++) {  // Create and start threads
        threads.push_back(std::thread(transfer, collectStats, std::ref(accounts), std::ref(dis), std::ref(amount_dis), num_loops));
    }

    for (auto& t : threads) {  // Wait for all threads to finish
        t.join();
    }

    // Check and print whether the money was correctly transferred without creating or destroying any
    std::cout << "No money created or destroyed: " << ((num_entries * initial_amount) == getSumOfArray(accounts)) << std::endl;

    if (collectStats) {  // Optionally print statistics collected during the transactions
        std::cout << "Total Aborts: " << myStatCollector.num_aborts.load() << std::endl;
        std::cout << "Total Inits: " << myStatCollector.num_inits.load() << std::endl;
        std::cout << "Total Restarts: " << myStatCollector.num_restarts.load() << std::endl;
    }

    return 0;
}
