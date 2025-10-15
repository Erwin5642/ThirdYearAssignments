#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t itemsCounter;

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

void *createArray(void *params){
    int *localArr = (int *) params;
    
    for(int i = 0; i < 100; i++){
        localArr[i] = rand() % 100;
        sem_post(&itemsCounter);
    }

    pthread_exit(0);
}

void *printArray(void *params){
    int *localArr = (int *) params;
    
    for(int i = 0; i < 100; i++){
        sem_wait(&itemsCounter);
        printf("%d\n", localArr[i]);
    }

    pthread_exit(0);
}

int main(){
    int arr[100];
    pthread_t producer_t, consumer_t;
    sem_init(&itemsCounter, PTHREAD_PROCESS_PRIVATE, 0);
    pthread_join
    pthread_join(producer_t, NULL);
    sem_destroy(&itemsCounter);

    return 0;
}