// Nome: João Vitor Antunes da Silva            RGM: 48935

#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define POISON_PILL -1 // Define o fim das operações em uma lista

sem_t *oddsSemaphore; // Semaforo para filtragem dos números impares 
sem_t *primeSemaphore; // Semaforo para fitragem dos numeros primos
sem_t *printSemaphore; // Semaforo para imprimir resultados

typedef struct Node
{
    int data;
    struct Node *next;
} Node; // Nó de uma lista encadeada

typedef struct
{
    Node *head;
    Node *tail;
} List; // Lista encadeada com ponteiro primeira e ultima posiçao, inserção O(1)

typedef struct {
    List *l1;
    List *l2;
}ListPair; // Par de listas

// Aloca dados em um nó
Node *safeAllocation(int data)
{
    Node *newNode = malloc(sizeof(Node));

    if (newNode == NULL)
    {
        perror("Erro ao alocar memória\n");
        return NULL;
    }

    newNode->data = data;
    newNode->next = NULL;
    return newNode;
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

// Inicia uma lista vazia
void initList(List *list)
{
    list->head = list->tail = NULL;
}

// Insere em uma lista encadeada
void insertList(List *list, int data)
{
    Node *newNode = safeAllocation(data);
    
    //Se a lista for vazia, aponta cabeça e cauda para o elemento alocado
    if (list->head == NULL)
    {
        list->tail = list->head = newNode;
        return;
    }
    // Se não, insere no fim da lista
    list->tail->next = newNode;
    list->tail = newNode;
}

// Desaloca todos os nós de uma lista encadeada
void destroyList(List *list)
{
    Node *iterator = list->head;
    while(iterator != NULL){
        Node *aux = iterator->next;
        free(iterator); //Desaloca cada nó
        iterator = aux;
    }
    list->head = list->tail = NULL; //Reseta os ponteiros da lista
}

//Thread que cria uma lista encadeada nova apenas com os números ímpares de uma lista original
void *filterOdd(void *param){
    ListPair pair = *(ListPair *)param;
    
    List *consume = pair.l1;
    List *produce = pair.l2;

    sem_wait(oddsSemaphore); // Espera pela criação de um elemento
    Node *iterator = consume->head; // Iterador da lista

    while(1){
        int data = iterator->data;

        if(data == POISON_PILL || data == 2 || data % 2 != 0){
            insertList(produce, data); //Insere apenas se for impar ou igual a 2 ou é a flag de encerramento
            sem_post(primeSemaphore);
            if(data == POISON_PILL){ //Se o elemento for a flag de encerramento, encerra execução
                pthread_exit(0);
            }
        }
        sem_wait(oddsSemaphore); //Espera pela criação de mais elementos
        iterator = iterator->next;
    }
}

//Thread que cria uma lista encadeada nova apenas com os números primos de uma lista original
void *filterPrime(void *param){
    ListPair pair = *(ListPair *)param;
    
    List *consume = pair.l1;
    List *produce = pair.l2;
    //Espera pela criação de pelo menos um elemento
    sem_wait(primeSemaphore);
    Node *iterator = consume->head; //iterador da lista

    while(1){
        int data = iterator->data;

        if(data == POISON_PILL || isPrime(data)){
            insertList(produce, data); //Insere apenas se for primo ou é a flag de encerramento
            sem_post(printSemaphore);
            if(data == POISON_PILL){ //Se o elemento for a flag de encerramento, encerra execução
                pthread_exit(0);
            }
        }
        sem_wait(primeSemaphore); //Espera pela produção de mais elementos
        iterator = iterator->next;
    }
}

//Thread que exibe os elementos de uma lista encadeada
void *printResult(void *param){
    List *consume = (List *) param;
    //Espera pela produção de um elemento
    sem_wait(printSemaphore);
    Node *iterator = consume->head; //Iterador da lista

    while(1){
        int data = iterator->data;
        //Se o elemento for a flag de encerramento, encerra execução
        if(data == POISON_PILL){
            pthread_exit(0);
        }
        printf("%d ", data);
        //Espera pela produção de mais elementos
        sem_wait(printSemaphore);
        iterator = iterator->next;
    }
}

//Rotina principal
int main()
{    
    //Unlink em caso dos semaforos existirem
    sem_unlink("/T4_odds_sem");
    sem_unlink("/T4_primes_sem");
    sem_unlink("/T4_print_sem");
    
    FILE *file;
    if(!(file = fopen("in.txt", "r"))){
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    }
    
    //Criação dos semaforos
    oddsSemaphore = sem_open("/T4_odds_sem", O_CREAT, 0644, 0);
    primeSemaphore = sem_open("/T4_primes_sem", O_CREAT, 0644, 0);
    printSemaphore = sem_open("/T4_print_sem", O_CREAT, 0644, 0);

    if (oddsSemaphore == SEM_FAILED || primeSemaphore == SEM_FAILED || printSemaphore == SEM_FAILED) {
        perror("Erro ao abrir semaforo");
        return EXIT_FAILURE;
    }

    //Criação das listas
    List l1, l2, l3;

    initList(&l1);
    initList(&l2);
    initList(&l3);

    ListPair pair1 = {&l1, &l2};
    ListPair pair2 = {&l2, &l3};

    //Criação das threads
    pthread_t l1l2Thread, l2l3Thread, l3_Thread;

    pthread_create(&l1l2Thread, NULL, filterOdd, (void *)&pair1);
    pthread_create(&l2l3Thread, NULL, filterPrime, (void *)&pair2);
    pthread_create(&l3_Thread, NULL, printResult, (void *)&l3);

    //Tarefa da thread principal, ler os numeros no arquivo
    int data;
    while(fscanf(file, "%d", &data) != EOF){
        insertList(&l1, data);
        sem_post(oddsSemaphore);
    }
    //Ao encerrar, insere um elementos invalido para encerrar a execução das threads subsequentes
    insertList(&l1, POISON_PILL);
    sem_post(oddsSemaphore);

    fclose(file);
    //Espera as threads secundarias
    pthread_join(l1l2Thread, NULL);
    pthread_join(l2l3Thread, NULL);
    pthread_join(l3_Thread, NULL);
    //Libera os recursos dos semaforos
    sem_close(oddsSemaphore);
    sem_close(primeSemaphore);
    sem_close(printSemaphore);

    sem_unlink("/T4_odds_sem");
    sem_unlink("/T4_primes_sem");
    sem_unlink("/T4_print_sem");
    //Libera os recursos das listas
    destroyList(&l1);
    destroyList(&l2);
    destroyList(&l3);

    return 0;
}