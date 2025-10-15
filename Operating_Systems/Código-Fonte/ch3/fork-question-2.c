/**
 * Resposta do exerc�cio 3.2
 *
 * A resposta � facilmente determinada pela contagem 
  * do n�mero de processos exclusivos que s�o exibidos
 * pela chamada a getpid() � que s�o 8 processos exclusivos.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
	printf("%d\n",getpid());
	fork();
	printf("%d\n",getpid());
	

	fork();
	printf("%d\n",getpid());

	fork();
	printf("%d\n",getpid());
	int value;
	if(value = wait(NULL) == -1){
		fprintf(stderr, "Error while parent %d waiting child\n", getpid());
	}
	printf ("Parent %d waited for %d\n", getpid(), value);

	return 0;
}
