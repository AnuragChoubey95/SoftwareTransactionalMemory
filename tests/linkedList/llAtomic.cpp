#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <random>

template<typename T>
struct Node {
    T value;
    std::atomic<Node*> next;

    explicit Node(T val) : value(val), next(nullptr) {}
};

template<typename T>
struct LinkedList {
    std::atomic<Node<T>*> head;
    LinkedList() : head(nullptr) {}
};

template<typename T>
void sortedInsert(LinkedList<T>& list, Node<T>* node) {
    Node<T>* prev = nullptr;
    Node<T>* curr = list.head.load(std::memory_order_acquire);

    while (true) {
        while (curr && curr->value < node->value) {
            prev = curr;
            curr = curr->next.load(std::memory_order_acquire);
        }
        
        node->next.store(curr, std::memory_order_relaxed);

        Node<T>* expected = curr;
        if (!prev) {
            if (list.head.compare_exchange_weak(
                    expected, node,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire))
                return;
        } else {
            if (prev->next.compare_exchange_weak(
                    expected, node,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire))
                return;
        }
        // CAS failed, restart traversal
        prev = nullptr;
        curr = list.head.load(std::memory_order_acquire);
    }
}

template<typename T>
Node<T>* popAtomic(LinkedList<T>& list) {
    while (true) {
        Node<T>* oldHead = list.head.load(std::memory_order_acquire);
        if (!oldHead)
            return nullptr;

        Node<T>* next = oldHead->next.load(std::memory_order_acquire);
        if (list.head.compare_exchange_weak(
                oldHead, next,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            oldHead->next.store(nullptr, std::memory_order_release);
            return oldHead;
        }
    }
}


template<typename T>
void popTransaction(LinkedList<T>& list) {
    for (int i = 0; i < 15; i++) {
        Node<T>* node = popAtomic(list);
        if (node) delete node;
    }
}

template<typename T>
bool isSorted(const LinkedList<T>& list) {
    Node<T>* curr = list.head.load(std::memory_order_acquire);
    if (!curr) return true;

    T currVal = curr->value;
    Node<T>* next = curr->next.load(std::memory_order_acquire);

    while (next) {
        T nextVal = next->value;
        if (nextVal < currVal)
            return false;

        curr = next;
        next = curr->next.load(std::memory_order_acquire);
        currVal = curr->value;
    }
    return true;
}


template<typename T>
void printList(const LinkedList<T>& list) {
    Node<T>* curr = list.head.load(std::memory_order_acquire);
    while (curr) {
        std::cout << curr->value << "->";
        curr = curr->next.load(std::memory_order_acquire);
    }
    std::cout << "null\n";
}

template<typename T>
int length(const LinkedList<T>& list) {
    int count = 0;
    Node<T>* curr = list.head.load(std::memory_order_acquire);

    while (curr) {
        ++count;
        curr = curr->next.load(std::memory_order_acquire);
    }
    return count;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <NUM_THREADS> <NUM_NODES_PER_THREAD>\n";
        return 1;
    }

    int num_threads = std::atoi(argv[1]);
    int num_nodes = std::atoi(argv[2]);

    LinkedList<int> list;
    std::vector<std::vector<Node<int>*>> groups(num_threads);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (std::vector<Node<int>*>& g : groups)
        for (int i = 0; i < num_nodes; i++)
            g.push_back(new Node<int>(dis(gen)));

    std::vector<std::thread> threads;
    for (std::vector<Node<int>*>& g : groups)
        threads.emplace_back([&list, &g]() {
            for (Node<int>* n : g) sortedInsert(list, n);
        });

    threads.emplace_back([&list]() {
        popTransaction(list);
    });

    for (auto& t : threads) t.join();

    printList(list);
    std::cout << "Sorted: " << isSorted(list) << std::endl;
    std::cout << "Length: " << length(list) << std::endl;

    for (std::vector<Node<int>*>& g : groups){
        for (Node<int>* n : g){
            if (n) delete n;
        }   
    }
        
}

