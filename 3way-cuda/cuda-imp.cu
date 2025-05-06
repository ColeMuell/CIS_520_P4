#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <cuda_runtime.h>

#define MAX_STRING_SIZE 2200
#define BATCH_SIZE 1000

int total_read = 0;

// Function to check CUDA errors
void checkCUDAError(cudaError_t err, const char *msg) {
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA error: %s: %s\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

// Kernel function to find the max ASCII character per line
__global__ void findMaxASCII(int *d_out, char *d_in, int lines, int max_string_size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= lines) return;

    char *line = &d_in[idx * max_string_size];
    int local_max = -1;

    for (int i = 0; i < max_string_size; i++) {
        char c = line[i];
        if (c == '\0') break;
        if (c > local_max) {
            local_max = c;
        }
    }

    d_out[idx] = local_max;
}

// Reads a batch of lines from a file
int readFile(FILE* fd, char linesArray[][MAX_STRING_SIZE]) {
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

// Prints results
void printResults(int* results, int totalLines) {
    for (int i = 0; i < totalLines; i++) {
        printf("Line %d: %d\n", i, results[i]);
    }
}

// Main function with dynamic thread/block configuration
int main(int argc, char *argv[]) {

    int threads_per_block = atoi(argv[1]);
    int blocks_per_grid = atoi(argv[2]);

    FILE* fd = fopen("/homes/dan/625/wiki_dump.txt", "r");

    char (*h_lines)[MAX_STRING_SIZE] = (char(*)[MAX_STRING_SIZE])malloc(BATCH_SIZE * MAX_STRING_SIZE * sizeof(char));
    int *h_max = (int*)malloc(BATCH_SIZE * sizeof(int));

    char *d_lines;
    int *d_max;

    checkCUDAError(cudaMalloc((void**)&d_lines, BATCH_SIZE * MAX_STRING_SIZE * sizeof(char)), "malloc");
    checkCUDAError(cudaMalloc((void**)&d_max, BATCH_SIZE * sizeof(int)), "malloc");

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    dim3 dimBlock(threads_per_block);
    dim3 dimGrid(blocks_per_grid);

    int totalLines = 0, readLines;
    
    while ((readLines = readFile(fd, h_lines)) > 0) {
        checkCUDAError(cudaMemcpy(d_lines, h_lines, readLines * MAX_STRING_SIZE * sizeof(char), cudaMemcpyHostToDevice), "memcpy");
        findMaxASCII<<<dimGrid, dimBlock>>>(d_max, d_lines, readLines, MAX_STRING_SIZE);
        checkCUDAError(cudaGetLastError(), "kernal");
        checkCUDAError(cudaDeviceSynchronize(), "synchronize");

        checkCUDAError(cudaMemcpy(h_max, d_max, readLines * sizeof(int), cudaMemcpyDeviceToHost), "memcpy");
        //printResults(h_max, readLines);
        totalLines += readLines;
    }

    // records end times
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

  //  printf("Total lines processed: %d\n", totalLines);
    //printf("Program finished in %.3f seconds.\n", milliseconds / 1000.0);
    printf("%.f", milliseconds / 1000.0);

    // Cleanup
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d_lines);
    cudaFree(d_max);
    free(h_lines);
    free(h_max);
    fclose(fd);

    return 0;
}