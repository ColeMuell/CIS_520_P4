#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 

#define ARRAY_SIZE 2000000
#define MAX_STRING_SIZE 2200
#define ALPHABET_SIZE 26
#define BATCH_SIZE 1000
#define TARGET_LINE 1000
#define PROCESS_PARTITION BATCH_SIZE/10

uint16_t numThreads;
int total_read = 0;
// static char ** receivedLines = NULL;
// int max_ascii[BATCH_SIZE];



//prints results. The param offset says how far off of 0 the batched lines are
void print_results(int max_ascii[],int offset, int count) {
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
int kernel(int id, char linesArray[][MAX_STRING_SIZE],int max_local[])
{
    printf("my id is %d\n",id);
    for (int i = 0; i < 100; i++) {      
            if(! linesArray[(int) i]) {
                printf("array was null\n");
                return 0;}
            max_local[i] = find_max(linesArray[(int)i], MAX_STRING_SIZE);
            // printf("%s",linesArray[(int)i]);
        //   printf("%d",max_local[i]);
    }
   
        return 0;
}

int read_file(FILE* fd,char linesArray[][MAX_STRING_SIZE]) {

    
    char buffer[MAX_STRING_SIZE];
    int count = 0;
    size_t len;

        while (count <BATCH_SIZE && fgets(buffer, MAX_STRING_SIZE, fd) ) {
       
            buffer[strcspn(buffer, "\n")] = 0;
            
            // linesArray[count] = strdup(buffer);
            snprintf(linesArray[count], MAX_STRING_SIZE,"%s",buffer);
            
            count++;
        }
        total_read +=count;
        return count;        
}

int main(int argc, char **argv)
{
    if (argc < 2) 
	{
		printf("%s <file> <cores>", argv[0]);
		return EXIT_FAILURE;
	}

    char* filename = argv[1];

    for (size_t i = 0; filename[i] != '\0'; i++) 
	{
        if (filename[i] == 60 ||filename[i] == 62 ||filename[i] == 58 ||filename[i] == 34 ||filename[i] == 92 ||filename[i] == 124 ||filename[i] == 63 ||filename[i] == 42)
            return EXIT_FAILURE;

		if(filename[i] < 31)
			return EXIT_FAILURE;

		if(i > 255)
			return EXIT_FAILURE;
    }

    numThreads = atoi(argv[2]);

    if(numThreads > 100 || numThreads < 1)
        return EXIT_FAILURE;

     int  myid;
     int ntasks;
     int rounds = 0;
     int read_lines = 10;
     int total_lines = 0;
    //  int max_local[BATCH_SIZE/10];
   
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    // char ** lines = (char **)calloc(BATCH_SIZE,  sizeof(char *));
    
    
    FILE* fd;

    double t1 = MPI_Wtime();

    if(myid == 0){

        char lines [BATCH_SIZE][MAX_STRING_SIZE];
        fd = fopen( filename, "r" );
        if (!fd) {
            perror("Error opening file");
            exit(1);
        }
    }
    while(total_lines < BATCH_SIZE * 1000 ){

    if(myid == 0){

        char lines [BATCH_SIZE][MAX_STRING_SIZE];
          
        read_lines = read_file(fd,lines);
        total_lines += read_lines;
        printf("total lines %d\n",total_lines);
               
        int index,i;
        int elements_per_process;
		elements_per_process = BATCH_SIZE / (ntasks-1);

        for (i = 1; i < ntasks; i++) {
            index = (i -1 ) * elements_per_process;

            MPI_Send(&elements_per_process,
                    1, MPI_INT, i, 0,
                    MPI_COMM_WORLD);
            
            MPI_Send(&lines[index],
                    elements_per_process * MAX_STRING_SIZE,
                    MPI_CHAR, i, 0,
                    MPI_COMM_WORLD);
        }
        printf("done sending\n");

		int buffer[elements_per_process] ;
        memset(buffer,0,sizeof(buffer));
		for (i = 1; i < ntasks; i++) {
			MPI_Recv(&buffer, elements_per_process,MPI_INT,
					i, 0,
					MPI_COMM_WORLD,
					&status);
                printf("received results\n");
                int offset = (i - 1) * PROCESS_PARTITION + (BATCH_SIZE * rounds);
                print_results(buffer, offset, PROCESS_PARTITION);
		}       
    }
    else{
    
        static char receivedLines[PROCESS_PARTITION][MAX_STRING_SIZE];
        int max_local[PROCESS_PARTITION];

       int num_of_elements_recieved = 0;
       MPI_Recv(&num_of_elements_recieved,
				1, MPI_INT, 0, 0,
				MPI_COMM_WORLD,
				&status);

        MPI_Recv(&receivedLines, num_of_elements_recieved * MAX_STRING_SIZE,
				MPI_CHAR, 0, 0,
				MPI_COMM_WORLD,
				&status);
	    kernel(myid, receivedLines, max_local);
         MPI_Send(&max_local, PROCESS_PARTITION, MPI_INT,
				0, 0, MPI_COMM_WORLD);
         printf("finised sending my id is %d\n",myid);
        
    }
    rounds += 1;

    }
        double t2 = MPI_Wtime();

	double walltime = t2 - t1;
	// Print the timings
	// printf("Mandelbrot set computed in %.3lf s, at %.3lf Mpix/s\n",
	//        walltime, h * w * 1e-6 / walltime );
    // }
    printf("%.3lf, ", myid, t2-t1);

    MPI_Finalize();

    return 0;
}
