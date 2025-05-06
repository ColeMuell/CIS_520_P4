The following project is split up in the following ways: There are 3 directories, /3way-pthread, /3way-mpi, /3way-openmi containing the pthread implementation, 
the mpi implementation, and the OpenMPI implementation respectively. There is also the /3way-cuda directory, which contains the CUDA implementation. It is important to load in the necessary modules with the
command module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0 as the programs will not work otherwise. 


In order to compile each set of code, you need to CD into the directory they are contained in an run a Make command. In order to clean them you need to CD into them and run a make clean command. Each directory has 3 scripts for submitting to beocat. Run-once.sh, which just runs the program once and outputs the results. run-all.sh, which runs the program 5 times, testing 1-20 cores 5 times. The final one is run-all-mem.sh, which also runs a perf command to help with seeing greater performance and memory details. All of them can be run like sbatch ./run-all.sh inside of the intended directory with the implementation that you want to test. You will have to run a make commmand before submitting the script, as well as possibly running the command chmod +x on either the script or the executable, so chmod +x ./pth-imp for the pthreads, ./mpi-imp for the mpi, ./openmp-imp for the open mp, and ./cuda-imp all respectively, otherwise beocat might not recognize it as an executable. 

When you run the executables, the resulting max ASCII outputs will be printed to a file called -output.txt, within the same directory.

For pthreads, in order to run the command on your own machine, you need to go into the terminal and input the following command: ./pth-imp /homes/dan/625/wiki_dump.txt $threads $batches $batchSize, replacing the variablles with the desired numbers.

For MPI, in order to run the command on your own machine, you need run the following command: mpirun --oversubscribe -np x ./mpi-imp /homes/dan/625/wiki_dump.txt y, where x is the number of cores, and y is the inteded batch size.

For OpenMP, in order to run the command on your own machine, you need to run the following command: export OMP_NUM_THREADS=$x
   ./openmp-imp /homes/dan/625/wiki_dump.txt $i $batches $batchSize, where $x and $i are the intended number of threads, $batchsize is the size of the batch required, and $batches is the number of batches required, usually 100000/batchSize.

For CUDA, you can't run the command on your own machine as it is GPU based, so you will have to run it on using the submission script for it, which is run-once.sh.



