CC      := gcc
CFLAGS  := -std=c11 -fopenmp -Wall -Wpedantic -fsanitize=address -O3 -g
LDFLAGS := -lm

SRCS = $(wildcard *.c)
TARGETS = $(basename $(SRCS))

all: $(TARGETS)

# 各 .c → 実行ファイル（同名）にコンパイル
$(TARGETS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

