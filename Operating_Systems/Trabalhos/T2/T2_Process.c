#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    //Testa se os argumentos foram informados corretamente
    if(argc != 3){
        printf("Numero de argumentos incorreto. Formato correto: <arquivo> <numero de processos adicionais>\n");
        exit(EXIT_FAILURE);
    }
    //Testa se o número de processos é válido
    int processNumber = atoi(argv[2]);
    if(processNumber <= 0){
        printf("O numero de processos deve ser positivo e maior que zero\n");
        exit(EXIT_FAILURE);
    } 
    //Abre o arquivo
    FILE *file;
    if ((file = fopen(argv[1], "rb")) == NULL) {
        perror("Nao foi possivel abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    //Obtem informações sobre o arquivo
    struct stat fileStat;
    if (fstat(fileno(file), &fileStat) == -1) {
        perror("Erro ao obter tamanho do arquivo");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    size_t size = fileStat.st_size; //Tamanho do arquivo
    size_t n = size / sizeof(int); //Número de elementos no arquivo
    size_t resultsArraySize = processNumber * sizeof(int); //Tamanho do vetor de resultados

    // Criando região e memória compartilhada
    int fd_shm = shm_open("vetor_compatilhado", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_shm == -1) {
        perror("Erro ao criar memoria compartilhada");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    ftruncate(fd_shm, resultsArraySize);
    void *sharedRegion = mmap(NULL, resultsArraySize, PROT_WRITE | PROT_READ, MAP_SHARED, fd_shm, 0);
    if(sharedRegion == MAP_FAILED){
        perror("Mapeamento de regiao compartilhada falhou");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    //Cria um vetor para salvar os números do arquivo
    int *vector;
    if ((vector = (int*) malloc(size)) == NULL) {
        perror("Erro ao alocar memoria");
        munmap(sharedRegion, resultsArraySize);
        close(fd_shm);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    //Salva os números do arquivo no vetor
    if (fread(vector, sizeof(int), n, file) != n) {
        perror("Erro ao ler arquivo");
        free(vector);
        munmap(sharedRegion, resultsArraySize);
        close(fd_shm);
        fclose(file);
        exit(EXIT_FAILURE);
    }   
    fclose(file);
    //Vetor utilizado para fazer operações com inteiros na região compartilhada
    int *resultsArray = (int *)sharedRegion;
    //Tamanho de cada subvetor para cada filho (exceto o último)
    size_t subArraySize = n / processNumber;
    printf("Somando os numeros do arquivo %s com %d processos\n", argv[1], processNumber);
    int k = 0;
    for(int i = 0; i < processNumber; i++){
        pid_t pid = fork();
        if(pid == 0){
            //Seleciona uma area para o subvetor
            int begin = i * subArraySize;
            int end = (i + 1) * subArraySize - 1;
            int result = 0;
            //Se for o último concatena a área com o restante
            if (i == processNumber - 1) { 
                end += n % processNumber; 
            }
            //Calcua a soma dos elementos do subvetor
            for(int j = begin; j <= end; j++){
                result += vector[j];
            }
            //Salva o resultado na memória compartilhada e encerra o filho
            resultsArray[i] = result;
            free(vector);
            exit(EXIT_SUCCESS);
        }
        else if(pid < 0){
            // Se após esperar todos os processos, ocorrer em seguida outro erro encerra a execução
            i--;
            if(k == i){
                perror("Erro no fork");
                free(vector);
                munmap(sharedRegion, resultsArraySize);
                close(fd_shm);
                shm_unlink("vetor_compatilhado");
                exit(EXIT_FAILURE); 
            }
            // Se der erro no fork, espera filhos já criados para criar novos no lugar
            while(k < i){
                wait(NULL);
                k++;
            }
        }
    }

    // Espera até que todos os filhos que estão em execução terminem
    for(int i = k; i < processNumber; i++){
        wait(NULL);    
    }
    // Calcula resultado dos subvetores 
    int totalSum = 0;
    for(int i = 0; i < processNumber; i++){
        totalSum += resultsArray[i];
    }
    // Exibe resultado
    printf("Soma total = %d\n", totalSum);
    // Libera os recursos
    free(vector);
    munmap(sharedRegion, resultsArraySize);
    close(fd_shm);
    shm_unlink("vetor_compatilhado");

    return 0;
}