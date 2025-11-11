// This program implements a multi-threaded operation on a sorted linked list using variables of the Stm_Variable class.
// It includes functionalities to insert nodes in a sorted manner, remove nodes, check if the list is sorted, and calculate the list's length.
// Multiple threads perform these operations concurrently, demonstrating the use of mutexes for synchronization.

// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#include <thread>
#include <random>
#include <mutex>
#include <vector>
#include <iostream>

#define MACRO_H_PATH "../../src/macro.h"
#include MACRO_H_PATH  

std::mutex listMutex;  // Mutex to ensure thread-safe modifications to the list

// Inserts a node into the list while maintaining list order; thread-safe using mutex
template<typename T>
void push(LinkedList<T>& list, Node<T>& argNode) {
    std::lock_guard<std::mutex> guard(listMutex);
    if (list.head.loadSTM_Var()) {
        argNode.next.storeSTM_Var(list.head.loadSTM_Var());
    }
    list.head.storeSTM_Var(&argNode);
}

// Inserts a node in sorted order; thread-safe
template<typename T>
void sortedPush(LinkedList<T>& list, Node<T>& argNode) {
    std::lock_guard<std::mutex> guard(listMutex);
    Node<T>* current = list.head.loadSTM_Var();
    Node<T>* previous = nullptr;

    while (current != nullptr && argNode.value.loadSTM_Var() > current->value.loadSTM_Var()) {
        previous = current;
        current = current->next.loadSTM_Var();
    }

    argNode.next.storeSTM_Var(current);
    if (previous == nullptr) {
        list.head.storeSTM_Var(&argNode);
    } else {
        previous->next.storeSTM_Var(&argNode);
    }
}

// Removes the head node from the list; thread-safe
template<typename T>
void pop(LinkedList<T>& list) {
    std::lock_guard<std::mutex> guard(listMutex);
    Node<T>* currentHead = list.head.loadSTM_Var();

    if (currentHead == nullptr) {
        return;
    }

    Node<T>* newHead = currentHead->next.loadSTM_Var();
    list.head.storeSTM_Var(newHead);
    currentHead->next.storeSTM_Var(nullptr);
}

// Repeatedly removes head nodes from the list
template<typename T>
void popTransaction(LinkedList<T>& list) {
    for (int i = 0; i < 15; i++) {
        pop(list);
    }
}

// Adds multiple nodes to the list in a transactional context
template<typename T>
void pushTransaction(LinkedList<T>& list, const std::vector<Node<T>*>& nodes) {
    for (auto node : nodes) {
        if (node != nullptr) {
            sortedPush<T>(list, *node);
        }
    }
}

// Prints all nodes in the list
template<typename T>
void printList(const LinkedList<T>& list) {
    Node<T>* current = list.head.loadSTM_Var();
    while (current != nullptr) {
        std::cout << current->value.loadSTM_Var() << "->";
        current = current->next.loadSTM_Var();
    }
    std::cout << "null\n";
}

// Checks if the list is sorted
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
        currVal = current->value.loadSTM_Var();
    }
    return true;
}

// Returns the length of the list
template<typename T>
int length(const LinkedList<T>& list) {
    int len = 0;
    Node<T>* current = list.head.loadSTM_Var();

    while (current) {
        ++len;
        current = current->next.loadSTM_Var();
    }
    return len;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <NUM_THREADS> <NUM_NODES_PER_THREAD>" << std::endl;
        return 1;
    }

    int num_threads = std::atoi(argv[1]);  // Number of threads from command line
    int num_nodes_per_thread = std::atoi(argv[2]);  // Nodes per thread from command line

    LinkedList<int> list1;  // Linked list instance
    std::vector<std::vector<Node<int>*>> nodeGroups(num_threads);  // Vector of node groups

    std::random_device rd;  // Random device for number generation
    std::mt19937 gen(rd());  // Random number generator
    std::uniform_int_distribution<> dis(1, 100);  // Uniform distribution

    for (auto& group : nodeGroups) {
        for (int i = 0; i < num_nodes_per_thread; i++) { 
            group.push_back(new Node<int>(dis(gen)));  // Populate with random values
        }
    }

    std::vector<std::thread> threads;  // Threads vector

    for (auto& group : nodeGroups) {
        threads.emplace_back(pushTransaction<int>, std::ref(list1), std::ref(group));
    }
    std::thread popper(popTransaction<int>, std::ref(list1));

    for (auto& thread : threads) {
        thread.join();  // Wait for threads to finish
    }
    popper.join();  // Wait for the popping thread to finish

    printList(list1);  // Print the final list state
    std::cout << "Sorted: " << isSorted(list1) << std::endl;
    std::cout << "Length: " << length(list1) << std::endl;

    // Clean up dynamically allocated nodes
    for (auto& group : nodeGroups) {
        for (auto node : group) {
            delete node;  
        }
    }

    return 0;
}