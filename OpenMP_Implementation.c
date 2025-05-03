#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_STRING_SIZE 2200
#define BATCH_SIZE 15000

int total_read = 0;

// Prints results. The param offset says how far off of 0 the batched lines are
void print_results(int max_ascii[], int offset, int count) {
    for (int i = 0; i < count; i++) {
        printf("Line %d: %d\n", i + offset, max_ascii[i]);
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
void kernel(char linesArray[][MAX_STRING_SIZE], int max_local[], int count) {
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {      
        max_local[i] = find_max(linesArray[i], MAX_STRING_SIZE);
    }
}

// Reads a batch of lines from file
int read_file(FILE* fd, char linesArray[][MAX_STRING_SIZE]) {
    char buffer[MAX_STRING_SIZE];
    int count = 0;

    while (count < BATCH_SIZE && fgets(buffer, MAX_STRING_SIZE, fd)) {
        buffer[strcspn(buffer, "\n")] = 0;
        snprintf(linesArray[count], MAX_STRING_SIZE, "%s", buffer);
        count++;
    }
    total_read += count;
    return count;        
}

int main(int argc, char **argv) {

    FILE* fd = fopen(argv[1], "r");

    char (*lines)[MAX_STRING_SIZE] = malloc(sizeof(char) * BATCH_SIZE * MAX_STRING_SIZE);
    int *max_local = malloc(sizeof(int) * BATCH_SIZE);

    if (lines == NULL || max_local == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    

    double t1 = omp_get_wtime();
    int total_lines = 0;
    int read_lines;

    while ((read_lines = read_file(fd, lines)) > 0) {
        kernel(lines, max_local, read_lines);
        print_results(max_local, total_lines, read_lines);
        total_lines += read_lines;
    }

    double t2 = omp_get_wtime();

    printf("Total lines processed: %d\n", total_lines);
    printf("Program finished in %.3lf seconds.\n", t2 - t1);
    fclose(fd);
    free(lines);
    free(max_local);
    return 0;
}
