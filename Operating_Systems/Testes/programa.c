#define MAX_PROCESS 28123

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>    // O_RDWR, O_CREAT
#include <sys/stat.h> // S_IRUSR, S_IWUSR
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> // Para wait()

void Leitura_Bin(char *filename, int *size, int **array)
{
    struct stat fileStat;
    FILE *fp = fopen(filename, "rb");

    if (fp)
    {

        int file_descript = fileno(fp);
        fstat(file_descript, &fileStat);

        *array = (int *)malloc(fileStat.st_size);
        *size = fileStat.st_size / sizeof(int);
        if (!*array)
        {
            printf("Erro de alocacao de memoria");
            free(*array);
            fclose(fp);
            return;
        }
        if(fread(*array, sizeof(int), *size, fp) != *size){
            printf("Erro de leitura para o vetor \n");
            return;
        }
        
        fclose(fp);
        return;
    }
    printf("Erro de Leitura: [%s]", filename);
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Erro de argumentos : make run [nome_arq] [nomumero_processos]\n");
        return -1;
    }

    if (atoi(argv[2]) <= 0)
    {
        printf("Numero de processos [%d] invalidos", atoi(argv[2]));
        return -1;
    }
    int tam = 0, quant_process = atoi(argv[2]), *vet;
    if (quant_process > MAX_PROCESS)
    {
        printf("Quantidade maxima de processos[%d] execedida, mudando [%d] para [%d]", MAX_PROCESS, quant_process, MAX_PROCESS);
        quant_process = MAX_PROCESS-1;
    }

    Leitura_Bin(argv[1], &tam, &vet);
    //for (int i = 0; i < tam; i++)
      //  printf("%d ", vet[i]);

    pid_t pid_id,
        div = tam / quant_process;
    if (div <= 0)
    {
        printf("Valor de processos [%d], e superior ao numero de elementos[%d], mudando numero de processos para [%d]"
            , quant_process, tam, tam);
        quant_process = tam;
        div = 1;
    }

    for (int i = 0; i < quant_process; i++)
    {
        pid_id = fork(); 
        if(pid_id <0){
            perror("Erro no fork");
            break;
        }else if (pid_id == 0){
            printf("pai criado\n");
            exit(0);
        }
    }

    for (int i = 0; i < quant_process; i++)
    {
        wait(NULL);
    }

    return 0;
}