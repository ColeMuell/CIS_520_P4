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

CC = gcc

#
# Modify TOPDIR if you use your own include files and library files
#

PROGRAM   = pth-imp			# name of the binary
SRCS      = pth-imp.c		# source file
OBJS      = $(SRCS:.c=.o)		# object file

#
# Targets
#

default: all

all: $(PROGRAM) 

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) $(LDFLAGS)

clean:
	/bin/rm -f $(OBJS) $(PROGRAM)
