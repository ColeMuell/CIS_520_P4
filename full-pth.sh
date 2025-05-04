#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

for i in $(seq 1 20); do

    sbatch --time=10 --mem=7G --cpus-per-task=1 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='1_core' ./3way-pthread/pth-imp.sh $i   

done 