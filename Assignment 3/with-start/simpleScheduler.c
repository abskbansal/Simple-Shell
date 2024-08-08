#include "simpleScheduler.h"

int NCPU;      // no of CPU cores
int TSLICE;    // time slice in milliseconds
timer_t timerid;
SharedMemory* shared_memory;

struct timespec starting_time;

pid_t scheduler_pid;
Process dummy_process = {-1, "NULL", 0, "NULL"};

char PROMPT[1024] = "\033[1;33mshell_sh> \033[0m";

// main
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s NCPU TSLICE\n", argv[0]);
        return EXIT_FAILURE;
    }

    NCPU = atoi(argv[1]);
    TSLICE = atof(argv[2]); 
    
    printf("total number of CPU resources(NCPU): %d\ntime quantum (TSLICE in seconds): %d milliseconds\n", NCPU, TSLICE);

    shared_memory = setup_shared_memory(NCPU);  

    scheduler_pid = fork();
    if (scheduler_pid < 0) {
        perror("Unable to initiate scheduler");
        exit(EXIT_FAILURE);
    } else if (scheduler_pid == 0) {
        // scheduler process created, will run until the shell is killed (daemon process)
        int sr, processes_left=NCPU, fork1;
        while (1) {
            fork1 = fork();
            if (fork1 < 0 ){
                printf("error in the fork1");
            }
            else if(fork1 == 0) {
                signal(SIGINT, SIG_DFL);
                usleep(TSLICE * 1000);

                if (shared_memory->start_running) run_scheduled();
                exit(EXIT_SUCCESS);
            }
            else{
                wait(NULL);
            }
        }
    } else {
        // signal handling
        signal(SIGINT, handler_exit);

        // starting the shell loop
        shell_loop();
    }
    return EXIT_SUCCESS;
}

// shell functions
void shell_loop() {
    int status = 1;

    char *command;
    char **args;
    char pwd[1024];

    do {
        pwd_command(pwd);
        update_prompt(pwd);

        printf("%s", PROMPT);
        fflush(stdout);
        
        command = read_line();
        args = splitting_function(command);

        int args_count = 0;
        while (args[args_count] != NULL){
            args_count++;
        }

        int child_pid;
        int child_status = fork(), command_status;
        
        if (child_status < 0) {
            printf("the fork command failed !");
        } else if (child_status == 0){
            signal(SIGINT, SIG_DFL);
            if (strcmp(args[0], "cd") == 0) {
                if (cd_command(args) != 1) {
                    perror("Cannot run cd command with given arguements\n");
                } else {
                    pwd_command(pwd);
                    update_prompt(pwd);
                }
            } else if (strcmp(args[0],"submit")==0) {
                if (args_count < 2 || args_count > 3) {
                    printf("Usage: \"submit\" \"command\" \"priority(default 1)\"\n");
                    continue;
                } else if (args_count == 3 && (atoi(args[2]) < 1 || atoi(args[2]) > MAX_PRIORITY)) {
                    printf("Error. priority should be in between 1 and %d\n", MAX_PRIORITY);
                    continue;
                }

                
                

                Process p;
                for (int i =0 ; i<MAX_QUEUE_SIZE ;i++){
                    strcpy(shared_memory->processes_running[i].command, "");
                    shared_memory->processes_running[i].priority = 0;
                    shared_memory->processes_running[i].executime_time = 0;
                    shared_memory->processes_running[i].waiting_time = 0;
                    shared_memory->processes_running[i].pid = -1 ;
                    shared_memory->processes_running[i].STATE = "NEW";

                }
                
                p.pid = 0;
                strcpy(p.command, args[1]);
                p.priority = (args_count == 2) ? 1 : atoi(args[2]);
                p.STATE = "NEW";    
                p.flag_printed = 1;
                
                
                int enqueued = enqueueProcess(p);

                shared_memory->num_procs ++ ;
                if (enqueued == EXIT_FAILURE) {
                    printf("Cannot add more processes!\n");
                    continue;
                } else {
                    printf("Process added succesfully\n");
                    show_all_processes();
                }
            } else if (strcmp(args[0],"start") == 0) {
                // Execution is to start now
                shared_memory->start_running = 1;
            } else if (strcmp(args[0],"show") == 0) {
                // Show all processes in queue
                show_all_processes();
            } else {
                if (execvp(args[0], args) == -1) {
                        perror("execvp failed");
                    exit(EXIT_FAILURE);
                }
            }
        } else{
            waitpid(child_status, NULL , 0);
            free(args);
        }
    } while(status);
}

char *read_line() {
    char *command = NULL;
    size_t buffsize = 0;
    
    ssize_t read = getline(&command , &buffsize, stdin);

    if (read == -1){
        if (feof(stdin)){
            fprintf(stderr, "EOF\n");
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Value of the errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
    return command;
}

char **splitting_function(char *command) {
    int buffsize = SH_TOKEN_BUFFSIZE;
    int i= 0;
    char **tokens =(char** ) malloc(buffsize*sizeof(char *));

    char *token ;
    char *tokens_duplicate;

    if (!tokens){
        fprintf(stderr, "sh_shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(command , " \t\r\n\a");
    while (token != NULL){
        tokens[i++] = token;

        if (i >= buffsize){
            buffsize *= 2;
            tokens_duplicate = *tokens;
            tokens =(char** )realloc(tokens, buffsize *sizeof(char *));
            
            if (!tokens){
                free(tokens_duplicate);
                fprintf(stderr, "sh_shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t\r\n\a");
    }

    tokens[i] = NULL;
    return tokens;
}

// extra commands
int cd_command(char ** args) {
    if (args[1] == NULL) {
        return -1;
    } else {
        if (chdir(args[1]) != 0) {
            return 0;
        }
        return 1;
    }
}

int pwd_command(char pwd[]) {
    if (getcwd(pwd, 1024) != NULL) {
        return 1;
    } else {
        return 0;
    }
}

void update_prompt(char* pwd) {
    snprintf(PROMPT, sizeof(PROMPT), "\033[1;33mshell_sh: \033[34m%s\033[0m> ", pwd);
}

// handlers
void handler_exit(int signum) {
    if (!queueIsEmpty()) {
        printf("Running remaining processes\n");
        int fork1 = fork();
        if (fork1 < 0 ){
            printf("error in the fork1");
        }
        else if(fork1 == 0) {
            signal(SIGINT, SIG_DFL);
           

            run_scheduled();
            exit(EXIT_SUCCESS);
        }
        else{
            wait(NULL);
        }
        
    }

    printf("\nNUMBER  COMMAND_NAME  PID  EXECUTION_TIME(IN ms)  WAITING_TIME(IN ms)\n");
    for (int i = 0 ; i<shared_memory->counter ;i++){
        printf("%3d     %s            %d   %f                     %f\n", i+1 , shared_memory->compelete_history[i].command, shared_memory->compelete_history[i].pid , shared_memory->compelete_history[i].executime_time , shared_memory->compelete_history[i].waiting_time);
    }

    cleanup();
    exit(signum);
}

void timer_callback(){
    // stop the already running processes which are stored in processes_running array.
    // put these processes back in the queue
    // call run_n_processes() to run next NPCU processes

    Process p;
     if (!queueIsEmpty()) {
        for (int i=MAX_PRIORITY-1; i>=0; i--) {
            for (int j=0; j<shared_memory->process_index[i]; j++) {
                shared_memory->process_table[i][j].waiting_time += TSLICE;
            }
        }

    }

    for (int i=0; i<NCPU; i++) {
        p = shared_memory->processes_running[i];
        // printf("%f\n", p.waiting_time );
        // printf("%s\n",p.command);
        if (kill(p.pid, 0) != 0){

            // printf("%f\n", p.executime_time);
            // printf("gegegegegegegge\n");
            p.STATE ="TERMINATED" ;
            
            struct timespec endtime;
            clock_gettime(CLOCK_REALTIME, &p.end_time);
            long seconds = p.end_time.tv_sec - p.start_time.tv_sec ; 
            long nseconds = p.end_time.tv_nsec - p.start_time.tv_nsec;
    
            if (nseconds < 0 ){
                seconds -=1 ;
                nseconds += (1000000000);
            }

            if ( shared_memory->num_procs > 0 ) {
                strcpy(shared_memory->compelete_history[shared_memory->counter].command, p.command) ;
                shared_memory->compelete_history[shared_memory->counter].pid = p.pid ;
                shared_memory->compelete_history[shared_memory->counter].executime_time = p.executime_time ;
                shared_memory->compelete_history[shared_memory->counter].waiting_time = p.waiting_time ;

                shared_memory->counter++;
                shared_memory->num_procs--;
            } else if (shared_memory->num_procs == 0) {
                shared_memory->start_running = 0;
            }

            p.flag_printed = 0;

        }

        if (strcmp(p.STATE, "TERMINATED") == 0) {
            // process already terminated
            continue;
        }
        else if (strcmp(p.STATE,"NEW") == 0) {
            continue; 
        }
        else if (kill(p.pid, SIGTSTP) == 0) { 
            clock_gettime(CLOCK_REALTIME, &p.stopping_time);
            
            shared_memory->processes_running[i].STATE = "READY";
            enqueueProcess(shared_memory->processes_running[i]);
        
        } else {
            printf("Some error while stopping the running process with pid %d\n", p.pid);
            exit(EXIT_FAILURE);
        }
    }
    run_n_processes(NCPU);
}

// process functions
void process_info(Process process){
    printf("-----------------------------\n"); 
    printf("Process PID is:   %d\n",process.pid);
    printf("Command is:       %s\n",process.command);
    printf("Process Priority: %d\n",process.priority);
    printf("Process State:    %s\n",process.STATE);
    printf("-----------------------------\n"); 
}

void show_all_processes() {
    if (!queueIsEmpty()) {
        for (int i=MAX_PRIORITY-1; i>=0; i--) {
            for (int j=0; j<shared_memory->process_index[i]; j++) {
                process_info(shared_memory->process_table[i][j]);
            }
        }
    } else {
        printf("Queue empty. No process to show!\n");
    }
}

void add_history_left(int left){
    for (int i = 0; i<left ;i++){
        shared_memory->processes_running[i].executime_time += TSLICE;
        shared_memory->processes_running[i].pid = getpid() + shared_memory->num_procs;
        // shared_memory->processes_running[i].pid = left_pid ;
        if ( shared_memory->num_procs > 0 ) {
                strcpy(shared_memory->compelete_history[shared_memory->counter].command, shared_memory->processes_running[i].command) ;
                shared_memory->compelete_history[shared_memory->counter].pid = shared_memory->processes_running[i].pid ;
                shared_memory->compelete_history[shared_memory->counter].executime_time = shared_memory->processes_running[i].executime_time ;
                shared_memory->compelete_history[shared_memory->counter].waiting_time = shared_memory->processes_running[i].waiting_time ;

                shared_memory->counter++;
                shared_memory->num_procs--;
            } else if (shared_memory->num_procs == 0) {
                shared_memory->start_running = 0;
            }
    }
}

// scheduler functions
int run_n_processes(int n) {
    if (queueIsEmpty() == 1){
        exit(EXIT_SUCCESS);
    }
    Process* p = malloc(sizeof(Process));
    char* args[2];
    for (int i=0; i<n; i++) {
        *p = dequeueProcess();
        pid_t left_pid = 0;
        if (p->pid > 0 ){
            left_pid = p->pid;
        }
        if (p->pid < 0) {
            n = i;
            continue;
        }
       shared_memory->processes_running[i] = *p;

    }

    for (int i =0 ;i<n;i++){
        Process* p1 = malloc(sizeof(Process));
        p1 = shared_memory->processes_running+i;
    }

    for (int i=0; i<n; i++) {

        p = shared_memory->processes_running+i;
        if (strcmp(p->STATE, "NEW") == 0) {
            int child_proc = fork();
            if (child_proc < 0){
                printf("fork process failed!\n");
            }
            else if (child_proc == 0) {
                args[0] = p->command;
                args[1] = NULL;
                
                p->executime_time += TSLICE;

                clock_gettime(CLOCK_REALTIME, &p->start_time);
                execvp(args[0], args);
                   
                
                exit(EXIT_SUCCESS);
            } else {
                
                p->pid = child_proc;
                p->STATE = "RUNNING";
            }
        } else if (strcmp(p->STATE, "READY") == 0) {
            p->executime_time += TSLICE;
            p->STATE = "RUNNING";

            clock_gettime(CLOCK_REALTIME, &p->running_again_time);

            if (kill(p->pid, SIGCONT) == 0) {
                // process resumed successfully.
            } else {
                //printf("Error in resuming process with pid %d\n", p->pid);
                exit(EXIT_SUCCESS);
            }
        }
    }
    for (int i= 0;i<n ;i++){
        waitpid(shared_memory->processes_running[i].pid,NULL,WNOHANG);
        
        usleep(TSLICE*1000);
        timer_callback();
        waitpid(shared_memory->processes_running[i].pid,NULL,0);
        shared_memory->processes_running[i].STATE = "TERMINATED";
    }
    
    return -1;
}

int run_scheduled() {
    int processes_left = run_n_processes(NCPU);
    printf("This is proc_left: %d\n", processes_left);
    if (processes_left <0){
        processes_left = run_n_processes(NCPU);
    }
    else if (processes_left == 0 && queueIsEmpty()) {
        printf("ALL PROCESSES ARE DONE!!\n");
        // all processes completed
    } else {
        // run these 'left' processes 
        printf("IN THE LEFT BLOCK\n");
        Process* p = malloc(sizeof(Process));
        char* args[2];
        for (int i=0; i<processes_left; i++) {
            p = shared_memory->processes_running+i;
            if (strcmp(p->STATE, "NEW") == 0) {
                
                int child_proc = fork();
                if (child_proc == 0) {
                    args[0] = p->command;
                    args[1] = NULL;

                    clock_gettime(CLOCK_REALTIME, &p->start_time);
                    execvp(args[0], args);
                    exit(EXIT_SUCCESS);
                } else {
                    // parent process
                    p->pid = child_proc;
                    p->STATE = "RUNNING";
                }
            } else if (strcmp(p->STATE, "READY") == 0) {
                p->STATE = "RUNNING";
                if (kill(p->pid, SIGCONT) == 0) {
                    // process resumed successfully.
                } else {
                    printf("Error in resuming process with pid %d\n", p->pid);
                    exit(EXIT_FAILURE);
                }
            } else {
                // can never reach here
                perror("Some error occurred in left \n");
                exit(EXIT_FAILURE);
            }
        }
        for (int i=0; i<processes_left; i++) {
            waitpid(shared_memory->processes_running[i].pid, NULL, 0);
            shared_memory->processes_running[i].STATE = "TERMINATED";
        }
    }

    return EXIT_SUCCESS;
}

void cleanup() {
    // kill the scheduler process if the queue is empty
    kill(scheduler_pid, SIGTERM);
    cleanup_shared_memory(shared_memory);
}

// Queue functions
int enqueueProcess(Process p) {
    
    int index = p.priority - 1;
    
    if (shared_memory->process_index[index] == MAX_QUEUE_SIZE) return EXIT_FAILURE;
    shared_memory->process_table[index][shared_memory->process_index[index]++] = p;
    
    return EXIT_SUCCESS;
}

Process dequeueProcess() {
    int hPriority = -1; // highest priority
    for (int i=MAX_PRIORITY-1; i>=0; i--) {
        if (shared_memory->process_index[i] > 0) {
            hPriority = i;
            break;
        }
    }
    
    if (hPriority == -1) {
        return dummy_process;
    }
    
    Process p = shared_memory->process_table[hPriority][0];

    // Shifting the remaining to right
    for (int i = 0; i < shared_memory->process_index[hPriority] - 1; i++) {
        shared_memory->process_table[hPriority][i] = shared_memory->process_table[hPriority][i+1];
    }
    
    shared_memory->process_index[hPriority]--;
    return p;
}

int queueIsEmpty() {
    int empty=1;
    for (int i=MAX_PRIORITY-1; i>=0; i--) {
        if (shared_memory->process_index[i] > 0) {
            empty = 0;
            break;
        }   
    }

    return empty;
}

// shared memory
SharedMemory* setup_shared_memory(int NCPU) {
    int shm_fd;
    char mem_name[] = "/my_shared_memory";
    SharedMemory* shared_memory;
    

    // create a shared memory object
    shm_fd = shm_open(mem_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // set the size of shared memory
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // map the shared memory
    shared_memory = (SharedMemory* ) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    shared_memory->shm = (shm_t*) malloc(sizeof(shm_t));
    strcpy(shared_memory->shm->memory_name, mem_name);
    shared_memory->shm->fd = shm_fd;

    // initialise the process table and all
    for (int i=0; i<MAX_PRIORITY; i++) shared_memory->process_index[i] = 0;
    shared_memory->start_running = 0;
    shared_memory->command_number = 1;

    for(int i= 0 ;i<MAX_QUEUE_SIZE ; i++){
        shared_memory->compelete_history[i].number = 0 ;
        strcpy(shared_memory->compelete_history[i].command, "");
        shared_memory->compelete_history[i].pid = 0 ; 
        shared_memory->compelete_history[i].executime_time = 0;
        shared_memory->compelete_history[i].waiting_time = 0;
    }
    shared_memory->counter = 0;
    shared_memory->num_procs = 0;


    return shared_memory;
}   

void cleanup_shared_memory(SharedMemory* shared_memory) {
    if (close(shared_memory->shm->fd) == -1) {  
        perror("close");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(shared_memory->shm->memory_name) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    free(shared_memory->shm);

    if (munmap(shared_memory, sizeof(SharedMemory)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}
