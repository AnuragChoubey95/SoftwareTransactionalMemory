#!/bin/bash

outfile="results.csv"
echo "threads,nodes,option,average_ratio" > "$outfile"

for threads in $(seq 2 2 10); do
    for nodes in $(seq 2 2 50); do
        for option in -m -f -a; do
            avg_ratio=$(bash llAverage.sh $option 1 $threads $nodes 25 | awk -F': ' '{print $2}')
            echo "$threads,$nodes,${option:1},$avg_ratio" >> "$outfile"
            echo "Done: threads=$threads nodes=$nodes option=$option"
        done
    done
done

echo "All results stored in $outfile"
