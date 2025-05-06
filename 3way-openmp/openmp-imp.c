#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>

#define MAX_STRING_SIZE 2200
#define BATCH_SIZE 15000

int total_read = 0;
size_t numThreads;

// Prints results. The param offset says how far off of 0 the batched lines are
void print_results(FILE* fout, int max_ascii[], int offset, int count) {
    for (int i = 0; i < count; i++) {
        //prints out to a given file
        fprintf(fout, "Line %d: %d\n", i + offset, max_ascii[i]);
    }
}

// Finds the max ascii value in a given line
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

// Kernel that computes max ASCII value per line using OpenMP. opm parallel for means that the for loop is automatically split between different threads
void kernel(char linesArray[][MAX_STRING_SIZE], int max_local[], int count) 
{
    //automatically optimizes out for the number of lines
    #pragma omp parallel for schedule(guided) num_threads(numThreads)
        for (int i = 0; i < count; i++) 
        {      
            max_local[i] = find_max(linesArray[i], MAX_STRING_SIZE);
        }
}

// Reads a batch of lines from file
int read_file(FILE* fd, char linesArray[][MAX_STRING_SIZE], size_t batchSize) {
    char buffer[MAX_STRING_SIZE];
    int count = 0;
    //reads in up to the batchsize and while there is still input to get in.
    while (count < batchSize && fgets(buffer, MAX_STRING_SIZE, fd)) {
        buffer[strcspn(buffer, "\n")] = 0;
        snprintf(linesArray[count], MAX_STRING_SIZE, "%s", buffer);
        count++;
    }
    total_read += count;
    return count;        
}

//main logic of the program, main node sends off to the worker nodes.
int main(int argc, char **argv) {

    if (argc < 3) 
	{
		printf("%s <file> <cores> <batches> <batchsize>", argv[0]);
		return EXIT_FAILURE;
	}

    char* filename = argv[1];
    //checking to make sure that the filename does not contain invalid chars.
    for (size_t i = 0; filename[i] != '\0'; i++) 
	{
        if (filename[i] == 60 ||filename[i] == 62 ||filename[i] == 58 ||filename[i] == 34 ||filename[i] == 92 ||filename[i] == 124 ||filename[i] == 63 ||filename[i] == 42)
            return 0;

		if(filename[i] < 31)
			return 0;

		if(i > 255)
			return 0;
    }

    //gest in the number of threads as the first argument
    numThreads = atoi(argv[2]);

    //makes sure there isn't too many or too few threads.
    if(numThreads > 100 || numThreads < 1)
    {
        printf("%d is an invalid number of threads\n", numThreads);
        return EXIT_FAILURE;
    }
    //gets in the number of threads as the second argument
    int batches = atoi(argv[3]);
    //makes sure there aren't too many batches or too few.
    if(batches > 1000 || batches < 1)
    {
        printf("%d is an invalid number of batches\n", batches);
        return EXIT_FAILURE;
    }
    //gets the batchsize as the third argument
    int batchSize = atoi(argv[4]);

    //if the batchsize is too big or too small, it will say there is an invalid size of batches.
    if(batchSize > 100000 || batchSize < 10)
    {
        printf("%d is an invalid size of batches\n", batchSize);
        return EXIT_FAILURE;   
    }

    //printf("Requested: %d\n", numThreads);

    FILE* fd = fopen("/homes/dan/625/wiki_dump.txt", "r");

    if (fd == NULL) 
    {
        printf(filename);
        perror("fopen Failed: ");
        return EXIT_FAILURE;
    }

    // Opens the file for output
    FILE* fout = fopen("./OpenMPOut.txt", "w");
    //if the file fails to open, show that it failed
    if (fout == NULL) 
    {
        printf("./OpenMPOut.txt");
        perror("fopen Failed for : ");
        return EXIT_FAILURE;
    }

    //alloate the lines array as well as the maximum_local array
    char (*lines)[MAX_STRING_SIZE] = malloc(sizeof(char) * batchSize * MAX_STRING_SIZE);
    int *max_local = malloc(sizeof(int) * batchSize);

    //if either of the mallocs fail, throw an error.
    if (lines == NULL || max_local == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    

    //gets the runtime
    int total_lines = 0;
    int read_lines;
    double t1 = omp_get_wtime();

    //While there are still files to read, master process reads in the files, then distributes it among the worker processes.
    while ((read_lines = read_file(fd, lines, batchSize)) > 0 && total_lines < batches * batchSize) 
    {
        kernel(lines, max_local, read_lines);
        print_results(fout, max_local, total_lines, read_lines);
        total_lines += read_lines;
    }
    //end time
    double t2 = omp_get_wtime();

    //printf("Total lines processed: %d\n", total_lines);
    //printf("Program finished in %.3lf seconds.\n", t2 - t1);
    //prints the times.
    printf("%lf, ", t2 - t1);
    fclose(fd);
    fclose(fout);
    free(lines);
    free(max_local);
    return 0;
}
