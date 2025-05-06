#!/bin/bash -l

#SBATCH --job-name=cuda-imp
#SBATCH --time=0-1:00:00
#SBATCH --nodes=1
#SBATCH --gres=gpu:1
#SBATCH --mem=7G
#SBATCH --output=cuda_imp_%j.out
#SBATCH --error=cuda_imp_%j.err

module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0

threads=256     # default
batches=1000    # default
csv_file="cuda_results.csv"

echo "threads_per_block,batches,runtime_sec,avg_gpu_utilization_pct,avg_memory_used_mb" > $csv_file

run_and_log() {
    local threads_per_block=$1
    local batches=$2

   #found you can do the nvidia smi stuff on stack oflow
    smi_log="nvidia_smi_${threads_per_block}_${batches}.log"
    
    nvidia-smi --query-gpu=timestamp,utilization.gpu,memory.used --format=csv -l 1 > $smi_log &
    smi_pid=$!

    runtime=$(./cuda-imp $threads_per_block $batches)

    kill $smi_pid
    wait $smi_pid 2>/dev/null

    #found this on stack overflow, gets the average utiliztaions nad memory usage from the logs
    avg_util=$(awk -F',' 'NR>1 {sum+=$2} END {print int(sum/(NR-1))}' $smi_log)
    avg_mem=$(awk -F',' 'NR>1 {sum+=$3} END {print int(sum/(NR-1))}' $smi_log)

    echo "$threads_per_block,$batches,$runtime,$avg_util,$avg_mem" >> $csv_file
}

echo "Testing threads/block from 1 to 20:"
for ((i=1;i<=20;i++))
do
    run_and_log $i $batches
done

echo "Testing threads/block from 32-1024 in GPU sizes:"
for i in 32 64 128 256 512 1024
do
    run_and_log $i $batches
done

echo "Testing varying batch sizes with default threads/block ($threads):"
for ((i=100;i<=1000;i+=100))
do
    run_and_log $threads $i
done

echo "Testing combinations of threads/block and batch sizes:"
for ((i=100;i<=1000;i+=100))
do
    for j in 32 64 128 256 512 1024
    do
        run_and_log $j $i
    done
done
