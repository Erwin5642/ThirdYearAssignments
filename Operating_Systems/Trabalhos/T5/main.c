// Nome: João Vitor Antunes da Silva            RGM: 48935

#include <pthread.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

static int operations; // Variável que determina quantas threads filtro adquiriram o lock do nó cabeça

typedef struct Node  // Nó da lista encadeada
{
    int data;
    pthread_mutex_t lock;

    struct Node *next;
} Node;


// Aloca um nó e inicializa variáveis
Node *nodeAllocation(int data)
{
    Node *newNode = malloc(sizeof(Node));

    if (newNode == NULL)
    {
        perror("Erro ao alocar memória\n");
        return NULL;
    }

    newNode->data = data;
    newNode->next = NULL;
    pthread_mutex_init(&newNode->lock, NULL);
    return newNode;
}

// Desaloca um nó
Node *nodeDeallocation(Node *node){
    pthread_mutex_destroy(&node->lock);
    free(node);
    return NULL;
}

// Retorna verdadeiro se um número for primo
int isPrime(int n) {
    if (n == 1) return 0; 

    int limit = (int)sqrt(n); // É necessário avaliar apenas os possíveis fatores menores que a raiz quadrado do número
    for (int i = 3; i <= limit; i += 2) { 
        if (n % i == 0) return 0;
    }
    return 1;
}

// Desaloca todos os nós de uma lista encadeada (incluindo nó cabeça)
Node *deallocateList(Node *head){
    Node *iterator = head;
    while(iterator){
        Node *aux = iterator->next;
        nodeDeallocation(iterator);
        iterator = aux;
    }
    return NULL;
}

// Thread que remove números não impares maiores que 2 de uma lista encadeada
void *filterOdd(void *param){
    Node *previous = (Node *)param;
    pthread_mutex_lock(&previous->lock);
    operations += 1; // Sinaliza que a thread atual conseguiu dar lock no nó cabeça

    Node *iterator = previous->next;

    // Mantém sempre o nó anterior travado, desta forma nenhuma thread consegue ultrapassá-la
    while(iterator != NULL){
        pthread_mutex_lock(&iterator->lock);

        // Realiza a remoção caso não passe no filtro
        if(iterator->data > 2 && iterator->data % 2 == 0){
            previous->next = iterator->next;
            nodeDeallocation(iterator);
            iterator = previous->next;
        }
        else{
            pthread_mutex_unlock(&previous->lock);
            previous = iterator;
            iterator = iterator->next;

            // Avança o nó atual e anterior, desta forma o antigo atual se torna o anterior (ou seja, nó anterior permanece travado)
        }
    }
    // Ao fim, pela invriante definida, o nó anterior está travado e precisamos liberá-lo
    pthread_mutex_unlock(&previous->lock);

    return NULL;
}

// Thread que remove números não primos de uma lista encadeada
void *filterPrime(void *param){
    Node *previous = (Node *)param;
    pthread_mutex_lock(&previous->lock);
    operations += 1; // Sinaliza que a thread atual conseguiu dar lock no nó cabeça

    Node *iterator = previous->next;

    // Mantém sempre o nó anterior travado, desta forma nenhuma thread consegue ultrapassá-la
    while(iterator != NULL){
        pthread_mutex_lock(&iterator->lock); 

        if(isPrime(iterator->data) == 0){
            previous->next = iterator->next;
            nodeDeallocation(iterator);
            iterator = previous->next;
        }
        else{
            pthread_mutex_unlock(&previous->lock);
            previous = iterator;
            iterator = iterator->next;
            // Avança o nó atual e anterior, desta forma o antigo atual se torna o anterior (ou seja, nó anterior permanece travado)
        }
    }
    // Ao fim, pela invriante definida, o nó anterior está travado e precisamos liberá-lo
    pthread_mutex_unlock(&previous->lock);

    return NULL;
}

//Thread que exibe os elementos de uma lista encadeada
void *printResult(void *param){
    Node *previous = (Node *)param;

    while(operations < 2); // Espera as duas threads de filtro adquirirem e liberarem o mutex do nó cabeça

    pthread_mutex_lock(&previous->lock); // Adquire acesso a cebeça assim que as outras threads terminarem
    Node *iterator = previous->next; // Pula o nó cabeça
    pthread_mutex_unlock(&previous->lock); // Libera o nó cabeça pois não haverá modificações nele

    while(iterator != NULL){
        pthread_mutex_lock(&iterator->lock); // Adquire acesso ao nó 
        printf("%d ", iterator->data); // Exibe os dados
        pthread_mutex_unlock(&iterator->lock); // Libera a trava do nó
        iterator = iterator->next; // Passa para o próximo
    }    

    return NULL;
}

//Rotina principal
int main()
{    
    FILE *file;
    if(!(file = fopen("in.txt", "r"))){
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    } //Erro caso não seja possível abrir arquivo

    //Criação da lista
    Node *head = nodeAllocation(-1);    
    Node *previous = head;
    pthread_mutex_lock(&previous->lock); // A thread principal (produtora) tem prioridade sobre todas as outras, por isso trava o mutex do 
    // nó cabeça prematuramente

    Node *iterator;

    //Criação das threads
    pthread_t filterOddThread, filterPrimeThread, printThread;

    pthread_create(&filterOddThread, NULL, filterOdd, (void *)head);
    pthread_create(&filterPrimeThread, NULL, filterPrime, (void *)head);
    pthread_create(&printThread, NULL, printResult, (void *)head);

    //Tarefa da thread principal, ler os numeros no arquivo
    int data;
    while(fscanf(file, "%d", &data) != EOF){
        previous->next = nodeAllocation(data);

        iterator = previous->next;
        pthread_mutex_lock(&iterator->lock);

        pthread_mutex_unlock(&previous->lock);
        previous = iterator;
    }
    pthread_mutex_unlock(&previous->lock);

    fclose(file);

    //Espera as threads secundarias
    pthread_join(filterOddThread, NULL);
    pthread_join(filterPrimeThread, NULL);
    pthread_join(printThread, NULL);

    //Libera os recursos das listas
    deallocateList(head);

    return 0;
}