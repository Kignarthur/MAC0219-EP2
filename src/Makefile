OMPI=mandel_ompi

CC=mpicc
CC_OPT=-std=c11
CC_OMP=-fopenmp
CC_PTH=-pthread

SHELL=/bin/bash

.PHONY: all
all: $(OMPI)_seq $(OMPI)_pth $(OMPI)_omp

$(OMPI)_seq: $(OMPI)_seq.c
	$(CC) -o $(OMPI)_seq $(CC_OPT) $(OMPI)_seq.c

$(OMPI)_pth: $(OMPI)_pth.c
	$(CC) -o $(OMPI)_pth $(CC_OPT) $(CC_PTH) $(OMPI)_pth.c

$(OMPI)_omp: $(OMPI)_omp.c
	$(CC) -o $(OMPI)_omp $(CC_OPT) $(CC_OMP) $(OMPI)_omp.c

.PHONY: clean
clean:
	rm $(OMPI)_omp $(OMPI)_pth $(OMPI)_seq
