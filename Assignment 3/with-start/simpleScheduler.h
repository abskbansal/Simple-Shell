#include <bits/types/timer_t.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <bits/types/struct_timeval.h>
#include <sys/time.h>
#include <semaphore.h>
#include <bits/time.h>
#include <bits/types/sigevent_t.h>
#include <bits/types/struct_itimerspec.h>
#include <linux/limits.h>
#include <stdint.h>

#define MAX_ARGC 6
#define SH_TOKEN_BUFFSIZE 70
#define MAX_SIZE 2000

#define HISTORY_SIZE 2000
#define HISTORY_ERROR_CODE -1

#define MAX_QUEUE_SIZE 100
#define MAX_PRIORITY 4
#define SLEEP_TIME 15000 // milliseconds

typedef struct {
    pid_t pid;
    char command[10];
    int priority;
    char* STATE;
    double executime_time;
    double waiting_time;
    int flag_printed;
    struct timespec start_time ;
    struct timespec end_time ;    
    struct timespec stopping_time ;
    struct timespec running_again_time ;
} Process;

typedef struct {
    int fd;
    char memory_name[128];
} shm_t;

typedef struct {
    int number;
    char command[10];
    pid_t pid ;
    double executime_time ;
    double waiting_time ;
} history ;

typedef struct {
    Process process_table[MAX_PRIORITY][MAX_QUEUE_SIZE];
    int process_index[MAX_PRIORITY];
    Process processes_running[MAX_QUEUE_SIZE];
    int start_running;
    shm_t* shm;
    int command_number; 
    history compelete_history[MAX_QUEUE_SIZE];
    int counter ;
    int num_procs ;

} SharedMemory;

typedef struct {
    struct sigevent sev;
    struct itimerspec* its;
    timer_t timerid;
} Timer;

// shell functions
void shell_loop(void);
char *read_line();
char **splitting_function(char *command);

// extra commands
int cd_command(char ** args);
int pwd_command(char *pwd);
void update_prompt(char* pwd);

// handlers
void handler_exit(int signum);
void timer_callback();

// process functions
void process_info(Process process);
void show_all_processes();

// scheduler functions
void run_scheduler();
int run_scheduled();
int run_n_processes(int n);
void cleanup();

// queue functions
int enqueueProcess(Process p);
Process dequeueProcess();
int queueIsEmpty();

// shared memory
SharedMemory* setup_shared_memory(int NCPU);
void cleanup_shared_memory(SharedMemory* shared_memory);
