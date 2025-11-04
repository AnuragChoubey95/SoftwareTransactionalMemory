// transaction.h
#include <unordered_set>
#include <unordered_map>

typedef enum 
{
    ACTIVE,
    ABORTED,
    COMMITTED
} Status;

struct TransactionContext{
    uint64_t read_version;
    unordered_set<void*> read_set;
    Status status;
}