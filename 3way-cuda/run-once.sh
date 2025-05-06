#!/bin/bash -l

#SBATCH --job-name=cuda-imp
#SBATCH --mem=5G
#SBATCH --time=2:00:00
#SBATCH --nodes=1
#SBATCH --gres=gpu:1
#SBATCH --output=max_ascii_cuda_%j.out
#SBATCH --error=max_ascii_cuda_%j.err
#SBATCH --cpus-per-task=1

module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0

threads=512
batches=1000

./cuda-imp $threads $batches
