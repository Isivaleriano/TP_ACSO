#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <wordexp.h>  

#define MAX_COMMANDS 200

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];

    while (1) {
        printf("Shell> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = '\0';

        int command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL) {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        int prev_pipe[2];

        for (int i = 0; i < command_count; i++) {
            int pipefd[2];
            if (i < command_count - 1) {
                pipe(pipefd);
            }

            pid_t pid = fork();
            if (pid == 0) {
                if (i > 0) {
                    dup2(prev_pipe[0], STDIN_FILENO);
                    close(prev_pipe[0]);
                    close(prev_pipe[1]);
                }

                if (i < command_count - 1) {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);

                }

                wordexp_t p;
                wordexp(commands[i], &p, 0);  

                execvp(p.we_wordv[0], p.we_wordv);
                perror("execvp");
                wordfree(&p);
                exit(1);
            }

            if (i > 0) {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            if (i < command_count - 1) {
                prev_pipe[0] = pipefd[0];
                prev_pipe[1] = pipefd[1];
                close(pipefd[1]);
            }
        }

        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }

    }
    return 0;
}

