#!/bin/bash

function print_usage() {
    echo "Usage: ./compile.sh [option]"
    echo "Options:"
    echo "    -m    Compile and run llMutex program"
    echo "    -s    Compile and run llSTM program"
    echo "    -f    Compile and run llFineGrained program"
    echo "    -a    Compile and run llAtomic program"
}

if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

while getopts ":msfa" opt; do
    case $opt in
        m)
            echo "Compiling llMutex.cpp..."
            g++ -o myprogramMutex llMutex.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramMutex [NUM_THREADS] [NUM_LOOPS]"
            ;;
        s)
            echo "Compiling llSTM.cpp..."
            g++ -o myprogramSTM llSTM.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramSTM [collectStats: 0 or 1] [NUM_THREADS] [NUM_LOOPS]"
            ;;
        f)
            echo "Compiling llFineGrained.cpp..."
            g++ -o myprogramFineGrained llFineGrained.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramFineGrained [NUM_THREADS] [NUM_LOOPS]"
            ;;
        a)
            echo "Compiling llAtomic.cpp..."
            g++ -o myprogramAtomic llAtomic.cpp -ggdb -O3 -std=c++17
            echo "Run the program as: ./myprogramAtomic [NUM_THREADS] [NUM_LOOPS]"
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            print_usage
            exit 1
            ;;
    esac
done
