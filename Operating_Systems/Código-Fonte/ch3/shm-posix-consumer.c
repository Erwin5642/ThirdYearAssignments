/**
 * Programa simples que demonstra a mem�ria compartilhada em sistemas POSIX.
 *
 * Esse � o processo consumidor
 *
 * Figure 3.18
 *
 * @author Gagne, Galvin, Silberschatz
 * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main()
{
	const char *name = "OS";
	const int SIZE = 4096;

	int shm_fd;
	void *ptr;
	int i;

	/* abre o segmento de mem�ria compartilhada */
	shm_fd = shm_open(name, O_RDONLY, 0666);
		if (shm_fd == -1) {
		printf("shared memory failed\n");
		exit(-1);
	}

	/* agora mapeia o segmento de mem�ria compartilhada no espa�o de endere�o do processo */
	ptr = mmap(0,SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		exit(-1);
	}

	/* em seguida, l� na regi�o de mem�ria compartilhada */
	printf("%s",ptr);

	/* remove o segmento de mem�ria compartilhada */
	if (shm_unlink(name) == -1) {
		printf("Error removing %s\n",name);
		exit(-1);
	}

	return 0;
}
