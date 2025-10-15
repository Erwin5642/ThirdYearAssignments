/**
  * Programa simples que demonstra os pipes anônimos do Win32.
 *
 * Figure 3.29
 *
 * @author Silberschatz, Gagne, and Galvin.
 *  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <windows.h>

#define BUFFER_SIZE 25

int main(VOID)
{
    HANDLE ReadHandle, WriteHandle; 
    CHAR buffer[BUFFER_SIZE];
    DWORD read;
    
    ReadHandle = GetStdHandle(STD_INPUT_HANDLE);
    WriteHandle= GetStdHandle(STD_OUTPUT_HANDLE);
    
    /* faz o filho ler no pipe */
    if (ReadFile(ReadHandle, buffer, BUFFER_SIZE, &read, NULL))
       printf("child: >%s<",buffer);
    else
        fprintf(stderr, "Child: Error reading from pipe\n");
    
    return 0;
}
