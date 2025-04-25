/* 
 * This example is based on the code of Andrew V. Adinetz
 * https://github.com/canonizer/mandelbrot-dyn
 * Licensed under The MIT License
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 

#define ARRAY_SIZE 2000000
#define MAX_STRING_SIZE 2200
#define ALPHABET_SIZE 26
#define BATCH_SIZE 800
#define TARGET_LINE 1000

int total_read = 0;
int max_ascii[BATCH_SIZE];

char **lines;

//prints results. The param offset says how far off of 0 the batched lines are
void print_results(int offset, int count) {
    int i;
    // prints out maxes
    printf("offset %d count %d\n",offset, count);

    for (i = 0; i < count; i++) {
        printf("Line %d: %d\n", i + offset, max_ascii[i]);
    }
}

//Finds the max ascii value in a given line
char find_max(const char* line, int length) {
    int max_int = -1; 
    int len = strnlen(line, length);
    for (int i = 0; i < len; i++) {
        if (line[i] > max_int) {

        max_int = line[i];
        }
    }
  
    return max_int;
}


// The kernel to 
int kernel(int id,int max_local[])
{
    printf("my id is %d\n",id);
    for (int i = 0; i < 100; i++) {      
            if(! lines[(int) i]) return 0;
            max_local[i] = find_max(lines[(int)i], MAX_STRING_SIZE);
    }
    printf("done, my id is %d\n",id);
        return 0;
}

int read_file(FILE* fd) {
    
    char buffer[MAX_STRING_SIZE];
    int count = 0;
    size_t len;

        while (fgets(buffer, MAX_STRING_SIZE, fd) && count <BATCH_SIZE ) {
       
            buffer[strcspn(buffer, "\n")] = 0;
            
            lines[count] = strdup(buffer);
            
            count++;
        }
        total_read +=count;
        return count;        
}

int main(int argc, char **argv)
{
     int  myid;
     int ntasks;
     int max_local[BATCH_SIZE/10];
   
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    lines = (char **)calloc(BATCH_SIZE,  sizeof(char *));

    if(myid == 0){
        FILE* fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
            if (!fd) {
                perror("Error opening file");
                exit(1);
            }
        read_file(fd);
        
        MPI_Bcast(lines, sizeof(lines), MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    
    /* Very simple parallelisation, where the domain is divided evenly 
     * along one dimension. Because computation cost varies within the domain
     * the code demonstrates how load imbalance hinders parallel scalability
    */
    double t1 = MPI_Wtime();
   
	kernel(myid, max_local);
     
    MPI_Gather(max_local, 100, MPI_INT, max_ascii, 100, MPI_INT, 0,MPI_COMM_WORLD);

    double t2 = MPI_Wtime();

    if (myid == 0)
    {
        print_results(0, 1000);
    }

	double walltime = t2 - t1;
	// Print the timings
	// printf("Mandelbrot set computed in %.3lf s, at %.3lf Mpix/s\n",
	//        walltime, h * w * 1e-6 / walltime );
    // }
    printf("Process %d finished in %.3lf s\n", myid, t2-t1);

    MPI_Finalize();

    return 0;
}
