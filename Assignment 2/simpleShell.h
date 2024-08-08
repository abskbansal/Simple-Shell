#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <signal.h>
#include <time.h>
#include <bits/types/struct_timeval.h>
#include <sys/time.h>

#define MAX_ARGC 6
#define SH_TOKEN_BUFFSIZE 70
#define MAX_SIZE 2000

#define HISTORY_SIZE 2000
#define HISTORY_ERROR_CODE -1

void handler(int signum);
void shell_loop();
char *read_line();
char **splitting_function(char *command);

int cd_command(char ** args);
int pwd_command(char *pwd);

int write_history(char*);
int get_history(char*);
void clear_history();

int run_script(char* filename);