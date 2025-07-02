// LINKED_LIST_H
// Defines Node and LinkedList templates for use in a transactional memory system,
// where nodes and links can be modified safely in concurrent environments.
// These classes use StmVariable for atomic operations on node values and pointers.
// Author: Anurag Choubey
// Instructor: Dr. Matthew Fluet

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// Define the path to the StmVariable header file
// Include the definition of StmVariable
#define STM_VAR_H_PATH "../stmVar.h"
#include STM_VAR_H_PATH  

// Template class for Node, designed to be used in a transactional memory system.
template<typename T>
class Node {
public:
    StmVariable<T> value;          // StmVariable wrapping the node's value to enable transactional operations.
    StmVariable<Node<T>*> next;    // StmVariable wrapping the pointer to the next node, also transactionable.

    // Default constructor initializes node with default values.
    Node() {
        value.storeSTM_Var(T{});           
        next.storeSTM_Var(nullptr);        
    }


    // Constructor initializing the node's value.
    Node(const StmVariable<T>& val) {
        value.storeSTM_Var(val.loadSTM_Var());
        next.storeSTM_Var(nullptr);
    }

    Node(const T& raw_value) : value(raw_value), next(nullptr) {}

    // Copy assignment operator to ensure proper copying in transactional contexts.
    Node& operator=(const Node& other) {
        if (this != &other) {
            value.storeSTM_Var(other.value.loadSTM_Var());
            next.storeSTM_Var(other.next.loadSTM_Var());
        }
        return *this;
    }


    // Default destructor is sufficient as StmVariable handles its own memory management.
    ~Node() = default;
};

// Template class for LinkedList, supporting transactional manipulation of linked lists.
template<typename T>
class LinkedList {
public:
    StmVariable<Node<T>*> head;   // Head of the list, wrapped in a StmVariable for transaction safety.

    // Default constructor initializes an empty list.
    LinkedList() {
        head.storeSTM_Var(nullptr);
    }

    // Constructor initializing the list's head.
    LinkedList(Node<T>* headNode) : head(StmVariable(headNode)) {}

    // Copy assignment operator for transaction-safe copying of lists.
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) { 
            // Use transaction-safe methods to store and load the head node.
            head.storeSTM_Var(other.head.loadSTM_Var());
        }
        return *this;
    }
};

#endif // LINKED_LIST_H
