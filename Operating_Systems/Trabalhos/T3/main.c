#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 128

typedef struct BurstList{
    unsigned burst;
    struct BurstList *next;
}BurstList;

typedef struct{
    unsigned priority;
    unsigned submissionInstant;
    BurstList *bursts;
}ProcessData;

typedef struct ProcessList{
    ProcessData data;
    struct ProcessList *next;
}ProcessList;

BurstList *insertBurstList(BurstList *list, unsigned burst){
    BurstList *new = malloc(sizeof(BurstList));
    
    *new = (BurstList){
        .burst = burst,
        .next = NULL
    };

    if(!list) return new;

    BurstList *aux = list;
    while(aux->next) aux = aux->next;
    aux->next = new; 

    return list;
}

ProcessList *insertProcessList(ProcessList *list, ProcessData data){
    ProcessList *new = malloc(sizeof(ProcessList));
    
    *new = (ProcessList){
        .data = data,
        .next = NULL
    };

    if(!list) return new;

    ProcessList *aux = list;
    while(aux->next) aux = aux->next;
    aux->next = new; 

    return list;
}

ProcessList *readDataFromFile(char *filename){
    FILE *file = fopen(filename, "r");
    if(!file) return NULL;
    
    char line[MAX_LINE];
    ProcessList *list = NULL;
    while (fgets(line, MAX_LINE, file)) {
        ProcessData data;
        char* token = strtok(line, " ");
        data.priority = atoi(token);

        token = strtok(NULL, "  ");
        data.submissionInstant = atoi(token);
        data.bursts = NULL;
        
        while((token = strtok(NULL, " \n")) != NULL) {
            data.bursts = insertBurstList(data.bursts, atoi(token));
        }
        list = insertProcessList(list, data);
    }
    
    fclose(file);
    return list;
}


int main(int argc, char **argv){
    if(!(argc == 2 || argc == 3)){
        printf("Numero invalido de argumentos\n Uso: <nome_arquivo> <quantum> <opt>");
        exit(EXIT_FAILURE);
    }

    ProcessList *list = readDataFromFile(argv[1]);

    return 0;
}