#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAX_STRING_SIZE 2001

pthread_mutex_t mutexmax;			// mutex for char_max
uint8_t numThreads;
size_t batches;
size_t batchSize;

char** lines; 
int* max_ascii;

//Finds the max ascii value in a given line
char find_max(const char* line, int length) 
{
    int max_ascii = -1; 
    int len = strnlen(line, length);

    for (int i = 0; i < len; i++) 
        if (line[i] > max_ascii) 
            max_ascii = line[i];

    return max_ascii;
}

// Reading lines into memory, batched so it only does the batch
int read_file(FILE* fd) 
{
    char buffer[MAX_STRING_SIZE];
    int count = 0;
    size_t len;
    
    while (count < batchSize && fgets(buffer, MAX_STRING_SIZE, fd) != NULL) {
        //print_string_ascii(buffer);
        len = strnlen(buffer, MAX_STRING_SIZE);
        
        if (len > 0 && buffer[len - 1] == '\n') {
            //printf("Line 75");
            buffer[len - 1] = '\0';
            len--;
        }
        if(len == 0) {
            //printf("line 80");
            continue;
        }
        
        char* copy = malloc(len + 1);
        
        if (!copy) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(1);
        }
        snprintf(copy, len + 1, "%s", buffer);
        //print_string_ascii(copy);
        lines[count] = copy;
        count++;
    }
    
    return count;
}

//Modify to use file io instead of random data
void *count_array(void *myID) {
    int i;
    uintptr_t id = (uintptr_t)myID;
    int startPos = id * (batchSize / numThreads);
    int endPos = startPos + (batchSize / numThreads) - 1;
    int segment_size = endPos - startPos + 1;

    // Make sure endPos doesn't exceed batchSize
    if (endPos > batchSize) 
        endPos = batchSize;

    int local_max[segment_size]; 

    // Calculate max values for our section
    for (i = 0; i < segment_size; i++) 
    {
        if (startPos + i < batchSize && lines[startPos + i] != NULL) 
        {
            char max = find_max(lines[startPos + i], MAX_STRING_SIZE);
            local_max[i] = max;
        }
    }
    
    // Add local maxes into the global arrays
    pthread_mutex_lock(&mutexmax);

    //Gets the local max values into the global array.
    for (i = 0; i < segment_size; i++) 
        max_ascii[startPos + i] = local_max[i];

    pthread_mutex_unlock(&mutexmax);
    
    pthread_exit(NULL);
}

//frees the memory of all the local lines array.
void free_lines() 
{
    int i;

    for (i = 0; i < batchSize; i++) 
    {
        if (lines[i] != NULL) 
        {
            free(lines[i]);
            lines[i] = NULL;
        }
    }
}


 //prints results. The param offset says how far off of 0 the batched lines are
 void print_results(FILE* fout, int offset, int count) 
 {
     int i;
     // prints out maxes
     for (i = 0; i < count; i++)
         fprintf(fout, "Line %d: %d\n", i + offset, max_ascii[i]);

 }

//main function that controls the logic of the program
int main(int argc, char **argv) {

    if (argc < 2) 
	{
		printf("%s <file> <cores> <batches> <batchsize>", argv[0]);
		return EXIT_FAILURE;
	}
    //gets the filename as the first argument
    char* filename = argv[1];

    //ensures the filename doesn't contain an invalid character
    for (size_t i = 0; filename[i] != '\0'; i++) 
	{
        if (filename[i] == 60 || filename[i] == 62 || filename[i] == 58 ||
            filename[i] == 34 || filename[i] == 92 || filename[i] == 124 ||
            filename[i] == 63 || filename[i] == 42 || filename[i] < 31 ||
            i > 255)
        {
            printf("%s is an invalid file name\n", filename);
            return EXIT_FAILURE;
        }
    }
    //gets the requested number of threads as the second argument
    numThreads = atoi(argv[2]);

    if(numThreads > 100 || numThreads < 1)
    {
        printf("%d is an invalid number of threads\n", numThreads);
        return EXIT_FAILURE;
    }

    //gets the number of batches for the third argument and checks
    batches = atoi(argv[3]);

    if(batches > 1000 || batches < 1)
    {
        printf("%d is an invalid number of batches\n", batches);
        return EXIT_FAILURE;
    }

    //gets the batch size as the fourth argument and checks
    batchSize = atoi(argv[4]);

    if(batchSize > 10000 || batchSize < 10)
    {
        printf("%d is an invalid size of batches\n", batchSize);
        return EXIT_FAILURE;   
    }

    //mallocs the local lines array and an array for storing it all
    lines = malloc(batchSize*sizeof(char*)); 
    max_ascii = malloc(batchSize*sizeof(int));


    uintptr_t i = 0;
    int rc, total_lines = 0, lines_in_batch = 0;
    pthread_t threads[numThreads];
    pthread_attr_t attr;
    void *status;
    
    // Initialize mutex
    if (pthread_mutex_init(&mutexmax, NULL) != 0) {
        printf("Mutex initialization failed\n");
        return 1;
    }
    
    // Opens the file for reading
    FILE* fd = fopen("/homes/dan/625/wiki_dump.txt", "r");

    if (fd == NULL) 
    {
        printf(filename);
        perror(" fopen Failed for : ");
        return EXIT_FAILURE;
    }

    // Opens the file for output
    FILE* fout = fopen("./PthreadOut.txt", "w");

    if (fout == NULL) 
    {
        printf("./PthreadOut.txt");
        perror("fopen Failed for : ");
        return EXIT_FAILURE;
    }
    
    // initializes and sets the detached state
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    // Initialize lines array
    for (i = 0; i < batchSize; i++)
        lines[i] = NULL;

    clock_t timeOne = clock();
    
    // Reads files 1000 lines at a time, which is batched
    while ((lines_in_batch = read_file(fd)) > 0 && total_lines < batches * batchSize)
    {
        // Reset max_ascii array for new batch
        memset(max_ascii, 0, sizeof(max_ascii));
        
        for (i = 0; i < numThreads; i++) 
        {
            rc = pthread_create(&threads[i], &attr, count_array, (void *)i);
            //printf("I created thread %d\n",i);
            if (rc) 
            {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
        
        /* Wait for the threads */
        for (i = 0; i < numThreads; i++) 
        {
            rc = pthread_join(threads[i], &status);

            if (rc) 
            {
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        
        print_results(fout, total_lines, lines_in_batch);
        
        // Free memory before processing next batch
        free_lines();
        
        total_lines += lines_in_batch;
    }

    clock_t timeTwo = clock();
    //calculates the time difference
    long double timeDiff = (long double)(timeTwo - timeOne);
    
    //cleans the memory and resets everything
    fclose(fd);
    fclose(fout);
    free(lines);
    free(max_ascii);

    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutexmax);
    printf("%Lf, ", timeDiff / CLOCKS_PER_SEC);

    pthread_exit(NULL);
    return 0;
} 