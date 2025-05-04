#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // C
#include <time.h>



#define NUM_THREADS 20
#define ARRAY_SIZE 2000000
#define MAX_STRING_SIZE 2001
#define ALPHABET_SIZE 26
#define BATCH_SIZE 1000
pthread_mutex_t mutexmax;			// mutex for char_max





char* lines[BATCH_SIZE]; 
int max_ascii[BATCH_SIZE];
uint8_t numThreads;

/*char getRandomChar()
{
	int randNum = 0;
	char randChar = ' ';

	randNum = ALPHABET_SIZE * (rand() / (RAND_MAX + 1.0)); 	// pick number 0 < # < 25
	randNum = randNum + 97;				// scale to 'a'
	randChar = (char) randNum;

	// printf("%c", randChar);
	return randChar;
}*/

void print_string_ascii(const char* str) {
    printf("String: ");
    printf("%s", str);
}
//Finds the max ascii value in a given line
char find_max(const char* line, int length) {

    int max_ascii = -1; 
    int len = strnlen(line, length);
    for (int i = 0; i < len; i++) {
        if (line[i] > max_ascii) {
        max_ascii = line[i];
        }
    }


    
    return max_ascii;
}
float find_avg(char* line, int nchars) {
   int i, j;
   float sum = 0;

   for ( i = 0; i < nchars; i++ ) {
      sum += ((int) line[i]);
   }

   if (nchars > 0) 
	return sum / (float) nchars;
   else
	return 0.0;
}


// Reading lines into memory, batched so it only does the batch
int read_file(FILE* fd) {
    char buffer[MAX_STRING_SIZE];
    int count = 0;
    size_t len;
    
    while (count < BATCH_SIZE && fgets(buffer, MAX_STRING_SIZE, fd) != NULL) {
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
    int startPos = id * (BATCH_SIZE / numThreads);
    int endPos = startPos + (BATCH_SIZE / numThreads) - 1;
    int segment_size = endPos - startPos + 1;
    // Make sure endPos doesn't exceed BATCH_SIZE
    if (endPos > BATCH_SIZE) {
        endPos = BATCH_SIZE;
    }
    
    
    int local_max[segment_size]; 
    
    //printf("myID = %lu startPos = %d endPos = %d \n", id, startPos, endPos);
    
    // Calculate max values for our section
    for (i = 0; i < segment_size; i++) {
        if (startPos + i < BATCH_SIZE && lines[startPos + i] != NULL) {
            
            char max = find_max(lines[startPos + i], MAX_STRING_SIZE);
            local_max[i] = max;
        }
    }
    
    // Add local maxes into the global arrays
    pthread_mutex_lock(&mutexmax);
    for (i = 0; i < segment_size; i++) {
        max_ascii[startPos + i] = local_max[i];
    }
    pthread_mutex_unlock(&mutexmax);
    
    pthread_exit(NULL);
}



//prints results. The param offset says how far off of 0 the batched lines are
void print_results(int offset, int count) {
    int i;
    // prints out maxes
    for (i = 0; i < count; i++) {
        printf("Line %d: %d\n", i + offset, max_ascii[i]);
    }
}

void free_lines() {
    int i;
    for (i = 0; i < BATCH_SIZE; i++) {
        if (lines[i] != NULL) {
            free(lines[i]);
            lines[i] = NULL;
        }
    }
}


int main(int argc, char **argv) {

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
    FILE* fd = fopen(filename, "r");

    if (fd == NULL) 
    {
        perror("fopen Failed: ");
        return EXIT_FAILURE;
    }
    
    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    // Initialize lines array
    for (i = 0; i < BATCH_SIZE; i++) {
        lines[i] = NULL;
    }

    clock_t timeOne = clock();
    
    // Reads files 1000 lines at a time, which is batched
    while ((lines_in_batch = read_file(fd)) > 0) {
        // Reset max_ascii array for new batch
        memset(max_ascii, 0, sizeof(max_ascii));
        
        for (i = 0; i < numThreads; i++) {
            rc = pthread_create(&threads[i], &attr, count_array, (void *)i);
            //printf("I created thread %d\n",i);
            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
        
        /* Wait for the threads */
        for (i = 0; i < numThreads; i++) {
            rc = pthread_join(threads[i], &status);
            //printf("I joined thread %d\n",i);
            if (rc) {
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        
        //print_results(total_lines, lines_in_batch);
        
        // Free memory before processing next batch
        free_lines();
        
        total_lines += lines_in_batch;
    }

    clock_t timeTwo= clock();
    
    fclose(fd);
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutexmax);
    printf("%f, ", (float)(timeTwo - timeOne / CLOCKS_PER_SEC));
    
    pthread_exit(NULL);
    return 0;
}