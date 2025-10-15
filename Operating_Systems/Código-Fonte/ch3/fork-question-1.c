/*
 * Resposta da pergunta 3.1
 * 
 * A resposta � 5 j� que tanto o processo pai quanto o processo filho t�m sua pr�pria c�pia de value.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int value = 5;

int main()
{
	pid_t pid;

	pid = fork();
	
	if (pid == 0) { /* child process */
		value += 15;
		return 0;
	}
	else if (pid > 0) { /* parent process */
		if(wait(NULL) == -1){
			fprintf(stderr, "Error while waiting child\n");
		}
		printf ("PARENT: value = %d\n",value); /* LINE A */
		return 0;
	}
}
