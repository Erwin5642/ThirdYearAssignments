/**
  * Esse programa cria um thread separado usando a chamada de sistema CreateThread().
 *
 * Figure 4.11
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <windows.h>


DWORD Sum; /* o dado é compartilhaod pelo(s) thread(s) */

/* o thread é executado nessa função separada */
DWORD WINAPI Summation(PVOID Param)
{
	DWORD Upper = *(DWORD *)Param;

	for (DWORD i = 0; i <= Upper; i++)
		Sum += i;



	return 0;
}


int main(int argc, char *argv[])
{
	DWORD ThreadId;
	HANDLE ThreadHandle;
	int Param;

	// faz alguma verificação de erros básica
	if (argc != 2) {
		fprintf(stderr,"An integer parameter is required\n");
		return -1;
	}

	Param = atoi(argv[1]);

	if (Param < 0) {
		fprintf(stderr, "an integer >= 0 is required \n");
		return -1;
	}

	// cria o thread
	ThreadHandle = CreateThread(NULL, 0, Summation, &Param, 0, &ThreadId);

	if (ThreadHandle != NULL) {
		WaitForSingleObject(ThreadHandle, INFINITE);
		CloseHandle(ThreadHandle);
		printf("sum = %d\n",Sum);
	}
}
