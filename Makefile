CC      := gcc
CFLAGS  := -std=c11 -fopenmp -Wall -Wpedantic -fsanitize=address -O3 -g
LDFLAGS := -lm

SRCS = $(wildcard *.c)
TARGETS = $(basename $(SRCS))

.PHONY: build

build:
	$(CC) $(CFLAGS) -o ray_tracing ray_tracing.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o ray_tracing_comb_omp ray_tracing_comb_omp.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o ray_tracing_comb ray_tracing_comb.c $(LDFLAGS)

run: build
	./ray_tracing
	./ray_tracing_comb_omp
	./ray_tracing_comb
