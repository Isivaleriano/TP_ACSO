#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf("Uso: anillo <n> <c> <s>\n");
		return 1;
	}

	int n = atoi(argv[1]);
	int value = atoi(argv[2]);
	int start = atoi(argv[3]);

	if (start < 0 || start >= n)
		return 1;

	printf("Se crearán %d procesos, se enviará el valor %d desde proceso %d\n", n, value, start);

	int pipe_padre[2];
	pipe(pipe_padre);

	int **pipes = malloc(n * sizeof(int*));
	for (int i = 0; i < n; i++) {
		pipes[i] = malloc(2 * sizeof(int));
		pipe(pipes[i]);
	}

	for (int i = 0; i < n; i++) {
		if (fork() == 0) {
			for (int j = 0; j < n; j++) {
				if (j != i) close(pipes[j][0]); 
				if (j != (i + 1) % n) close(pipes[j][1]); 
			}
			close(pipe_padre[0]);

			int num;
			read(pipes[i][0], &num, sizeof(int));
			num++;

			if ((i + 1) % n == start)
				write(pipe_padre[1], &num, sizeof(int));
			else
				write(pipes[(i + 1) % n][1], &num, sizeof(int));

			close(pipes[i][0]);
			close(pipes[(i + 1) % n][1]);
			close(pipe_padre[1]);

			for (int k = 0; k < n; k++) {
				free(pipes[k]);
			}
			free(pipes);

			exit(0);
		}
	}

	for (int i = 0; i < n; i++) {
		close(pipes[i][0]);
		if (i != start)
			close(pipes[i][1]);
	}
	close(pipe_padre[1]); 

	write(pipes[start][1], &value, sizeof(int));
	close(pipes[start][1]);

	int final_result;
	read(pipe_padre[0], &final_result, sizeof(int));
	close(pipe_padre[0]);

	for (int i = 0; i < n; i++)
		wait(NULL);

	printf("Resultado final: %d\n", final_result);

	for (int i = 0; i < n; i++)
		free(pipes[i]);
	free(pipes);

	return 0;
}
