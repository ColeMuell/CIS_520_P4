#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // C


#define NUM_THREADS 4

#define ARRAY_SIZE 2000000
#define MAX_STRING_SIZE 2001
#define ALPHABET_SIZE 26
#define BATCH_SIZE 1000

pthread_mutex_t mutexmax;			// mutex for char_max





char* lines[BATCH_SIZE]; 
int max_ascii[BATCH_SIZE];

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

//Finds the max ascii value in a given line
int find_max(const char* line, int length) {
    int i;
    int max_val = 0;
    
    for (i = 0; i < length; i++) {

        int char_val = (int)line[i];
        printf("%c",line[i]);
        if(char_val == (int)'\0')
        {
            break;
        }

        if (char_val > max_val) {
            max_val = char_val;
        }
    }
    printf("\n");
    
    return max_val;
}


// Reading lines into memory, batched so it only does the batch
int read_file(FILE* fd) {
    
    char buffer[MAX_STRING_SIZE];
    int count = 0;

    while (count < BATCH_SIZE && fgets(buffer, MAX_STRING_SIZE, fd)) {
        size_t len = strnlen(buffer, MAX_STRING_SIZE);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        char* copy = malloc(len + 1);
        
        if (!copy) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(1);
        }
        snprintf(copy, len + 1, "%s", buffer); 
        lines[count] = copy;

        count++;
    }

    return count;
}

//Modify to use file io instead of random data
void *count_array(void *myID)
{
  char theChar;
  int i, j, charLoc;
  

  int startPos = (uintptr_t) myID * (BATCH_SIZE / NUM_THREADS);
  int endPos = startPos + (BATCH_SIZE / NUM_THREADS);

  int local_max[endPos - startPos];

  printf("myID = %d startPos = %d endPos = %d \n", (uintptr_t) myID, startPos, endPos);

					// init local count array
  for ( i = 0; i < endPos - startPos; i++ ) {
  	local_max[i] = 0;
  }
					// count up our section of the global array
  for ( i = startPos; i < endPos; i++) {
        local_max[i - startPos] = find_max(lines[i], MAX_STRING_SIZE);
	}
  
					// add local maxes into the global arrays
  pthread_mutex_lock (&mutexmax);
  for ( i = startPos; i < endPos; i++ ) {
     max_ascii[i] = local_max[i - startPos];
  }
  pthread_mutex_unlock (&mutexmax);

  pthread_exit(NULL);
}



//prints results. The param offset says how far off of 0 the batched lines are
void print_results(int offset)
{
  int i,j, total = 0;
  					// prints out maxes
  for ( i = 0; i < BATCH_SIZE; i++ ) {
     printf(" Line %d: %d\n", i + offset, max_ascii[i]);
  }
}



int main() {

    //initialize some variables+
    uintptr_t i = 0;
	int rc, total_lines, lines_in_batch = 0;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;

    const char* filepath = "/homes/dan/625/wiki_dump.txt";

    //opens the file for reading+
    FILE* fd = fopen(filepath, "r");
    if (!fd) {
        perror("Error opening file");
        exit(1);
    }

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    //reads files 1000 lines at a time, which is batched.
    while((lines_in_batch = read_file(fd)) > 0){
        for (i = 0; i < NUM_THREADS; i++ ) {
            rc = pthread_create(&threads[i], &attr, count_array, (void *)i);
            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
            }
        }

        /* Free attribute and wait for the other threads */
        
        for(i=0; i<NUM_THREADS; i++) {
            rc = pthread_join(threads[i], &status);
            if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
            }
        }

        print_results(total_lines);      
        total_lines += lines_in_batch;
    }
	fclose(fd);

    pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutexmax);
	printf("Main: program completed. Exiting.\n");
    
	pthread_exit(NULL);
    return 0;
}
