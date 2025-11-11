    // This program demonstrates a thread-safe sorted linked list using fine-grained locking.
    // It allows for concurrent modifications (insertions and deletions) while maintaining the list's sorted property.
    // The operations leverage STM varibales for consistency and mutexes for thread safety.

    // Author: Anurag Choubey
    // Instructor: Dr. Matthew Fluet

    #include <thread>
    #include <random>
    #include <mutex>
    #include <vector>
    #include <iostream>

    #include "../../src/macro.h"
    #include "../../src/stmVar.h"
    #include "../../src/transaction.h"
    #include "../../src/gvc.h"
    #include "../../src/stats.h"
    #include "../../src/custom_types/ll.h"

    std::mutex listMtx; // workaround

    // Thread-safe insertion of a node at the list's beginning
    template<typename T>
    void push(LinkedList<T>& list, Node<T>& argNode) {
        auto& head = list.head;  // Direct reference for easier access
        head.acquireLock(std::chrono::milliseconds(100));  // Attempt to lock the head with a timeout
        Node<T>* currentHead = head.loadSTM_Var();  // Retrieve current head
        argNode.next.storeSTM_Var(currentHead);  // Set new node's next to point to current head
        head.storeSTM_Var(&argNode);  // Update the head to the new node
        head.releaseLock();  // Unlock the head
    }

    template<typename T>
    void sortedPush(LinkedList<T>& list, Node<T>& argNode) {
        Node<T>* prev;
        Node<T>* current;
        listMtx.lock();
        
        if (list.head.loadSTM_Var() == nullptr) {
            std::cout << "Here" <<std::endl;
            list.head.storeSTM_Var(&argNode);
            listMtx.unlock();
            return;
        }
 
        prev = list.head.loadSTM_Var();
        current = prev->next.loadSTM_Var();

        prev->nodeMtx.lock();
        listMtx.unlock();
        if (current) current->nodeMtx.lock();

        while (current){
            if (current->value.loadSTM_Var() > argNode.value.loadSTM_Var()){
                break;
            }

            Node<T>* oldPrev = prev;
            prev = current;
            current = current->next.loadSTM_Var();
            oldPrev->nodeMtx.unlock();
            if (current) current->nodeMtx.lock();
        }
        argNode.next.storeSTM_Var(current);
        prev->next.storeSTM_Var(&argNode);

        prev->nodeMtx.unlock();
        if (current) current->nodeMtx.unlock();
    }


    // Removes the head node from the list in a thread-safe manner
    template<typename T>
    void pop(LinkedList<T>& list) {
        auto& head = list.head;  
        head.acquireLock(std::chrono::milliseconds(100));  // Lock the head
        Node<T>* currentHead = head.loadSTM_Var();  // Get current head

        if (currentHead) {
            Node<T>* newHead = currentHead->next.loadSTM_Var();  // New head is the next of current head
            head.storeSTM_Var(newHead);  // Update the head
            currentHead->next.storeSTM_Var(nullptr);  // Isolate the old head
        }

        head.releaseLock();  // Unlock the head
    }

    // Repeatedly removes the head node 15 times, simulating a transactional operation
    template<typename T>
    void popTransaction(LinkedList<T>& list){
        for(int i = 0; i < 15; i++){
            pop(list);  
        }        
    }

    // Processes a vector of nodes for sorted insertion in a transactional context
    template<typename T>
    void pushTransaction(LinkedList<T>& list, const std::vector<Node<T>*>& nodes){
        for (auto node : nodes) {
            if (node != nullptr) {
                sortedPush(list, *node);  // Insert each node in sorted order
            }
        }
    }

    // Outputs the contents of the list
    template<typename T>
    void printList(const LinkedList<T>& list) {
        Node<T>* current = list.head.loadSTM_Var();
        while (current) {
            std::cout << current->value.loadSTM_Var() << "->";  // Print the value of each node
            current = current->next.loadSTM_Var();  // Move to the next node
        }
        std::cout << "null\n";  // Indicate the end of the list
    }

    // Checks if the list is sorted in ascending order
    template<typename T>
    bool isSorted(const LinkedList<T>& list) {
        Node<T>* current = list.head.loadSTM_Var();  
        if (!current) return true;  

        while (current && current->next.loadSTM_Var()) {
            if (current->value.loadSTM_Var() > current->next.loadSTM_Var()->value.loadSTM_Var()) {
                return false;  
            }
            current = current->next.loadSTM_Var();  
        }
        return true;  // All nodes are in order, list is sorted
    }

    // Computes the number of nodes in the list
    template<typename T>
    int length(const LinkedList<T>& list) {
        int len = 0;
        Node<T>* current = list.head.loadSTM_Var();  
        while (current) {
            len++;  
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
        std::mt19937 gen(rd());  // Mersenne twister generator
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
