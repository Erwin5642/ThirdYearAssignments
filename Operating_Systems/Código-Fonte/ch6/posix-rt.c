/**
  * Um programa pthread simples que ilustra o scheduling RT do Pthreads.
 *
 * Figure 6.20
 *
 * Para compilar:
 *
 *	gcc posix-rt.c -o posix-rt -lpthread
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013.
 */

#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS 5

/* o thread é executado nessa função */
void *runner(void *param); 

main(int argc, char *argv[])
{
	int i, policy;
	pthread_t tid[NUM_THREADS]; 	/* identificador do thread */
	pthread_attr_t attr; 		/* configuração de atributos do thread */

	/* captura os atributos padrão */
	pthread_attr_init(&attr);

	/* primeiro pesquisa sobre o escopo atual */
	if (pthread_attr_getschedpolicy(&attr,&policy) != 0)
		fprintf(stderr, "Unable to get policy.\n");
	else {
		if (policy == SCHED_OTHER)
			printf("SCHED_OTHER\n");
		else if (policy == SCHED_RR)
			printf("SCHED_OTHER\n");
		else if (policy == SCHED_FIFO)
			printf("SCHED_FIFO\n");
	}
	
	/* configura o algoritmo de scheduling com PCS ou SCS */
	if (pthread_attr_setschedpolicy(&attr, SCHED_OTHER) != 0)
		printf("unable to set scheduling policy to SCHED_OTHER \n");

	/* cria os threads */
	for (i = 0; i < NUM_THREADS; i++) 
		pthread_create(&tid[i],&attr,runner,NULL); 

	/**
	  * Agora vincula todos os threads */
	 */
	for (i = 0; i < NUM_THREADS; i++) 
		pthread_join(tid[i], NULL);
}

/**
  * O thread assumirá o controle nessa função.
 */
void *runner(void *param) 
{
	/* faz alguma coisa ... */

	pthread_exit(0);
}

