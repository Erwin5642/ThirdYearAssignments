/**
  * Programa simples que demonstra os pipes anônimos do Win32.
 *
 * Figures 3.27 & 3.28
 *
 * @author Silberschatz, Gagne and Galvin.
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BUFFER_SIZE 25

int main(VOID)
{
 HANDLE ReadHandle, WriteHandle;
 STARTUPINFO si;
 PROCESS_INFORMATION pi;
 char message[BUFFER_SIZE] = "Greetings";
 DWORD written;
 
 /* configura os atributos de segurança de modo que os handles dos pipes sejam herdados. */
 SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL,TRUE};

 /* aloca memória */
 ZeroMemory(&pi, sizeof(pi));
 
 /* cria o pipe */
 if ( !CreatePipe(&ReadHandle, &WriteHandle, &sa, 0)) {
    fprintf(stderr,"Create Pipe Failed\n");
    return 1;
 }
  
 /* estabelece a estrutura START_INFO do processo filho */
 GetStartupInfo(&si);
 si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
 si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
 
 /* redireciona a entrada padrão para a extremidade de leitura do pipe */
 si.hStdInput = ReadHandle;
 si.dwFlags = STARTF_USESTDHANDLES;
 
 /* não queremos que o filho herde a extremidade de gravação do pipe */
 SetHandleInformation( WriteHandle, HANDLE_FLAG_INHERIT, 0); 
 
 /* cria o processo filho */
 if (!CreateProcess(NULL, 
                     ".\\child.exe",
                     NULL,
                     NULL,
                     TRUE, /* herda handles */
                     0,
                     NULL,
                     NULL,
                     &si,
                     &pi))
  {
     fprintf(stderr, "Process Creation Failed\n");
     return -1;
  }
               
 /* fecha a extremidade não usada do pipe */
 CloseHandle(ReadHandle);
       
 /* agora o pai quer gravar no pipe */
 if (!WriteFile (WriteHandle, message, BUFFER_SIZE, &written, NULL))
    fprintf(stderr, "Error writing to pipe\n");
    
 /* fecha a extremidade de gravação do pipe */
 CloseHandle(WriteHandle);

 /* espera o filho terminar */
 WaitForSingleObject(pi.hProcess, INFINITE);  

 /* fecha todos os handles */
 CloseHandle(pi.hProcess);
 CloseHandle(pi.hThread);                 
}
