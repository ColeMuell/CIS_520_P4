#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024
#define BATCH_SIZE 10000

int max_ascii_in_line(const char *line) {
    int max_val = 0;
    for (int i = 0; line[i] != '\0' && line[i] != '\n'; ++i)
        if ((unsigned char)line[i] > max_val)
            max_val = (unsigned char)line[i];
    return max_val;
}

int main(int argc, char *argv[]) {
    int rank, size;
    FILE *fp = NULL;
    int global_line_num = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    char (*buffer)[MAX_LINE_LEN] = NULL;
    if (rank == 0) {
        fp = fopen(argv[1], "r");

        if (fp == NULL) {
            perror("Error opening file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        buffer = malloc(BATCH_SIZE * MAX_LINE_LEN);
    }

    int *sendcounts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));

    while (1) {
        int lines_read = 0;

        if (rank == 0) {
            for (lines_read = 0; lines_read < BATCH_SIZE; ++lines_read) {
                if (!fgets(buffer[lines_read], MAX_LINE_LEN, fp))
                    break;
            }
        }

        MPI_Bcast(&lines_read, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (lines_read == 0) break;

        int offset = 0;
        for (int i = 0; i < size; ++i) {
            int count = lines_read / size + (i < (lines_read % size));
            sendcounts[i] = count * MAX_LINE_LEN;
            displs[i] = offset;
            offset += sendcounts[i];
        }

        int my_bytes = sendcounts[rank];
        int my_lines = my_bytes / MAX_LINE_LEN;

        char *my_buffer = malloc(my_bytes);
        int *my_results = malloc(my_lines * sizeof(int));

        MPI_Scatterv(rank == 0 ? buffer : NULL, sendcounts, displs, MPI_CHAR,
                     my_buffer, my_bytes, MPI_CHAR, 0, MPI_COMM_WORLD);

        for (int i = 0; i < my_lines; ++i)
            my_results[i] = max_ascii_in_line(&my_buffer[i * MAX_LINE_LEN]);

        int *recvcounts = NULL, *recvdispls = NULL, *batch_results = NULL;
        if (rank == 0) {
            recvcounts = malloc(size * sizeof(int));
            recvdispls = malloc(size * sizeof(int));
            batch_results = malloc(lines_read * sizeof(int));

            for (int i = 0, pos = 0; i < size; ++i) {
                recvcounts[i] = sendcounts[i] / MAX_LINE_LEN;
                recvdispls[i] = pos;
                pos += recvcounts[i];
            }
        }

        MPI_Gatherv(my_results, my_lines, MPI_INT,
                    batch_results, recvcounts, recvdispls, MPI_INT,
                    0, MPI_COMM_WORLD);

        if (rank == 0) {
            /*for (int i = 0; i < lines_read; ++i)
                printf("Line %d: %d\n", global_line_num + i + 1, batch_results[i]);
            */
            global_line_num += lines_read;
            free(recvcounts);
            free(recvdispls);
            free(batch_results);
        }

        free(my_buffer);
        free(my_results);
    }

    if (rank == 0) {
        fclose(fp);
        free(buffer);
    }

    free(sendcounts);
    free(displs);
    MPI_Finalize();
    return 0;
}