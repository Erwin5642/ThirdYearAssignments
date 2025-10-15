/*
 * @Author: Victor Keony
 * @Description: Trabalho 4 de Sistemas Operacionais (semáforos)
 * @Github: keon-y
 */

//COMPILAR COM A FLAG -pthread

#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <unistd.h>          

sem_t *sem_odds;
sem_t *sem_primes;
sem_t *sem_printer;

typedef struct Node {
    struct Node* next;
    int value;
} Node;

typedef struct Lists {
    Node* L1;
    Node* L2;
    Node* L3;
} Lists;

/// @brief cria um nó com um valor inteiro passado por parametro
/// @param val inteiro
/// @return ponteiro para o nó criado
Node* createNode(int val) {
    Node* created_node = malloc(sizeof(Node));
    if (!created_node) return NULL;
    created_node->next = NULL;
    created_node->value = val;
    return created_node;
}

/// @brief limpa uma lista inteira
/// @param ptr ponteiro para a lista
void clearList(Node* ptr){
    while(ptr) {
        Node* aux = ptr;
        ptr = ptr->next;
        free(aux);
    }
}

/// @brief insere um elemento no final de uma lista
/// @param ptr ponteiro para algum elemento da lista (não necessariamente o primeiro)
/// @param value valor a ser inserido
void pushValue(Node** ptr, int value) {
    if(! *(ptr) ) { //caso não acontece, mas é bom garantir no código
        *ptr = createNode(value);
        return;
    }
    if ( (*ptr)->value == -1 ) { //lista inicializada com -1 apenas substituir os valores
        (*ptr)->value = value;
        return;
    }

    Node* runner = *ptr;
    while (runner->next != NULL) {
        runner = runner->next;
    }
    runner->next = createNode(value);
}

/// @brief verifica se um número é primo
/// @param número a ser verificado 
/// @return 1 se for primo, 0 se não for
int is_prime(int n) {
    if (n % 2 == 0)
        return n == 2;
    
    for (int i = 3; i <= n / i; i += 2) {
        if (n % i == 0) 
            return 0;
    }
    return n > 1;
}

/// @brief pega os valores pares de L1 e coloca em L2
/// @param parameters Lists* que aponta para as listas
/// @return null
void* thread_getOdds(void* parameters) {
    Lists *lists = (Lists*) parameters;
    Node* runner = NULL;
    Node* list_tail = NULL;

    while (1) {
        sem_wait(sem_odds);
        
        if (!runner) {
            runner = lists->L1;
            list_tail = lists->L2;
        }
        else runner = runner->next;

        if (runner->value == -2) {
            pushValue(&list_tail, -2);
            sem_post(sem_primes);
            break;
        }

        if (runner->value & 1) {
            pushValue(&list_tail, runner->value);
            if(list_tail->next) list_tail = list_tail->next;
            sem_post(sem_primes);
        }
    }
    pthread_exit(NULL);
}

/// @brief thread que pega os valores primos de L2 e coloca em L3
/// @param parameters Lists* que aponta para as listas
/// @return null 
void* thread_getPrimes(void* parameters) {
    Lists *lists = (Lists*) parameters;
    Node* runner = NULL;
    Node* list_tail = NULL;

    while (1) {
        sem_wait(sem_primes);

        if (!runner) {
            runner = lists->L2;
            list_tail = lists->L3;
        }
        else runner = runner->next;

        if (runner->value == -2) {
            pushValue(&list_tail, -2);
            sem_post(sem_printer);
            break;
        }

        if (is_prime(runner->value)) {
            pushValue(&list_tail, runner->value);
            if(list_tail->next) list_tail = list_tail->next;
            sem_post(sem_printer);
        }
    }
    pthread_exit(NULL);
}

/// @brief thread que imprime os primos de L3
/// @param parameters Lists* que aponta para as listas
/// @return null
void* thread_printPrimes(void* parameters) {
    Lists *lists = (Lists*) parameters;
    Node* runner = NULL;

    while (1) {
        sem_wait(sem_printer);

        if (!runner) runner = lists->L3;
        else runner = runner->next;

        if(runner->value == -2) break;

        printf("%d ", runner->value);
    }
    pthread_exit(NULL);
}


int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Erro! uso do comando: %s <nome do arquivo>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int arrSize = atoi(argv[1]), *arr = malloc(sizeof(int) * arrSize);
    for(int i = 0; i < arrSize; i++)
    {
        arr[i] = rand() % 100;
    }
    Lists lists;

    lists.L1 = createNode(-1); //inicializar o primeiro elemento, ele muda depois
    lists.L2 = createNode(-1);
    lists.L3 = createNode(-1);

    Node* L1_tail = lists.L1; //como pode ter listas grandes, a insercao no final fica mais rapida se a gente tiver um ponteiro para o ultimo elemento

    sem_unlink("/sem_odds");
    sem_unlink("/sem_primes");
    sem_unlink("/sem_printer");

    sem_odds = sem_open("/sem_odds", O_CREAT, 0644, 0);
    sem_primes = sem_open("/sem_primes", O_CREAT, 0644, 0);
    sem_printer = sem_open("/sem_printer", O_CREAT, 0644, 0);

    if (sem_odds == SEM_FAILED || sem_primes == SEM_FAILED || sem_printer == SEM_FAILED) {
        perror("Falha ao criar semáforo");
        exit(EXIT_FAILURE);
    }

    pthread_t t_getOdds, t_getPrimes, t_printer;
    pthread_create(&t_getOdds, NULL, thread_getOdds, &lists);
    pthread_create(&t_getPrimes, NULL, thread_getPrimes, &lists);
    pthread_create(&t_printer, NULL, thread_printPrimes, &lists);
    
    for(int i = 0; i < arrSize; i++) {
        pushValue(&L1_tail, arr[i]);
        if (L1_tail->next) L1_tail = L1_tail->next;
        sem_post(sem_odds);
    }

    pushValue(&lists.L1, -2); // Avisar que acabou
    sem_post(sem_odds);

    pthread_join(t_getOdds, NULL);
    pthread_join(t_getPrimes, NULL);
    pthread_join(t_printer, NULL);

    clearList(lists.L1);
    clearList(lists.L2);
    clearList(lists.L3);

    sem_close(sem_odds);
    sem_close(sem_primes);
    sem_close(sem_printer);

    sem_unlink("/sem_odds");
    sem_unlink("/sem_primes");
    sem_unlink("/sem_printer");

    return 0;
}