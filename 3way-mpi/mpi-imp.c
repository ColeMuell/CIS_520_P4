#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>   
#include <sys/stat.h> 
#include <sys/mman.h>  
#include <unistd.h> 

#define LINE_MAX 1000001

int numThreads;
int numLines = 0;
char* mFile;
off_t fSize;
off_t lineOff[LINE_MAX];

int* max_ascii;
int* rankMax;

int numReq = -1;

void read_file() {
	// open file
	int fd = open("/homes/dan/625/wiki_dump.txt", O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

	// get file info using fstat()
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat failed");
        exit(EXIT_FAILURE);
    }
    fSize = sb.st_size;

	// map file into memory
	// had to rework logic from previous 'line-by-line' code
    mFile = mmap(NULL, fSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mFile == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // need to start scanning mapped file to find where each line begins. (Line 0 will always start at 0)
    lineOff[numLines++] = 0;

	// loop through file byte by byte
    for (off_t i = 0; i < fSize; i++) {
        if (mFile[i] == '\n') {
            if (numLines < LINE_MAX) {
                lineOff[numLines++] = i + 1;
                if (numReq > 0 && numLines >= numReq) // used for varying input lines testing
                {
                    break;
                }
            } 
            else 
            {
                fprintf(stderr, "Reached max line limit (%d)\n", LINE_MAX);
                break;
            }
        }
    }
	// lineOff holds the starting byte positions of each line in the memory-mapped file.

    close(fd);
}

void *max_per_line(void *rank)
{
    int myID =  *((int*) rank);

    int startPos = ((long) myID) * (numLines / numThreads); // (numLines / numThreads) gives the number of lines each MPI process should handle
														      // (int) myID) identifies which process we are dealing with
														      // say numLines = 1000, each process handles 1000/4 or 250 lines
														      // More specifically process with rank 0 handles lines 0 to 249, process 1 handles lines 250 to 499, and so on
    int endPos = startPos + (numLines / numThreads); // Number of lines per thread

    printf("myID = %d startPos = %d endPos = %d \n", myID, startPos, endPos); fflush(stdout);

	// Allocate rankMax[] to hold max for each of my lines for this rank
    rankMax = (int*)calloc(sizeof(int), (numLines / numThreads));
    
    if (rankMax == NULL) {
        perror("Allocation Failed");
        exit(EXIT_FAILURE);
    }

    for (int i = startPos; i < endPos; i++) // for all of the lines this process is handling
    {
	    int lineMax = 0;		//set max to 0
	    off_t startingByte = lineOff[i]; // starting byte for this line
	    off_t endingByte;
	    if (i == numLines - 1) // on the last line of the file
	    {
		    endingByte = fSize; // lineOff[numLines] doesn't work, would be out of bounds
	    }
	    else
	    {
		    endingByte = lineOff[i + 1]; // the ending byte of this line is the starting byte of the next line
	    }

	    for (off_t n = startingByte; n < endingByte; n++) // for each byte of this line
	    {
		    if((int)mFile[n] > lineMax) // mFile or memory mapped I/O is indexed by bytes
		    {
			    lineMax = (int)mFile[n];
		    }

	    }
	    rankMax[i - startPos] = lineMax; // i is global line index, rankMax just goes from 0 to numLines / numThreads - 1
    }
    return NULL;
}

void print_results()
{
	//int limit = numLines > 10 ? 10 : numLines;
	for (int m = 0; m < numLines; m++ )
	{
		printf("%d: %d\n", m, max_ascii[m]);
	}
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
	{
		printf("%s <file> <lines>", argv[0]);
		return EXIT_FAILURE;
	}

    char* filename = argv[1];

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

    numReq = atoi(argv[2]);

    if(numReq > LINE_MAX || numThreads < 10)
    {
        printf("%d is an invalid number of lines\n", numReq);
        return EXIT_FAILURE;
    }

	int rc;
	int numtasks, rank;

	rc = MPI_Init(&argc,&argv); // initialize MPI
	if (rc != MPI_SUCCESS) 
	{
	    printf ("Error withMPI_Init.\n");
        MPI_Abort(MPI_COMM_WORLD, rc); // Terminates all MPI processes associated with the communicator
    }

    MPI_Comm_size(MPI_COMM_WORLD,&numtasks); // represents the number of MPI tasks available to your application, get number of tasks
    MPI_Comm_rank(MPI_COMM_WORLD,&rank); // Returns the rank of the calling MPI process within the specified communicator, get my rank

	numThreads = numtasks;
	//printf("I am %d of %d\n", rank, numtasks);
	//fflush(stdout);

	if ( rank == 0 ) 
    {
		read_file();
        max_ascii = (int*)calloc(sizeof(int), numLines);
        
        if (max_ascii == NULL) 
        {
            perror("Allocation Failed");
            exit(EXIT_FAILURE);
        }
	}
	
    MPI_Bcast(&numLines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    long fSize_long = (long)fSize;
    MPI_Bcast(&fSize_long, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    MPI_Bcast(lineOff, numLines, MPI_LONG, 0, MPI_COMM_WORLD);

    if (rank != 0) // rank 0 already has its local copy of fSize defined, because it was defined in read_file()
                   // we have to reset fSize to the off_t type because every other rank does not have the fSize defined in read_file() as an off_t
    {
        fSize = (off_t)fSize_long;

        // open file
	    int fd = open("/homes/dan/625/wiki_dump.txt", O_RDONLY);

        if (fd == -1) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        // map file into memory for each rank other than rank 0, this was done above in read_file() for rank 0
        mFile = mmap(NULL, fSize, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mFile == MAP_FAILED) 
        {
            perror("mmap failed");
            exit(EXIT_FAILURE);
        }

        close(fd);
    }
    
	max_per_line(&rank); // do analysis work to find max ASCII per line

    // gather back all the results from all the ranks into rank 0
    MPI_Gather(rankMax, numLines / numThreads, MPI_INT, max_ascii, numLines / numThreads, MPI_INT, 0, MPI_COMM_WORLD);
    
    /////////////////////////////

	if ( rank == 0 ) {
		print_results();
        free(max_ascii);
	}

    free(rankMax);

    int unmapResult = munmap(mFile, fSize);
	if (unmapResult != 0)
	{
		perror("munmap failed");
        exit(EXIT_FAILURE);
	}

	printf("Main: program completed. Exiting.\n");

    // done with MPI
	MPI_Finalize(); // Terminates the MPI execution environment
	return 0;
}