#!/bin/bash -l

#SBATCH --job-name=cuda-imp
#SBATCH --mem=5G
#SBATCH --time=2:00:00
#SBATCH --nodes=1
#SBATCH --gres=gpu:1
#SBATCH --output=max_ascii_cuda_%j.out
#SBATCH --error=max_ascii_cuda_%j.err
#SBATCH --cpus-per-task=4

module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0

echo "Starting CUDA batch tests"

# CSV file to store results
csv_file="cuda_batch_results.csv"

# Write CSV header
echo "threads,batches,run_number,runtime_sec" > $csv_file

for threads in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 32 64 128 256 512 1024
do
    for batches in 500 1000 2000
    do
        for run_number in {1..5}
        do
            runtime=$(./cuda-imp $threads $batches)

            echo "$threads,$batches,$run_number,$runtime" >> $csv_file
        done

        echo ""
    done
done

echo "All CUDA tests completed."
