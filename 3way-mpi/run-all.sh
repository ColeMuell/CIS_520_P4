#!/bin/bash -l
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

#SBATCH --job-name=mpi
#SBATCH --time=0-10:00:00
#SBATCH --ntasks-per-node=16
#SBATCH --nodes=20
#SBATCH --mem=7G
#SBATCH --constraint=moles
##SBATCH --cpus-per-task=1 
##SBATCH --threads-per-core=1

threads=10
batches=1000
batchSize=1000

for i in {1..20}
do
    for j in {1..5}
    do
        mpirun --oversubscribe -np $i ./mpi-imp 1000000
    done

    echo -e ""
done 

for ((i=100000;i<1000001;i+=100000))
do
    for j in {1..5}
    do
        mpirun --oversubscribe -np 10 ./mpi-imp $i
    done

    echo -e ""
done 