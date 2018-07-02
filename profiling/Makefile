all: profiling
spy: profiling.c ../cacheutils.h
	gcc -std=gnu11 -O2 -o $@ $< -lrt -pthread
