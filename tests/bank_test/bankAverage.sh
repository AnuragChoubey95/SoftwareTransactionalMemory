#!/bin/bash

function print_usage() {
    echo "Usage: $0 -m|-f arg1 arg2 arg3 <num_runs>"
    echo "  -m|-f  Choose which executable to test against myprogramSTM"
    echo "  arg1  Additional argument for statistics collection for myprogramSTM"
    echo "  arg2 arg3  NUM_THREADS and NUM_LOOPS for the executable and myprogramSTM"
    echo "  num_runs  Number of runs to average"
    echo "Example: $0 -m 1 10 5000 100"
}

if [ $# -lt 5 ]; then
    print_usage
    exit 1
fi

option=$1

if ! [[ "$option" == "-m" || "$option" == "-f" ]]; then
    echo "Error: First argument must be -m or -f"
    print_usage
    exit 1
fi

# Arguments for the executables
arg1=$2
arg2=$3
arg3=$4
num_runs=$5

if ! [[ "$num_runs" =~ ^[0-9]+$ ]] || [ "$num_runs" -le 0 ]; then
    echo "Error: num_runs must be a positive integer."
    exit 1
fi

total_ratio=0

for i in $(seq 1 $num_runs); do
    ratio_output=$(bash bankTime.sh $option $arg1 $arg2 $arg3)
    ratio=$(echo "$ratio_output" | grep -oP 'Relative time ratio \([^)]+\): \K[\d.]+')
    total_ratio=$(echo "$total_ratio + $ratio" | bc -l)
    # echo "Iteration $i: Ratio = $ratio"
done

average_ratio=$(echo "scale=5; $total_ratio / $num_runs" | bc -l)

echo "Average Relative time ratio STM vs (${option:1}) over $num_runs runs: $average_ratio"
