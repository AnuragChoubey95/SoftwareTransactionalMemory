// This program demonstrates the use of Software Transactional Memory (STM) to manage a sorted linked list across multiple threads.
// It supports operations such as inserting nodes in a sorted manner, removing nodes, and assessing the list's properties (sorted and length).
// These operations are synchronized using STM, ensuring thread safety without explicit locking mechanisms (except for STM's internal mechanisms).
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#include <thread>       
#include <random>       
#include <vector>       
#include <iostream>    

#define MACRO_H_PATH "../../src/macro.h"
#include MACRO_H_PATH  

Statistics myStatCollector;  // Object to collect statistics about STM operations

// Function to push a node to the front of the list using STM
template<typename T>
void push(Transaction* t_ptr, LinkedList<T>& list, Node<T>& argNode) {
    Node<T>* tmp;  // Declare a pointer to store the current head of the list
    auto tmp2 = &argNode;  // Create a pointer to the new node
    Transaction::readTransactional(t_ptr, &tmp, &list.head);  // Transactionally read the head of the list

    if (tmp) {
        Transaction::writeTransactional(t_ptr, &argNode.next, &tmp);  // Transactionally set new node's next to current head
    }
    Transaction::writeTransactional(t_ptr, &list.head, &tmp2);  // Transactionally update the list's head to the new node
}

// Function to insert a node into the list in a sorted order using STM
template<typename T>
void sortedPush(Transaction* t_ptr, LinkedList<T>& list, Node<T>& argNode){
    Node<T>* currNodeAddr;  // Pointer to track the current node during traversal
    auto argNodePtr = &argNode;  // Pointer to the new node

    Transaction::readTransactional(t_ptr, &currNodeAddr, &list.head);  // Transactionally read the head of the list
    
    if (!currNodeAddr) {  // If the list is empty
        Transaction::writeTransactional(t_ptr, &list.head, &argNodePtr);  // Transactionally set the new node as head
    } else {
        T argNodeVal = argNode.value.loadSTM_Var();  // Retrieve the value of the new node
        T currNodeVal;
        Transaction::readTransactional(t_ptr, &currNodeVal, &currNodeAddr->value);  // Retrieve the value of the current node

        if (argNodeVal < currNodeVal) {
            Transaction::writeTransactional(t_ptr, &argNode.next, &currNodeAddr);
            Transaction::writeTransactional(t_ptr, &list.head, &argNodePtr);
            return;
        }

        Node<T>* nextNodeAddr;
        Transaction::readTransactional(t_ptr, &nextNodeAddr, &currNodeAddr->next);

        // Traverse the list to find the correct insertion point
        while (nextNodeAddr && nextNodeAddr->value.loadSTM_Var() < argNodeVal) {
            currNodeAddr = nextNodeAddr;
            Transaction::readTransactional(t_ptr, &nextNodeAddr, &currNodeAddr->next);
        }

        Transaction::writeTransactional(t_ptr, &currNodeAddr->next, &argNodePtr);
        if (nextNodeAddr) {
            Transaction::writeTransactional(t_ptr, &argNodePtr->next, &nextNodeAddr);
        }
    }
}

// Function to remove the head of the list
template<typename T>
void pop(Transaction* t_ptr, LinkedList<T>& list) {
    Node<T>* tmp;  // Temporary pointer to hold the head node
    Transaction::readTransactional(t_ptr, &tmp, &list.head);  // Transactionally read the head of the list

    if (tmp) {  // If the list is not empty
        Node<T>* newHead = tmp->next.loadSTM_Var();  // Determine the new head of the list
        Transaction::writeTransactional(t_ptr, &list.head, &newHead);  // Transactionally update the head of the list
    }
}

// Repeatedly removes nodes from the head of the list
template<typename T>
void popTransaction(bool collectStats, LinkedList<T>& list){
    START_TX(tr, collectStats, myStatCollector) {  // Begin a transaction
        for(int i = 0; i < 15; i++) {
            pop(&tr, list);  // Perform pop operation repeatedly
        }
    } END_TX(tr);  // End the transaction
}

// Adds multiple nodes to the list in a transactional context to ensure thread safety
template<typename T>
void pushTransaction(bool collectStats, LinkedList<T>& list, const std::vector<Node<T>*>& nodes){
    START_TX(tr, collectStats, myStatCollector) {  // Begin a transaction
        for (auto node : nodes) {  // Iterate through all nodes
            if (node != nullptr) {
                sortedPush<T>(&tr, list, *node);  // Perform a sorted insert for each node
            }
        }
    } END_TX(tr);  // End the transaction
}

// Function to print all nodes in the list
template<typename T>
void printList(const LinkedList<T>& list) {
    Node<T>* current = list.head.loadSTM_Var();  
    while (current != nullptr) {
        std::cout << current->value.loadSTM_Var() << "->";  
        current = current->next.loadSTM_Var();  
    }
    std::cout << "null\n";  
}

// Function to check if the list is sorted
template<typename T>
bool isSorted(const LinkedList<T>& list) {
    Node<T>* current = list.head.loadSTM_Var();  
    if (!current) return true;  

    T currVal = current->value.loadSTM_Var();  
    Node<T>* next = current->next.loadSTM_Var();  

    while (next) {  
        T nextVal = next->value.loadSTM_Var();  
        if (nextVal < currVal) {
            return false;  
        }
        current = next;
        next = next->next.loadSTM_Var();
        currVal = nextVal;
    }
    return true;  
}

// Function to calculate the length of the list
template<typename T>
int length(const LinkedList<T>& list) {
    int len = 0;
    Node<T>* current = list.head.loadSTM_Var();  // Start from the head

    while (current) {  // Traverse the list
        len++;  // Increment for each node
        current = current->next.loadSTM_Var();  // Move to the next node
    }
    return len;  // Total length of the list
}

int main(int argc, char* argv[]) {
    if (argc < 4) {  // Check for correct number of arguments
        std::cerr << "Usage: " << argv[0] << " [collectStats: 0 or 1] num_threads num_nodes_per_thread" << std::endl;
        return 1;
    }

    bool collectStats = std::atoi(argv[1]);  // Collect statistics flag
    int num_threads = std::atoi(argv[2]);  // Number of threads
    int num_nodes_per_thread = std::atoi(argv[3]);  // Nodes per thread

    LinkedList<int> list1;  // Create a linked list
    std::vector<std::vector<Node<int>*>> nodeGroups(num_threads);  // Vector of node groups for each thread

    std::random_device rd;  // Random device for number generation
    std::mt19937 gen(rd());  // Random number generator
    std::uniform_int_distribution<> dis(1, 100);  // Distribution for node values

    for (auto& group : nodeGroups) {  // Populate each group with nodes
        for (int i = 0; i < num_nodes_per_thread; i++) {
            group.push_back(new Node<int>(dis(gen)));
        }
    }

    std::vector<std::thread> threads;  // Vector to store threads

    for (auto& group : nodeGroups) {  // Create threads to perform push operations
        threads.emplace_back(pushTransaction<int>, collectStats, std::ref(list1), std::ref(group));
    }
    std::thread popper(popTransaction<int>, collectStats, std::ref(list1));  // Thread to perform pop operations

    for (auto& thread : threads) {  // Join all threads
        thread.join();
    }
    popper.join();  // Join the popper thread

    // Output the results
    std::cout << "Sorted: " << isSorted(list1) << std::endl;
    std::cout << "Length: " << length(list1) << std::endl;

    // Clean up nodes
    for (auto& group : nodeGroups) {
        for (auto node : group) {
            delete node;
        }
    }

    // Print statistics if collected
    if (collectStats) {
        std::cout << "Total Aborts: " << myStatCollector.num_aborts.load() << std::endl;
        std::cout << "Total Inits: " << myStatCollector.num_inits.load() << std::endl;
        std::cout << "Total Restarts: " << myStatCollector.num_restarts.load() << std::endl;
    }

    return 0;
}
