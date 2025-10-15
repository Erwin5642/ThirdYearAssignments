#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define POISON_PILL -1

typedef struct Node
{
    int data;
    struct Node *next;
} Node;

typedef struct
{
    Node *head;
    Node *tail;
} List;

typedef struct {
    List *l1;
    List *l2;
}ListPair;

sem_t semaphoreItemsL1;
sem_t semaphoreItemsL2;
sem_t semaphoreItemsL3;

// Safe memory allocation function
void *safeAllocation(size_t size)
{
    void *ptr = malloc(size);
    // Check if malloc succeeded
    if (ptr == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    return ptr;
}

int isPrime(int n) {
    if (n == 1) return 0; 

    int limit = (int)sqrt(n);
    for (int i = 3; i <= limit; i += 2) { 
        if (n % i == 0) return 0;
    }
    return 1;
}

void initList(List *list)
{
    list->head = NULL;
    list->tail = NULL;
}

void insertList(List *list, int value)
{
    Node *newNode = safeAllocation(sizeof(Node));
    newNode->next = NULL;
    newNode->data = value;  
    if (list->tail == NULL)
    {
        list->tail = list->head = newNode;
        return;
    }
    list->tail->next = newNode;
    list->tail = newNode;
}

void destroyList(List *list)
{
    Node *iterator = list->head;
    while(iterator != NULL){
        Node *aux = iterator->next;
        free(iterator);
        iterator = aux;
    }
    list->head = list->tail = NULL;
}

void *getOdds(void *param){
    void **args = (void **) param;
    
    List *producer = (List *) (args[0]);
    List *consumer = (List *) (args[1]);

    Node *iterator = NULL;
    while(1){
        // Consuming L1
        sem_wait(&semaphoreItemsL1);

        if(iterator == NULL){
            iterator = producer->head;
        }
        else{
            iterator = iterator->next;
        }
        int data = iterator->data;

        // Producing L2
        if(data == POISON_PILL || data % 2 != 0){
            insertList(consumer, data);
            sem_post(&semaphoreItemsL2);
            if(data == POISON_PILL){
                printf("Get odds poisoned\n");
                pthread_exit(0);
            }
        }
    }
}

void *getPrimes(void *param){
    void **args = (void **) param;
    
    List *producer = (List *) (args[0]);
    List *consumer = (List *) (args[1]);

    Node *iterator = NULL;
    while(1){
        // Consuming L2
        sem_wait(&semaphoreItemsL2);

        if(iterator == NULL){
            iterator = producer->head;
        }
        else{
            iterator = iterator->next;
        }
        int data = iterator->data;

        // Producing L3
        if(data == POISON_PILL || isPrime(data)){
            insertList(consumer, data);
            sem_post(&semaphoreItemsL3);
            if(data == POISON_PILL){
                printf("Get primes poisoned\n");
                pthread_exit(0);
            }
        }
    }
}

void *getResult(void *param){
    List *producer = (List *) param;
    
    Node *iterator = NULL;
    while(1){
        // Consuming L3
        sem_wait(&semaphoreItemsL3);
    
        if(iterator == NULL){
            iterator = producer->head;
        }
        else{
            iterator = iterator->next;
        }

        // Producing 
        int data = iterator->data;
        if(data == POISON_PILL){
            printf("Get results poisoned\n");
            pthread_exit(0);        
        }
        printf("%d ", data);
    }
}

int main(int arg, char **argv)
{
    //Creating a testing array
    if(arg != 2){
        fprintf(stderr, "Numero incorreto de argumentos\n");
        exit(EXIT_FAILURE);
    }    
    
    int arrSize = atoi(argv[1]);
    int *arr = malloc(sizeof(int) * arrSize);

    srand(time(NULL));
    for(int i = 0; i < arrSize; i++){
        arr[i] = rand() % (arrSize * 10);
    }

    List *lists;
    lists = malloc(sizeof(List) * 3);
    initList(&lists[0]);
    initList(&lists[1]);
    initList(&lists[2]);

    pthread_t l1l2Thread, l2l3Thread, l3_Thread;
    sem_init(&semaphoreItemsL1, PTHREAD_PROCESS_PRIVATE, 0);
    sem_init(&semaphoreItemsL2, PTHREAD_PROCESS_PRIVATE, 0);
    sem_init(&semaphoreItemsL3, PTHREAD_PROCESS_PRIVATE, 0);

    void *params[2];
    params[0] = &lists[0];
    params[1] = &lists[1];
    pthread_create(&l1l2Thread, NULL, getOdds, (void *)params);
    params[0] = &lists[1];
    params[1] = &lists[2];
    pthread_create(&l2l3Thread, NULL, getPrimes, (void *)params);
    pthread_create(&l3_Thread, NULL, getResult, params[1]);

    for(int i = 0; i < arrSize; i++){
        insertList(&lists[0], arr[i]);
        sem_post(&semaphoreItemsL1);
    }
    insertList(&lists[0], POISON_PILL);
    sem_post(&semaphoreItemsL1);

    pthread_join(l1l2Thread, NULL);
    pthread_join(l2l3Thread, NULL);
    pthread_join(l3_Thread, NULL);

    sem_destroy(&semaphoreItemsL1);
    sem_destroy(&semaphoreItemsL2);
    sem_destroy(&semaphoreItemsL3);

    destroyList(&lists[0]);
    destroyList(&lists[1]);
    destroyList(&lists[2]);
    free(arr);
    free(lists);

    return 0;
}
