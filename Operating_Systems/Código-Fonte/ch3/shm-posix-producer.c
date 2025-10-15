/**
 * Programa simples que demonstra a mem�ria compartilhada em sistemas POSIX.
 *
 * Esse � o processo produtor que grava na regi�o de mem�ria compartilhada
 *
 * Figure 3.17
 *
 * @author Silberschatz, Galvin, and Gagne
  * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main()
{
	const int SIZE = 4096;
	const char *name = "OS";
	const char *message0= "Studying ";
	const char *message1= "Operating Systems ";
	const char *message2= "Is Fun!";

	int shm_fd;
	void *ptr;

	/* cria o segmento de mem�ria compartilhada */
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

	/* configura o tamanho do segmento de mem�ria compartilhada */
	ftruncate(shm_fd,SIZE);

	/* agora mapeia o segmento de mem�ria compartilhada no espa�o de endere�o do processo */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}

	/**
	 * em seguida, grava na regi�o de mem�ria compartilhada *
 	 *
	 * Observe que devemos incrementar o valor de ptr ap�s cada grava��o.
	*/
	sprintf(ptr,"%s",message0);
	ptr += strlen(message0);
	sprintf(ptr,"%s",message1);
	ptr += strlen(message1);
	sprintf(ptr,"%s",message2);
	ptr += strlen(message2);

	return 0;
}
