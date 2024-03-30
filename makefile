# Uncomment and provide the paths to nauty and Eigen here.
# NAUTY_DIR=<path to nauty>
# EIGEN_DIR=<path to Eigen>

CC=gcc
CXX=g++
CFLAGS=-O4 -mpopcnt -march=native

.PHONY: all test clean

ifdef EIGEN_DIR
all: gensparseg filter_sparse filter_rank
else
all: gensparseg filter_sparse
endif

gensparseg: prunesparse.h
	$(CC) -o gensparseg ${CFLAGS} -I. -DMAXN=WORDSIZE \
	-D'PLUGIN="prunesparse.h"' ${NAUTY_DIR}geng.c ${NAUTY_DIR}nauty1.a

filter_sparse: filter_sparse.c
	$(CC) -o filter_sparse ${CFLAGS} -I${NAUTY_DIR} -DMAXN=32 \
	filter_sparse.c ${NAUTY_DIR}gtools.c

filter_rank: filter_rank.cpp
	$(CXX) -o filter_rank ${CFLAGS} -I${EIGEN_DIR} filter_rank.cpp

test: gensparseg filter_sparse
	./run_known_tests && ./run_sparse_tests

clean:
	rm -f gensparseg filter_sparse filter_rank
