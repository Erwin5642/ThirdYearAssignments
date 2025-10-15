/**
  * Exemplo de programa que demonstra pipes UNIX.
 *
 * Figures 3.25 & 3.26
 *
 * @author Silberschatz, Galvin, and Gagne
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 25
#define READ_END	0
#define WRITE_END	1

int main(void)
{
	char write_msg[BUFFER_SIZE] = "Greetings";
	char read_msg[BUFFER_SIZE];
	pid_t pid;
	int fd[2];

	/* cria o pipe */
	if (pipe(fd) == -1) {
		fprintf(stderr,"Pipe failed");
		return 1;
	}

	/* agora cria um processo filho */
	pid = fork();

	if (pid < 0) {
		fprintf(stderr, "Fork failed");
		return 1;
	}

	if (pid > 0) {  /* processo pai */
		/* fecha a extremidade não usada do pipe */
		close(fd[READ_END]);

		/* grava no pipe */
		write(fd[WRITE_END], write_msg, strlen(write_msg)+1); 

		/* fecha a extremidade de gravação do pipe */
		close(fd[WRITE_END]);
	}
	else { /* processo filho */
		/* fecha a extremidade não usada do pipe */
		close(fd[WRITE_END]);

		/* lê no pipe */
		read(fd[READ_END], read_msg, BUFFER_SIZE);
		printf("child read %s\n",read_msg);

		/* fecha a extremidade de gravação do pipe */
		close(fd[READ_END]);
	}

	return 0;
}
