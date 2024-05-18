#!/bin/bash

function print_usage() {
    echo "Usage: $0 -m|-f arg1 for myprogramSTM, followed by arg2 arg3 for the chosen program and myprogramSTM"
    echo "Options:"
    echo "  arg1  Additional argument for statistics collection for myprogramSTM"
    echo "  arg2 arg3  NUM_THREADS and NUM_LOOPS for the executable and myprogramSTM"
    echo "    -m    Compare './myprogramMutex arg2 arg3' against './myprogramSTM arg1 arg2 arg3'"
    echo "    -f    Compare './myprogramFineGrained arg2 arg3' against './myprogramSTM arg1 arg2 arg3'"
    echo "Example: $0 -m 1 1000 50"
}

if [ $# -ne 4 ]; then
    print_usage
    exit 1
fi

option="$1"
arg1="$2"  # Stat collection argument for myprogramSTM
arg2="$3"  # NUM_THREADS for both programs
arg3="$4"  # NUM_LOOPS for both programs

case $option in
    -m)
        executable="./myprogramMutex"
        ;;
    -f)
        executable="./myprogramFineGrained"
        ;;
    *)
        echo "Invalid option: $option"
        print_usage
        exit 1
        ;;
esac

function calc_time() {
    local start_time=$(date +%s.%N)
    eval $1 >/dev/null 2>&1
    local end_time=$(date +%s.%N)
    local elapsed_time=$(echo "$end_time - $start_time" | bc)
    
    echo $elapsed_time  
}

time_chosen=$(calc_time "$executable $arg2 $arg3")

time_stm=$(calc_time "./myprogramSTM $arg1 $arg2 $arg3")

if [[ $(echo "$time_chosen == 0" | bc -l) -eq 1 ]]; then
    echo "Error: Division by zero - The execution time of $executable is zero."
    exit 1
fi

ratio=$(echo "scale=2;  $time_stm / $time_chosen" | bc -l)
echo "Relative time ratio (myprogramSTM/${executable##*/}/): $ratio"
echo "--------------*--------------*--------------*--------------"
