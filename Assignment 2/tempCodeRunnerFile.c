while ((child_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
                printf("[%d]\t\t%d completed\n", background_process_number--, child_pid);
            }