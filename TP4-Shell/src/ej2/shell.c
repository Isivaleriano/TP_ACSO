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

        int pipes[MAX_COMMANDS - 1][2];  

        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        if (command_count == 1) {
            wordexp_t p;
            if (wordexp(commands[0], &p, 0) == 0) {
                if (strcmp(p.we_wordv[0], "exit") == 0) {
                    wordfree(&p);
                    break;
                }
                wordfree(&p);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) { 
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }

                if (i < command_count - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                for (int j = 0; j < command_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                wordexp_t p;
                if (wordexp(commands[i], &p, 0) != 0) {
                    fprintf(stderr, "Error en wordexp\n");
                    exit(EXIT_FAILURE);
                }

                execvp(p.we_wordv[0], p.we_wordv);
                perror("execvp");
                wordfree(&p);
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }
    }
    return 0;
}
