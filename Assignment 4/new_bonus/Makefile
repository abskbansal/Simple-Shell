#Compile the launch.c by linking it with the lib_simpleloader.so
# Compile loader.c into lib_simpleloader.so

CC = gcc
CFLAGS = -m32

all: CC = gcc
CFLAGS = -m32

TARGETS = sum sum_better fib prime loader_bonus

all: $(TARGETS)

prime: prime.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o prime prime.c

sum: sum.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o sum sum.c

sum_better: sum_better.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o sum_better sum_better.c

fib: fib.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o fib fib.c

loader_bonus: loader_bonus.c
	$(CC) $(CFLAGS) -o loader_bonus loader_bonus.c

clean:
	@rm -f $(TARGETS)






