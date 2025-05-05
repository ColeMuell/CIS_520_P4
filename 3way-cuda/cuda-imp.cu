#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <cuda_runtime.h>

#define MAX_STRING_SIZE 2200
#define BATCH_SIZE 1000
#define THREADS_PER_BLOCK 256
int total_read = 0;

// Prints results. The param offset says how far off of 0 the batched lines are
void print_results(int max_ascii[], int offset, int count) {
    for (int i = 0; i < count; i++) {
        printf("Line %d: %d\n", i + offset, max_ascii[i]);
    }
}

// Host function to find the max ascii value in a given line
char find_max_host(const char* line, int length) {
    int max_int = -1;
    
    int len = strnlen(line, length);
    for (int i = 0; i < len; i++) {
        if (line[i] > max_int) {
            max_int = line[i];
        }
    }
    return max_int;
}

// kernal for finding max value in a line
__global__ void find_max_kernel(char* d_lines, int* d_max, int count, int max_string_size) {
    int line_idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (line_idx < count) {
        char* line = &d_lines[line_idx * max_string_size];
        int max_int = -1;
    
        int len = 0;
        while (len < max_string_size && line[len] != '\0') {
            len++;
        }
        
        // Find max ASCII value in this line
        for (int i = 0; i < len; i++) {
            if (line[i] > max_int) {
                max_int = line[i];
            }
        }
        
        d_max[line_idx] = max_int;
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

// Check CUDA errors
void checkCudaError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        fprintf(stderr, "%s failed: %s\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {

    
    FILE* fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
    
    if (fd == NULL) {
        perror("fopen Failed: ");
        return EXIT_FAILURE;
    }
    
    // allocate all the host memory
    char (*h_lines)[MAX_STRING_SIZE] = (char(*)[MAX_STRING_SIZE])malloc(sizeof(char) * BATCH_SIZE * MAX_STRING_SIZE);
    int *h_max = (int*)malloc(sizeof(int) * BATCH_SIZE);
    
    //checking to see if the host memory fails
    if (h_lines == NULL || h_max == NULL) {
        fprintf(stderr, "Host memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    // allocate all the device memory
    char *d_lines;
    int *d_max;
    
    checkCudaError(cudaMalloc((void**)&d_lines, sizeof(char) * BATCH_SIZE * MAX_STRING_SIZE), 
                  "cudaMalloc for d_lines");
    checkCudaError(cudaMalloc((void**)&d_max, sizeof(int) * BATCH_SIZE), 
                  "cudaMalloc for d_max");
    
    int total_lines = 0;
    int read_lines;
    
    // Record timing
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    //important note to self, threads go into blocks, blocks go into grids. Usually want multiples of 32 threads, as that matches nicely with the GPU stuff
    
    while ((read_lines = read_file(fd, h_lines)) > 0) {
        // cpoy the batch from host to device
        checkCudaError(cudaMemcpy(d_lines, h_lines, sizeof(char) * read_lines * MAX_STRING_SIZE, 
                               cudaMemcpyHostToDevice), "cudaMemcpy h_lines to d_lines");
        
        // calculate number of blocks based on batch size
        int blocks = (read_lines + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        // Launch kernel, the execution config is how many blocks and how many threads
        find_max_kernel<<<blocks, THREADS_PER_BLOCK>>>(d_lines, d_max, read_lines, MAX_STRING_SIZE);
        
        // Check for kernel errors
        checkCudaError(cudaGetLastError(), "CUDA kernel launch");
        checkCudaError(cudaDeviceSynchronize(), "cudaDeviceSynchronize");
        
        // reuslsts to host
        checkCudaError(cudaMemcpy(h_max, d_max, sizeof(int) * read_lines, cudaMemcpyDeviceToHost), 
                      "cudaMemcpy d_max to h_max");
        
        // print reuslts
        print_results(h_max, total_lines, read_lines);
        total_lines += read_lines;
    }
    
    // Record end time and calculate elapsed time
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    printf("Total lines processed: %d\n", total_lines);
    printf("Program finished in %.3f seconds.\n", milliseconds / 1000.0);
    
    // Clean up
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d_lines);
    cudaFree(d_max);
    fclose(fd);
    free(h_lines);
    free(h_max);
    
    return 0;
}