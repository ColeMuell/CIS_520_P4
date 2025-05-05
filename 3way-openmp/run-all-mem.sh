#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

#SBATCH --job-name=mpi
#SBATCH --time=0-1:00:00
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=1
#SBATCH --mem=7G
#SBATCH --constraint=moles
##SBATCH --cpus-per-task=1 
##SBATCH --threads-per-core=1

threads=10
batches=1000
batchSize=1000

for i in {1..20}
do
    export OMP_NUM_THREADS=$i
    perf stat ./openmp-imp ~dan/625/wiki_dump.txt $i $batches $batchSize
done

echo -e ""

for ((i=100;i<1001;i+=100))
do
    export OMP_NUM_THREADS=$threads
    perf stat ./openmp-imp ~dan/625/wiki_dump.txt $threads $i $batchSize
done 

echo -e ""

for ((i=100;i<1001;i+=100))
do
    export OMP_NUM_THREADS=$threads
    perf stat ./openmp-imp ~dan/625/wiki_dump.txt $threads $batches $i
done 
