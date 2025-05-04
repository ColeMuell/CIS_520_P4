#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

sbatch --time=10 --mem=7G --cpus-per-task=1 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='1_core' ./pthread/pth-imp.sh 1
sbatch --time=10 --mem=7G --cpus-per-task=5 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='5_core' ./pthread/pth-imp.sh 5
sbatch --time=10 --mem=7G --cpus-per-task=10 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='10_core' ./pthread/pth-imp.sh 10
sbatch --time=10 --mem=7G --cpus-per-task=15 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='15_core' ./pthread/pth-imp.sh 15
sbatch --time=10 --mem=7G --cpus-per-task=20 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='20_core' ./pthread/pth-imp.sh 20
