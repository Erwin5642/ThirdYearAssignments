/**
  * Exemplo de região paralela do OpenMP
 * 
  * Para compilar, insira:
 *
 *	gcc -fopenmp openmp.c
 *
  * Você deve ver a mensagem "I am a parallel region" para cada
 * núcleo de processamento de seu sistema.
 *
  * Para quem estiver usando uma máquina virtual, certifique-se de configurar o número de
 * núcleos de processamento com um valor > 1 para ver a execução simultânea da região paralela.
 */

#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	/* código sequencial */

	#pragma omp parallel
	{
		printf("I am a parallel region\n");
	}

	/* código sequencial */

	return 0;
}
