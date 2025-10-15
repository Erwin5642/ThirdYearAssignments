#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_SIZE_PER_THREADS 100000

typedef struct {
    int *arr;
    int left;
    int mid;
    int right;
} merge_sort_args;

void merge(int *arr, int left, int mid, int right) {
    int *temp = malloc(sizeof(int) * (right - left));
    
    int leftIterator = left, rightIterator = mid, tempIterator = 0;

    while (leftIterator < mid  && rightIterator < right) {
        temp[tempIterator++] = arr[leftIterator] < arr[rightIterator] ? arr[leftIterator++] : arr[rightIterator++];
    }
    
    while (leftIterator < mid) {
        temp[tempIterator++] = arr[leftIterator++];
    }
        
    while (rightIterator < right) {
        temp[tempIterator++] = arr[rightIterator++];
    }

    for(int i = 0; i < tempIterator; i++){
        arr[left + i] = temp[i];
    }
    
    free(temp);
}

void merge_sort(merge_sort_args *args){
    merge_sort_args *data = args;
    int *arr = data->arr;
    int left = data->left;
    int mid = data->mid;
    int right = data->right;
    
    if (left < mid) {
        merge_sort_args left_args = {arr, left, (left + mid) / 2, mid};
        merge_sort_args right_args = {arr, mid, (mid + right) / 2, right};
        
        merge_sort(&left_args);
        merge_sort(&right_args);
        merge(arr, left, mid, right);
    }
}

void *merge_sort_threads(void *args) {
    merge_sort_args *data = (merge_sort_args *)args;
    int *arr = data->arr;
    int left = data->left;
    int mid = data->mid;
    int right = data->right;
    
    if (left < mid) {
        merge_sort_args left_args = {arr, left, (left + mid) / 2, mid};
        merge_sort_args right_args = {arr, mid, (mid + right) / 2, right};
        
        if(right - left > MAX_SIZE_PER_THREADS){
            // Sort the left and right halves in parallel
            pthread_t thread1, thread2;
            
            // If sub arry size is too big, create new threads
            pthread_create(&thread1, NULL, merge_sort_threads, &left_args);
            pthread_create(&thread2, NULL, merge_sort_threads, &right_args);
            
            // Wait for both threads to finish
            pthread_join(thread1, NULL);
            pthread_join(thread2, NULL);    
        }
        else{
            merge_sort(&left_args);
            merge_sort(&right_args);
        }
        merge(arr, left, mid, right);
    }
    
    return NULL;
}

int checkSorted(int *arr, int arrSize){
    for(int i = 0; i < arrSize - 1; i++){
        if(arr[i] > arr[i + 1]){
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    if(argc != 2){
        printf("Numero de argumentos invalido. Formato correto: <quantidade de numeros> <numero de threads>\n");
        exit(EXIT_FAILURE);
    }
    int arrSize = atoi(argv[1]);
    int *arr = malloc(arrSize* sizeof(int));

    srand(time(NULL));
    for(int i = 0; i < arrSize; i++){
        arr[i] = rand() % (arrSize * 2);
    }
    
    merge_sort_args args = {arr, 0, arrSize/2, arrSize};
    
    // Create the initial thread for sorting
    pthread_t initial_thread;
    pthread_create(&initial_thread, NULL, merge_sort_threads, &args);
    
    // Wait for the sorting thread to finish
    pthread_join(initial_thread, NULL);
    
    printf(checkSorted(arr, arrSize) ? "Vetor ordenado\n" : "Nao foi possivel ordenar o vetor\n");
    printf("Numero de threads usadas %d\n", arrSize / MAX_SIZE_PER_THREADS + 1);
    free(arr);
    return 0;
}
