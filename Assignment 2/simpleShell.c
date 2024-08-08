#include "simpleShell.h"

int command_number = 1;
int background_process_number = 0;

char PROMPT[1024] = "\033[1;33mshell_sh> \033[0m";
char HISTORY_FILE[1024];

void update_prompt(char* pwd) {
    snprintf(PROMPT, sizeof(PROMPT), "\033[1;33mshell_sh:\033[34m%s\033[0m> ", pwd);
}

int main() {
    char* input[MAX_SIZE];
    signal(SIGINT, handler);
    shell_loop(*input);
    return EXIT_SUCCESS;
}

void handler(int signum) {
    printf("\nCtrl + C was pressed! Exiting...\n");
    printf("Showing history...\n\n");
    sleep(1);

    char history[2000];
    if (get_history(history) == HISTORY_ERROR_CODE) {
        perror("Some error occurred!\n");
    } else {
        printf("%s", history);
    }

    clear_history();
    exit(signum);
}

void shell_loop() {
    int status = 1;

    char *command;
    char **args;
    char pwd[1024];

    pwd_command(pwd);
    strcat(pwd, "/history.txt");
    strcpy(HISTORY_FILE, pwd);
    
    do{
        pwd_command(pwd);
        update_prompt(pwd);

        printf("%s", PROMPT);
        fflush(stdout);
        
        command = read_line();
        args = splitting_function(command);

        int background = 0, bg_ended = 0, child_pid;

        int arg_count = 0;
        while (args[arg_count] !=NULL){
            arg_count ++;
        }

        if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
            background = 1;
            background_process_number++;
            args[arg_count - 1] = NULL;
        }


        char *commands[10][MAX_ARGC+1] ;
        int i=0 ,j =0;
        for (int h =0 ;args[h] !=NULL ;h++){
            if (strcmp(args[h],"|") != 0){
                commands[i][j++] = args[h];
            }
            else{
                commands[i][j] = NULL;
                i++;
                j=0;
            }

        }
        commands[i][j] = NULL;

        int prev_pipe, pfds[2];
        prev_pipe = STDIN_FILENO;

        int child_status = fork() , command_status;
        
        if (child_status < 0) {
            printf("the fork command failed !");
        }
        else if (child_status == 0){
            signal(SIGINT, SIG_DFL);
            
            if (background){
                int dev_null = open("/dev/null",O_RDWR);
                dup2(dev_null,STDIN_FILENO);
                dup2(dev_null,STDIN_FILENO);
                close(dev_null);
            }
            for (int l = 0; l < i; l++) {
                pipe(pfds);

                if (fork() == 0) {
                    
                    if (prev_pipe != STDIN_FILENO) {
                        dup2(prev_pipe, STDIN_FILENO);
                        close(prev_pipe);
                    }

                    dup2(pfds[1], STDOUT_FILENO);
                    close(pfds[1]);

                    if (strcmp(commands[l][0], "history") == 0) {
                        char * arguments[] = {"cat", HISTORY_FILE, NULL};
                        if (execvp("cat", arguments) == -1) {
                            perror("execvp failed");
                            exit(1);
                        }
                    } else {
                        if (execvp(commands[l][0], commands[l]) == -1) {
                            perror("execvp failed");
                            exit(1);
                        }
                    }
                }
                else{
                    wait(NULL);
                }

                close(prev_pipe);
                close(pfds[1]);

                prev_pipe = pfds[0];
            }

            if (prev_pipe != STDIN_FILENO) {
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }

            if (strcmp(commands[i][0], "cd") == 0) {
                if (cd_command(commands[i]) != 1) {
                    perror("Cannot run cd command with given arguements\n");
                } else {
                    pwd_command(pwd);
                    update_prompt(pwd);
                }
            } else if (strcmp(commands[i][0], "history") == 0) {
                char history[2000];
                if (get_history(history) == HISTORY_ERROR_CODE) {
                    perror("Some error occurred!\n");
                } else {
                    printf("%s", history);
                }
            } else if (strcmp(commands[i][0], "run") == 0) {
                int a = run_script(commands[i][1]);
                if (a == EXIT_FAILURE) {
                    perror("Some error occured!\n");
                }
            } else {
                if (execvp(commands[i][0], commands[i]) == -1) {
                    perror("execvp failed");
                    exit(1);
                }
            }
        }
        else{
            if (!background){
                waitpid(child_status, NULL , 0);
            } else {
                printf("[%d]\t\t%d\n", background_process_number, child_status);
            }

            while ((child_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
                printf("[%d]\t\t%d completed\n", background_process_number--, child_pid);
            }

            free(args);
        }
    }
    while(status);
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
    write_history(command);
    return command;
}

char **splitting_function(char *command) {
    int buffsize = SH_TOKEN_BUFFSIZE;
    int i= 0;
    char **tokens = malloc(buffsize*sizeof(char *));

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
            tokens =realloc(tokens, buffsize *sizeof(char *));
            
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

int write_history(char *str) {
    FILE *file_ptr = fopen(HISTORY_FILE, "a");

    if (file_ptr == NULL) {
        return EXIT_FAILURE;
    }

    int white_spaces = 0 ;  
    for (int i = 0 ;i< strlen(str) ;i++){
        if (str[i] == ' '){
            white_spaces ++;
        }
    }

    if (white_spaces == (strlen(str) - 1)){
        return 0;
    }
    if (str != NULL && str[0] != '\0' ) {
        time_t current_time;

        struct timeval time1, time2;
        gettimeofday(&time1, NULL);

        time(&current_time);
        struct tm *time_details;

        time_details = localtime(&current_time);

        int child_pid;
        int status1;

        if ((child_pid = fork()) < 0) {
            perror("fork command failed in the write_history !");
            exit(EXIT_FAILURE);
        } else if ( child_pid == 0) {
            char *args1[] = {str, NULL};
            if (execvp(args1[0], args1) == -1) {
                //perror("execvp failed");
                exit(1);
            }
        } else {
            waitpid(child_pid, &status1,0);
            gettimeofday(&time2, NULL);
            time_t ending_time;
            time(&ending_time);

            // converting the time in milliseconds
            double total_duration = (double)(time2.tv_usec - time1.tv_usec)/(double)1000 + (time2.tv_sec - time1.tv_sec)*1000;  

            fprintf(file_ptr, "%d  process_pid:[%d] time of execution: %02d:%02d:%02d  total_time_of_duration(in ms): %04.06f %10s", command_number++, child_pid,
                time_details->tm_hour, time_details->tm_min, time_details->tm_sec, total_duration, str);
        }
    
        
    }

    fclose(file_ptr);
    return EXIT_SUCCESS;
}

int get_history(char history[HISTORY_SIZE]) {
    FILE * file_ptr = fopen(HISTORY_FILE, "r");
    char ch;
    int i=0;

    if (file_ptr == NULL) {
        return HISTORY_ERROR_CODE;
    }

    while (ch != EOF && i < HISTORY_SIZE) {
        ch = fgetc(file_ptr);
        history[i++] = ch;
    };
    history[--i] = '\0';

    fclose(file_ptr);
    return i;
}

void clear_history() {
    remove(HISTORY_FILE);
}

int run_script(char* filename) {
    FILE* file_ptr = fopen(filename, "r");
    char line[100], **args;
    char pwd[1024];

    if (file_ptr == NULL) {
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), file_ptr) != NULL) {
        
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        args = splitting_function(line);
        char *commands[10][MAX_ARGC+1];
        int i = 0, j = 0;

        for (int h = 0; args[h] != NULL; h++) {
            if (strcmp(args[h], "|") != 0) {
                commands[i][j++] = args[h];
            } else {
                commands[i][j] = NULL;
                i++;
                j = 0;
            }
        }
        commands[i][j] = NULL;

        int prev_pipe, pfds[2];
        prev_pipe = STDIN_FILENO;

        int child_status = fork(), command_status;

        if (child_status < 0) {
            printf("the fork line failed !");
        } else if (child_status == 0) {
            signal(SIGINT, SIG_DFL);
            for (int l = 0; l < i; l++) {
                pipe(pfds);

                if (fork() == 0) {

                    if (prev_pipe != STDIN_FILENO) {
                        dup2(prev_pipe, STDIN_FILENO);
                        close(prev_pipe);
                    }

                    dup2(pfds[1], STDOUT_FILENO);
                    close(pfds[1]);

                    if (strcmp(commands[l][0], "history") == 0) {
                        char * arguments[] = {"cat", HISTORY_FILE, NULL};
                        if (execvp("cat", arguments) == -1) {
                            perror("execvp failed");
                            exit(1);
                        }
                    } else {
                        if (execvp(commands[l][0], commands[l]) == -1) {
                            perror("execvp failed");
                            exit(1);
                        }
                    }
                } else {
                    wait(NULL);
                }

                close(prev_pipe);
                close(pfds[1]);

                prev_pipe = pfds[0];
            }

            // Get stdin from the last pipe
            if (prev_pipe != STDIN_FILENO) {
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }

            if (strcmp(commands[i][0], "cd") == 0) {
                if (cd_command(commands[i]) != 1) {
                    perror("Cannot run cd command with given arguements\n");
                } else {
                    pwd_command(pwd);
                    update_prompt(pwd);
                }
            } else if (strcmp(commands[i][0], "history") == 0) {
                char history[2000];
                if (get_history(history) == HISTORY_ERROR_CODE) {
                    perror("Some error occurred!\n");
                } else {
                    printf("%s", history);
                }
            } else if (strcmp(commands[i][0], "run") == 0) {
                int a = run_script(commands[i][1]);
                if (a == EXIT_FAILURE) {
                    perror("Some error occured!\n");
                }
            } else {
                if (execvp(commands[i][0], commands[i]) == -1) {
                    perror("execvp failed");
                    exit(1);
                }
            }
        } else {

            //printf("I am the parent shell\n");
            waitpid(child_status, NULL, 0);
            free(args);
        }
    }

    fclose(file_ptr);
    return EXIT_SUCCESS;
}