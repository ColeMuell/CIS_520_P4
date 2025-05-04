#
# Open Systems Lab
# http://www.lam-mpi.org/tutorials/
# Indiana University
#
# MPI Tutorial
# Sample Makefile
#

# "mpicc" adds the directories for the include and lib files.  Hence,
# -I and -L for MPI stuff is not necessary
#

CC        = mpicc
CFLAGS = -fopenmp

#
# Modify TOPDIR if you use your own include files and library files
#

PROGRAM   = openmpi			# name of the binary
SRCS      = OpenMP_Implementation.c		# source file
#SRCS      = MPI_C_SAMPLE.c		# source file
OBJS      = $(SRCS:.c=.o)		# object file

#
# Targets
#

default: all

all: $(PROGRAM) 

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LDFLAGS)

clean:
	/bin/rm -f $(OBJS) $(PROGRAM)
