#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // C


#define NUM_THREADS 4
#define ARRAY_SIZE 2000000
#define MAX_STRING_SIZE 2200
//?
#define ALPHABET_SIZE 26
#define BATCH_SIZE 1000
#define THREAD_COUNT 20
#define TARGET_LINE 1000

pthread_mutex_t mutexmax;			// mutex for char_max


int max_ascii[BATCH_SIZE];
uintptr_t rounds = 0;
char **lines;
int total_read;


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

//read 1000 lines at a time
//so loop 1000 times
//after reading the 1000 lines
//20 threads
//each thread reads 50 lines
//print out 1000 results in the end

// Reading lines into memory, batched so it only does the batch
int read_file(FILE* fd) {
    
    char buffer[MAX_STRING_SIZE];
    int count = 0;
    size_t len;
  
    //     while (fgets(buffer, sizeof(buffer), fd) ) {
    //         buffer[strcspn(buffer, "\n")] = 0;
    //         lines[count] = strdup(buffer);
    //         // snprintf(copy, MAX_STRING_SIZE, "%s", buffer);
    //     count++;
    // }

        while (fgets(buffer, MAX_STRING_SIZE, fd) && count < BATCH_SIZE) {
       
                buffer[strcspn(buffer, "\n")] = 0;
                
                lines[count] = strdup(buffer);
                if(total_read >= 999000){
                    printf("%s",buffer);
                }
                
               
                count++;
        }
        total_read +=count;
        return count;


        
}


//Modify to use file io instead of random data
void *count_array(void *myID) {
    // printf("my id is %d\n", myID);
    uintptr_t id = (uintptr_t)myID;
    
    int startPos = id * 50 ;
    int endPos = startPos + 50;
    
    for (int i = startPos; i < endPos; i++) {
       
            if(! lines[(int) i]) {pthread_exit(NULL);};
            max_ascii[i] = find_max(lines[(int)i], MAX_STRING_SIZE);
    }
    
   
    pthread_exit(NULL);
}



//prints results. The param offset says how far off of 0 the batched lines are
void print_results(int offset, int count) {
    int i;
    // prints out maxes
    printf("offset %d count %d\n",offset, count);

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


int main() {

    uintptr_t i = 0;
    int rc, total_lines = 0, lines_in_batch = 0;
    pthread_t threads[THREAD_COUNT];
    pthread_attr_t attr;
    void *status;

    
    
    //const char* filepath = "/homes/dan/625/wiki_dump.txt";
    
    // Initialize mutex
    if (pthread_mutex_init(&mutexmax, NULL) != 0) {
        printf("Mutex initialization failed\n");
        return 1;
    }
    
    // Opens the file for reading
    FILE* fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
    if (!fd) {
        perror("Error opening file");
        exit(1);
    }
    
    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
   
        


        // Reset max_ascii array for new batch
        memset(max_ascii, 0, sizeof(max_ascii));
        // return 0;
        int lines_read = 0;
        
        for(int k = 0;k < 1000;k++){
            lines = (char **)calloc(BATCH_SIZE,  sizeof(char *));
             int read = read_file(fd);
            
             printf("read file again %d",read);
             printf("total read %d\n",total_read);
                for (i = 0; i < THREAD_COUNT; i++) {
                    printf("new thread %d\n",i);
                    uintptr_t id = (k * THREAD_COUNT) + i;

                    rc = pthread_create(&threads[i], &attr, count_array, (void *)i);
                    // printf("I created thread %d\n",i);
                    if (rc) {
                        printf("ERROR; return code from pthread_create() is %d\n", rc);
                        exit(-1);
                    }
                }

                for (i = 0; i < THREAD_COUNT; i++) {
                    rc = pthread_join(threads[i], &status);
                    //printf("I joined thread %d\n",i);
                    if (rc) {
                        printf("ERROR; return code from pthread_join() is %d\n", rc);
                        exit(-1);
                    }
                }
                
                printf("printing results  %d\n", rounds) ;
               
                print_results(lines_read, BATCH_SIZE);
                 lines_read += BATCH_SIZE;
                
                // Free memory before processing next batch
                free_lines();
                
                rounds +=1;

        }
        
        // total_lines += lines_in_batch;
 
    // }
    
    fclose(fd);
    
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutexmax);
    printf("Main: program completed. Exiting.\n");
    
    pthread_exit(NULL);
    return 0;
}