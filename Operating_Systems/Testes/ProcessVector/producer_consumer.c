#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Funcionamento: Sempre que uma thread transitar de nó para nó ela requisita acesso exclusivo
//Lista com nó cabeça

#define POISON_PILL -1
#define FINISH_OPERATIONS 3

typedef struct Node{
    int data;
    int operationsPerformed;
    struct Node* next;
    pthread_mutex_t lock;
}Node;

typedef struct{
    Node head;
    Node *tail;
}List;

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
    Node aux;
    aux.next = NULL;
    pthread_mutex_init(&(aux.lock), NULL);
    list->head = aux;
    list->tail = &(list->head);
}

void insertList(List *list, int value)
{
    Node *newNode = safeAllocation(sizeof(Node));
    newNode->next = NULL;
    newNode->data = value;  
    newNode->operationsPerformed = 0;
    pthread_mutex_init(&newNode->lock, NULL);
    
    pthread_mutex_lock(&(list->tail->lock));
    list->tail->next = newNode;
    pthread_mutex_unlock(&(list->tail->lock));
}

void destroyList(List *list)
{
    Node *iterator = list->head.next;
    while(iterator != NULL){
        Node *aux = iterator->next;
        free(iterator);
        iterator = aux;
    }
    list->head.next = NULL;
    list->tail = &(list->head);
}

void *removeEven(void *params){
    List *list = (List *) params;
    
    Node *previous = &(list->head), *current = NULL;
    while(1){
        pthread_mutex_lock(&(previous->lock));
        current = previous->next;
        if(current != NULL){
            pthread_mutex_lock(&(current->lock));
            if(current->data == POISON_PILL){
                pthread_mutex_unlock(&(previous->lock));
                pthread_mutex_unlock(&(current->lock));
                pthread_exit(0);
            }
            else if(current->data > 2 && current->data % 2){
                previous->next = current->next;
                pthread_mutex_unlock(&(previous->lock));
                pthread_mutex_unlock(&(current->lock));
                free(current);
            }
            else{
                pthread_mutex_unlock(&(current->lock));
            }
            previous = current;
        }
        else{
            pthread_mutex_unlock(&(previous->lock));
        }
    }
}

void *removeNotPrime(void *params){
    List *list = (List *) params;

}

void *printList(void *params){
    List *list = (List *) params;

}

int main(){


    return 0;
}





