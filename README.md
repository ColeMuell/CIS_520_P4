The following project is split up in the following ways: There are 3 directories, /3way-pthread /3way-mpi /3way-openmi containing the pthread implementation, 
the mpi implementation, and the OpenMPI implementation respectively. It is important to load in the necessary modules with the
command module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0 CUDA/11.7.0 as the programs will not work otherwise. 


In order to compile the code, cd into the directory and run the Make command. To clean the directory, run the make clean command inside the directory. In order to run each of the programs,
for the pthreads implementation, you run the program with the following command: /3way-pthread/pth-imp ~dan/625/wiki_dump.txt i, where i is the number of threads you would like to run it with. 
You could additionally run the pth-imp.sh script, which will run it 5 times for each range of cores from 1 to 20. In order to submit it, you need to sbatch the pthread-imp.sh.

For the OpenMPI implementation, to compile the code you cd into the directory and run the make command, with make clean cleaning the directory of executables. 
In order to run the program, you first have to specify the number of threads involved, with typing the command into the terminal OMP_NUM_THREADS=x, 
where x is the number of threads you want. Then you can run the command as ./openmp-imp, with an argument for the text file which in this case is ~dan/625/wiki_dump.txt, 
so overall you're execution would look like openmp-imp ~dan/625/wiki_dump.txt. In order to submit the job to beocat, you can run an sbatch on the opmpi-imp.sh script, which would look like sbatch
--time=30 --mem=7G --cpus-per-task=20 --threads-per-core=1 \
--ntasks=1 --nodes=1 --constraint=moles --job-name='20_core' ./full-opmpi.sh


For the MPI implementation, to compile the code you cd into the directory and run the make command, with make clean cleaning the directory of executables and object files.
In order to run the program, you must run the following command: mpirun -n x /homes/sekabanj/CIS_520_P4/mpi_version/mpi_version $i, where x is the number of nodes you want 
to run it with and i is the number of nodes, so for example mpirun -n 11 /homes/sekabanj/CIS_520_P4/mpi_version/mpi_version. In order to use the submission script, you have to run an sbatch on mpi-imp.sh
