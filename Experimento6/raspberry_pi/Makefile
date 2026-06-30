CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all clean

all: calculadora benchmark

calculadora: calculadora.c
	$(CC) $(CFLAGS) -o $@ $<

benchmark: benchmark.c
	$(CC) $(CFLAGS) -o $@ $< -lm

clean:
	rm -f calculadora benchmark
