/**
  * Um programa pthread que ilustra como
  * criar um thread simples e mostra parte da API pthread
 * Esse programa implementa a função de soma onde
 * a operação de adição é executada como um thread separado.
 *
  * Para a maioria dos usuários do Unix/Linux/OS X
 * gcc thrd.c -lpthread
 *
  * Usuários do Solaris devem inserir
 * gcc thrd.c -lpthreads
 *
 * Figure 4.9
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */

#include <pthread.h>
#include <stdio.h>

int sum; /* esse dado é compartilhado pelo(s) thread(s) */

void *runner(void *param); /* o thread */

int main(int argc, char *argv[])
{
pthread_t tid; /* o identificador do thread */
pthread_attr_t attr; /* configuração de atributos do thread */
if (argc != 2) {
	fprintf(stderr,"usage: a.out <integer value>\n");
	/*exit(1);*/
	return -1;
}

if (atoi(argv[1]) < 0) {
	fprintf(stderr,"Argument %d must be non-negative\n",atoi(argv[1]));
	/*exit(1);*/
	return -1;
}

/* captura os atributos padrão */
pthread_attr_init(&attr);

/* cria o thread */
pthread_create(&tid,&attr,runner,argv[1]);

/* agora espera o thread terminar */
pthread_join(tid,NULL);

printf("sum = %d\n",sum);
}

/**
  * O thread assumirá o controle nessa função
 */
void *runner(void *param) 
{
int i, upper = atoi(param);
sum = 0;

	if (upper > 0) {
		for (i = 1; i <= upper; i++)
			sum += i;
	}

	pthread_exit(0);
}
