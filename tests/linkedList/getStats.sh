#!/bin/bash

SCRIPT_DIR=$(dirname "$0")

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <mode>"
    echo "  <mode> should be '-m' for Mutex or '-f' for Fine-Grained"
    exit 1
fi

MODE_FLAG=$1  

THREAD_COUNTS=(2 4 8 10 11 12)

NODE_COUNTS=(5 10 20 35 50)

NUM_RUNS=100

if [ "$MODE_FLAG" = "-m" ]; then
    COMPARE_MSG="Comparing STM vs. Mutex"
elif [ "$MODE_FLAG" = "-f" ]; then
    COMPARE_MSG="Comparing STM vs. Fine Grained"
elif [ "$MODE_FLAG" = "-a" ]; then
    COMPARE_MSG="Comparing STM vs. Atomic"
else
    echo "Invalid mode flag. Use '-m' for Mutex or '-f' for Fine-Grained"
    exit 1
fi

echo -e "\n$COMPARE_MSG\n"

for num_nodes in "${NODE_COUNTS[@]}"; do
    for num_threads in "${THREAD_COUNTS[@]}"; do
        echo "Mode: $MODE_FLAG, CollectStats: 0, NUM_THREADS: $num_threads, NUM_NODES_PER_THREAD: $num_nodes, NUM_RUNS: $NUM_RUNS"
        
        timeout 200 bash $SCRIPT_DIR/llAverage.sh $MODE_FLAG 0 $num_threads $num_nodes $NUM_RUNS
        exit_status=$?

        if [ $exit_status -eq 124 ]; then
            echo "Execution timed out for NUM_THREADS=$num_threads, NUM_NODES_PER_THREAD=$num_nodes"
        elif [ $exit_status -ne 0 ]; then
            echo "An error occurred (Exit Status: $exit_status)"
        fi

        echo "----------------------------------------------------------------------------------"
    done
done
