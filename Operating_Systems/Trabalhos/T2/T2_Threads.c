#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>

void *sumSubarray(void *param); //Função para somar os elementos de um subvetor
int *vector; //Vetor com os números do arquivo

struct ThreadArgs{
    int begin; //Inicio do subvetor
    int end; //Fim do subvetor (aberto)
    int result; //Resultado da soma do subvetor
};

int main(int argc, char **argv){
    //Testa se os argumentos forma informados de forma correta
    if(argc != 3){
        printf("Numero de argumentos incorreto. Formato correto: <arquivo> <numero de threads adicionais>\n");
        exit(EXIT_FAILURE);
    }
    //Testa se o número de threads é um número válido
    int threadsNumber = atoi(argv[2]);
    if(threadsNumber <= 0){
        printf("O numero de threads deve ser positivo e maior que zero\n");
        exit(EXIT_FAILURE);
    }
    //Abre o arquivo
    FILE *file;
    if ((file = fopen(argv[1], "rb")) == NULL) {
        perror("Nao foi possivel abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    //Obtém informações do arquivo
    struct stat fileStat;
    if (fstat(fileno(file), &fileStat) == -1) {
        perror("Erro ao obter tamanho do arquivo");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t size = fileStat.st_size; //Tamanho do arquivo
    size_t n = size / sizeof(int); //Numero de elementos do arquivo
    
    //Aloca memória para o vetor onde serao salvos os números do arquivo
    if ((vector = (int*) malloc(size)) == NULL){
        perror("Erro ao alocar memoria");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    //Salva os numeros no vetor
    if (fread(vector, sizeof(int), n, file) != n) {
        perror("Erro ao ler arquivo");
        free(vector);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);
    //Aloca espaço para threads adicionais
    pthread_t *threads;
    if((threads = malloc(threadsNumber * sizeof(pthread_t))) == NULL){
        perror("Erro ao alocar memória para as threads");
        free(vector);
        exit(EXIT_FAILURE);
    }
    //Aloca espaço para os argumentos dessas threads
    struct ThreadArgs *subArrayThreadArgs;
    if((subArrayThreadArgs = malloc(threadsNumber * sizeof(struct ThreadArgs))) == NULL){
        perror("Erro ao alocar espaço para os argumentos de cada thread");
        free(threads);
        free(vector);
        exit(EXIT_FAILURE);
    }
    //Somar os subvetores
    printf("Somando os numeros do arquivo %s com %d threads\n", argv[1], threadsNumber);
    int totalSum = 0;
    size_t subArraySize = n / threadsNumber;
    int k = 0;
    for(int i = 0; i < threadsNumber; i++){
        //Calculo do intervalo do subvevtor 
        subArrayThreadArgs[i].begin = i * subArraySize; 
        subArrayThreadArgs[i].end = (i + 1) * subArraySize;
        if (i == threadsNumber - 1) { 
            subArrayThreadArgs[i].end += n % threadsNumber; 
        }
        //Inicialização da soma resultante
        subArrayThreadArgs[i].result = 0;
        //Cria as threads
        if(pthread_create(&threads[i], NULL, sumSubarray, &subArrayThreadArgs[i]) != 0){
            i--;
            if(k == i){
                perror("Erro ao criar threads");
                exit(EXIT_FAILURE); 
            }
            // Se der erro ao criar thread, espera algumas que já foram criadas para substitui-las
            if(k < i){
                pthread_join(threads[k], NULL);
                totalSum += subArrayThreadArgs[k].result;
                k++;
            }
        }
    }
    //Soma os resultados dos subvetores 
    for(int i = k; i < threadsNumber; i++){
        pthread_join(threads[i], NULL);
        totalSum += subArrayThreadArgs[i].result;
    }    
    printf("Soma total = %d\n", totalSum);
    //Libera os recursos
    free(vector);
    free(threads);
    free(subArrayThreadArgs);
    return 0;
}

void *sumSubarray(void *params){
    struct ThreadArgs *bounds_result = params;
    for(int i = bounds_result->begin; i < bounds_result->end; i++){
        bounds_result->result += vector[i];
    }
    return NULL;
}