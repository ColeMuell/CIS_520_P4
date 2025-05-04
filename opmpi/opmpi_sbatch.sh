#!/bin/bash -l
#SBATCH --job-name=mpi_p4
#SBATCH --time=0-1:00:00
#SBATCH --ntasks-per-node=4
#SBATCH --nodes=4
#SBATCH --mem=2G
##SBATCH --constraint=dwarves

module load foss
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0

mpirun openmpi ~dan/625/wiki_dump.txt