CC = nvcc

CUDA_FLAGS = -O3 -arch=sm_60

PROGRAM = cuda-imp
SRCS = cuda-imp.cu
OBJS = $(SRCS:.cu=.o)

default: all

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) $(LDFLAGS)

$(OBJS): $(SRCS)
	$(CC) -c $(CUDA_FLAGS) $(SRCS)

clean:
	/bin/rm -f $(OBJS) $(PROGRAM)