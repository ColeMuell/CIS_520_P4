#!/bin/bash -l

 export OMP_NUM_THREADS=$1
mpirun -x OMP_NUM_THREADS=1 ./3way-openmp/openmp-imp ~dan/625/wiki_dump.txt mdrun -nsteps 500000 -ntomp 1 -v -deffnm 1ns -c 1ns.pdb -nice 0
