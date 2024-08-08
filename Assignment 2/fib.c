/*
 * No changes are allowed in this file
 */
#include <stdio.h>
#include <stdlib.h> // Include this header for atoi()

int fib(int n) {
  if (n < 2) return n;
  else return fib(n - 1) + fib(n - 2);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <n>\n", argv[0]);
    return 1; // Exit with an error code
  }

  int n = atoi(argv[1]); 

  int val = fib(n);
  printf("%d\n", val);

  return 0; 
}

