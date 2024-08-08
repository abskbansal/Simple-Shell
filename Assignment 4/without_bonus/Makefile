CC = gcc
CFLAGS = -m32

TARGETS = prime sum sum_better loader fib

all: $(TARGETS)

prime: prime.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o prime prime.c

sum: sum.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o sum sum.c

sum_better: sum_better.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o sum_better sum_better.c

loader: loader.c loader.h
	$(CC) $(CFLAGS) -o loader loader.c

fib: fib.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o fib fib.c

# Provide the command for cleanup
clean:
	@rm -f $(TARGETS)



