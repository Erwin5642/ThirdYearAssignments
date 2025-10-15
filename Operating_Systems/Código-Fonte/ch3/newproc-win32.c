/**
 * Esse programa cria um processo separado usando a chamada de sistema CreateProcess().
 *
 * Figure 3.10
 *
 * @author Gagne, Galvin, Silberschatz, Galvin, and Gagne
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */
 
#include <windows.h>
#include <stdio.h>

int main( VOID )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Inicia o processo filho.
    if( !CreateProcess( NULL,   // Sem nome do módulo (usa a linha de comando).
        "C:\\WINDOWS\\system32\\mspaint.exe", // Linha de comando.
        NULL,             // Handle de processo não herdável.
        NULL,             // Handle de thread não herdável.
        FALSE,            // Configura a herança do handle com FALSE.
        0,                // Sem flags de criação.
        NULL,             // Usa o bloco de ambiente do pai.
        NULL,             // Usa o diretório inicial do pai.
        &si,              // Ponteiro para a estrutura STARTUPINFO.
        &pi )             // Ponteiro para a estrutura PROCESS_INFORMATION.
    )
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return -1;
    }

    // Espera o processo filho terminar.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Fecha os handles do processo e do thread.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

