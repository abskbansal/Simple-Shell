#Compile the launch.c by linking it with the lib_simpleloader.so
# Compile loader.c into lib_simpleloader.so

CC = gcc
CFLAGS = -m32

all: loader fib

loader: loader.c loader.h
	$(CC) $(CFLAGS) -o loader loader.c

fib: fib.c
	$(CC) $(CFLAGS) -no-pie -nostdlib -o fib fib.c


#Provide the command for cleanup
clean:
	@rm -f loader fib

