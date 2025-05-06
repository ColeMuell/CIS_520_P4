#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

echo "Threads,Run,Time(s)" > results2.csv

for threads in $(seq 1 20); do
    export OMP_NUM_THREADS=$threads

    
    for run in $(seq 1 5); do
        
        time_taken=$(time ./3way-openmp/openmp-imp ~dan/625/wiki_dump.txt)

        
        echo "$threads,$run,$time_taken" >> results2.csv
    done
done