#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

#SBATCH --job-name=mpi
#SBATCH --time=0-1:00:00
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=1
#SBATCH --mem=7G
#SBATCH --constraint=moles
##SBATCH --cpus-per-task=20
##SBATCH --threads-per-core=16

threads=10
batches=100
batchSize=10000

for i in {1..20}
do
    perf stat ./pth-imp ~dan/625/wiki_dump.txt $i $batches $batchSize
done 

echo -e ""

for ((i=100;i<1001;i+=100))
do
    perf stat ./pth-imp ~dan/625/wiki_dump.txt $threads $i $batchSize
done 

echo -e ""

for ((i=100;i<1001;i+=100))
do

    perf stat ./pth-imp ~dan/625/wiki_dump.txt $threads $batches $i
    
done 
