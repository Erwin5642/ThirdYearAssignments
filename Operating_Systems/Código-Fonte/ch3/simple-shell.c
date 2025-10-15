/**
  * Programa de interface de shell simples.
 *
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <unistd.h>



#define MAX_LINE		80 /* 80 caracteres por linha, por comando */

int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* a linha de comando (de 80 caracteres) tem um máximo de 40 argumentos */
    int should_run = 1;
	
	int i, upper;
		
    while (should_run){   
        printf("osh>");
        fflush(stdout);
        
        /**
         * Após a leitura da entrada do usuário, as etapas são:
         * (1) criar um processo filho
         * (2) o processo filho chamará execvp()
         * (3) se o comando incluir &, o pai chamará wait()
         */
    }
    
	return 0;
}
