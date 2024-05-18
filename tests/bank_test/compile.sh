#!/bin/bash

function print_usage() {
    echo "Usage: ./compile.sh [option]"
    echo "Options:"
    echo "    -m    Compile and run bankMutex program"
    echo "    -s    Compile and run bankSTM program"
    echo "    -f    Compile and run bankFineGrained program"
}

if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

while getopts ":msf" opt; do
    case $opt in
        m)
            echo "Compiling bankMutex.cpp..."
            g++ -o myprogramMutex bankMutex.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramMutex [NUM_THREADS] [NUM_LOOPS]"
            ;;
        s)
            echo "Compiling bankSTM.cpp..."
            g++ -o myprogramSTM bankSTM.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramSTM [collectStats: 0 or 1] [NUM_THREADS] [NUM_LOOPS]"
            ;;
        f)
            echo "Compiling bankFineGrained.cpp..."
            g++ -o myprogramFineGrained bankFineGrained.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramFineGrained [NUM_THREADS] [NUM_LOOPS]"
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            print_usage
            exit 1
            ;;
    esac
done
