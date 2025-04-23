#!/bin/bash 


    #SBATCH --job-name=pthreads
    #SBATCH --time=0-2:00:00
    #SBATCH --ntasks-per-node=4
    #SBATCH --nodes=$nodes 
    #SBATCH --mem=10G
    #SBATCH --constraint=mole
    module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 

    /homes/sekabanj/CIS_520_P4/pthreads
