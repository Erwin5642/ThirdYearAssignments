/**
 * Esse programa cria um processo separado usando as chamadas de sistema fork()/exec().
 *
 * Figure 3.09
 *
 * @author Silberschatz, Galvin, and Gagne
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
pid_t pid;

	/* cria um processo filho */
	pid = fork();

	if (pid < 0) { /* ocorreu um erro */
		fprintf(stderr, "Fork Failed\n");
		return 1;
	}
	else if (pid == 0) { /* processo filho */
		printf("I am the child %d\n",pid);
		execlp("/bin/ls","ls",NULL);
	}
	else { /* processo pai */
		/* o pai esperar� o filho terminar */
		printf("I am the parent %d\n",pid);
		wait(NULL);
		
		printf("Child Complete\n");
	}
    
    return 0;
}
