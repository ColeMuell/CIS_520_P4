#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

#SBATCH --job-name=mpi
#SBATCH --time=0-1:00:00
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=1
#SBATCH --mem=7G
##SBATCH --constraint=moles
##SBATCH --cpus-per-task=1 
##SBATCH --threads-per-core=1

for i in $(seq 1 20); do

echo $1, >> results.csv
export OMP_NUM_THREADS=$1
for i in $(seq 1 20); do
./3way-openmp/openmp-imp ~dan/625/wiki_dump.txt >> results.csv
echo , >> results.csv
done
