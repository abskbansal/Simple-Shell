
#include <bits/types/struct_timespec.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
#include <functional>
#include <pthread.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>

float total_duration = 0 ;


int user_main(int argc, char **argv);
/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}
typedef struct {
  int low ;
  int high ;
  std::function<void(int)> lambda_function  ;
} thread_args ;

typedef struct {
  int low1 ;
  int high1 ;
  int low2;
  int high2;
  std::function<void(int ,int)> lambda_function  ;
} thread_args2 ;

void helper_function(thread_args* t){

  for (int j = t->low ; j<t->high ;j++){
    t->lambda_function(j);
  }

}


void *thread_func(void *ptr){
  thread_args* t = ((thread_args*) ptr);
  
  helper_function(t);

  return NULL; 
}


void helper_function_2(thread_args2* t){
  for (int k = t->low1 ; k< t->high1 ;k++){
    for (int j = t->low2;j<t->high2 ; j++){
      t->lambda_function(k,j);
    }
    
  }
}


void *thread_func2(void *ptr){
  thread_args2* t = ((thread_args2*) ptr);
  helper_function_2(t);
  return NULL;
}
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
  // printf("hello1\n");
  struct timespec start_time , end_time ;

  clock_gettime(CLOCK_REALTIME, &start_time);
  pthread_t tid[numThreads];
  thread_args args[numThreads];
  int chunks = (high - low) / (numThreads);
  int left = (high - low) - (chunks * numThreads);
  for (int i = 0; i < numThreads; i++) {
    args[i].low = low + i * chunks;
    if (i != numThreads - 1) {
      args[i].high = low + (i + 1) * chunks;
    } else {
      args[i].high = low + (i + 1) * chunks + left;
    }
    
    args[i].lambda_function = lambda;
    if (pthread_create(&tid[i], NULL, thread_func, (void*)&args[i]) != 0) {
      perror("pthread create");
      exit(EXIT_FAILURE);
    }
  }
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(tid[i], NULL) != 0) {
      perror("pthread join");
      exit(EXIT_FAILURE);
    }
  }
  struct timespec endtime;
  clock_gettime(CLOCK_REALTIME, &end_time);
  long seconds = end_time.tv_sec - start_time.tv_sec ; 
  long nseconds = end_time.tv_nsec - start_time.tv_nsec;
    
  if (nseconds < 0 ){
    seconds -=1 ;
    nseconds += (1000000000);
  }
  total_duration += (seconds * 1000.0 + nseconds/1000000.0) ;

  printf("The elapsed time is: %f milliseconds\n " , seconds * 1000.0 + nseconds/1000000.0);
  
}
void parallel_for(int low1, int high1,  int low2, int high2, std::function<void(int, int)>  &&lambda, int numThreads){  
  
  struct timespec start_time , end_time ;
  clock_gettime(CLOCK_REALTIME, &start_time);
  pthread_t tid1[numThreads] ;
  thread_args2 args1[numThreads] ;
  int chunk1 = (high1 - low1) / (numThreads);
  int left1 = high1 - low1 - (numThreads * chunk1)  ;
  // printf("%d\n", left1);
  for (int i = 0; i<numThreads;i++){
    // printf("hello2\n");
    args1[i].low1 = low1 + i * chunk1;
    if (i != numThreads - 1) {
      args1[i].high1 = low1 + (i + 1) * chunk1;
    } else {
      args1[i].high1 = low1 + (i + 1) * chunk1 + left1;
    }
    
    // printf("h1\n");

    args1[i].low2 = low2 ; 
    args1[i].high2 = high2 ;
    args1[i].lambda_function = lambda;
    
    // printf("h3\n");
    if (pthread_create(&tid1[i], NULL, thread_func2, (void*)&args1[i]) != 0) {
      perror("pthreade create");
      exit(EXIT_FAILURE);
    }

    // printf("h4\n");
    
  }
  for (int i = 0; i < numThreads; i++) {
    
    if (pthread_join(tid1[i], NULL) != 0) {
      perror("pthread join");
      exit(EXIT_FAILURE);
    }
    // printf("h5\n");
  }
  struct timespec endtime;
  clock_gettime(CLOCK_REALTIME, &end_time);
  long seconds = end_time.tv_sec - start_time.tv_sec ; 
  long nseconds = end_time.tv_nsec - start_time.tv_nsec;
    
  if (nseconds < 0 ){
    seconds -=1 ;
    nseconds += (1000000000);
  }
  
  total_duration += (seconds * 1000.0 + nseconds/1000000.0) ;
  printf("The elapsed time is: %f milliseconds\n " , seconds * 1000.0 + nseconds/1000000.0);

}
int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5
  int rc = user_main(argc, argv);
  
  // printf("h1\n");
  auto /*name*/ lambda2 = [/*nothing captured*/]() {

    printf("the total duration is: %f milliseconds\n ", total_duration);
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}
#define main user_main
