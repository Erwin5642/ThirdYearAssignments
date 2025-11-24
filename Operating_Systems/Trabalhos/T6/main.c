// Aluno: João Vitor Antunes da Silva    RGM: 48935

/*
Faça um programa em C que calcule a quantidade de erros de páginas
para os seguintes algoritmos de substituição:

FIFO;
OPT;
LRU.
Os argumentos de linha de comando do programa são, nessa ordem:
1) O tamanho em bytes de cada página;
2) O tamanho em bytes da memória disponível para alocação de páginas;
3) O nome de um arquivo contendo uma sequência de endereços acessados.
Esse, no formato texto, conterá endereços separados por um espaço cujos
valores variam 0 a 65.535. Em cada acesso será referenciado apenas um byte.

Ao final da execução devem ser apresentados na tela, para cada algoritmo,
o número de erros de página acompanhado do percentual de erros em relação
à quantidade de endereços acessados. Também deve ser gravado no arquivo
texto erros.out os endereços e páginas que ocasionaram os erros de página
para cada algoritmo de substituição.

O arquivo seq-20-paginas-10.txt é um exemplo de entrada com 123 endereços
acessados. A execução do programa com páginas de tamanho 10 e tamanho de
memória variando de 30 a 39 (3 quadros), irá produzir o arquivo erros.out
a seguir, com taxa de erros de página de 12,19% para o FIFO,  7,31% para
o OPT e 9,75% para o LRU.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// FIFO -> Fila

// OPT -> Vetor com próximo uso

// LRU -> Pilha

typedef struct AddressNode {
    int address;
    struct AddressNode *next;
} AddressNode;

AddressNode *createAddressNode(int address)
{
    AddressNode *newNode = malloc(sizeof(AddressNode));

    *newNode = (AddressNode){
        .address = address,
        .next = NULL};

    return newNode;
}

AddressNode *appendAddress(AddressNode *head, int address)
{
    AddressNode *newNode = createAddressNode(address);

    if (!head)
        return newNode;

    AddressNode *current = head;
    while (current->next)
        current = current->next;

    current->next = newNode;
    return head;
}

AddressNode *loadAddressesList(const char *filename)
{
    FILE *file = fopen(filename, "r");

    if(!file){
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);

    }

    AddressNode *addressList = NULL;

    int address;
    while(fscanf(file, "%d", &address) != EOF)
        addressList = appendAddress(addressList, address);

    fclose(file);

    return addressList;
}

AddressNode *freeAddressList(AddressNode *head){
    while(head){
        AddressNode *next = head->next;
        free(head);
        head = next;
    }
    return NULL;
}

int isPageInMemory(int *frames, int size, int page) {
    for (int i = 0; i < size; i++)
        if (frames[i] == page) return 1;
    return 0;
}

int getPageNumber(int address, int pageSize) {
    return address / pageSize;
}

void runFIFO(FILE *outputFile, int pageSize, int avaiableMemorySize, AddressNode *addressList){
    int frameCount = avaiableMemorySize / pageSize;
    int *frames = malloc(sizeof(int) * frameCount);
    int front = 0, size = 0;

    int misses = 0, accesses = 0;
    
    fprintf(outputFile, "FIFO:\n");
    printf("FIFO\n");

    AddressNode *current = addressList;
    while (current)
    {
        int page = getPageNumber(current->address, pageSize);
        
        if(!isPageInMemory(frames, size, page)){
            fprintf(outputFile, "erro de pagina endereço :%d pagina: %d\n", current->address, current->address/pageSize);

            if(size < frameCount){
                frames[size++] = page;
            }
            else{
                frames[front] = page;
                front = (front + 1) % frameCount;
            }

            misses += 1;
        }

        accesses += 1;
        current = current->next;
    }

    printf("Numero de Erros: %d\nTaxa de erros: %.2f%%\n", misses, (100.0 * misses) / accesses);
    fprintf(outputFile, "\n");

    free(frames);
}

int findNextUseTime(int page, AddressNode *start, int startTime, int pageSize){
    AddressNode *current = start;

    int t = startTime;
    while(current){
        int pageToCompare = getPageNumber(current->address, pageSize);

        if(page == pageToCompare){
            return t;
        }

        t += 1;
        current = current->next;
    }

    return INT_MAX;
}

int findFarthest(int *nextUse, int framesCount){
    int farthestDistance = -1, farthest = -1;
    for(int i = 0; i < framesCount; i++){
        if(nextUse[i] > farthestDistance){
            farthest = i;
            farthestDistance = nextUse[i];
        }
    }
    return farthest;
}

void runOPT(FILE *outputFile, int pageSize, int avaiableMemorySize, AddressNode *addressList){
    int frameCount = avaiableMemorySize / pageSize;
    int *frames = malloc(sizeof(int) * frameCount);
    int *nextUse = malloc(sizeof(int) * frameCount);
    int size = 0;

    int misses = 0, accesses = 0;
    
    fprintf(outputFile, "OPT:\n");
    printf("OPT\n");

    AddressNode *current = addressList;
    int currentTime = 0;
    while (current)
    {
        int page = getPageNumber(current->address, pageSize);
     
        int hit = 0;
        for (int i = 0; i < size; i++)
            if (frames[i] == page){
                nextUse[i] = findNextUseTime(page, current->next, currentTime + 1, pageSize);
                hit = 1;
                break;
            } 

        if(!hit){
            fprintf(outputFile, "erro de pagina endereço :%d pagina: %d\n", current->address, current->address/pageSize);

            if(size < frameCount){
                frames[size] = page;
                nextUse[size] = findNextUseTime(page, current->next, currentTime + 1, pageSize);
                size += 1;
            }
            else{
                int farthest = findFarthest(nextUse, frameCount);
                frames[farthest] = page;
                nextUse[farthest] = findNextUseTime(page, current->next, currentTime + 1, pageSize);
            }

            misses += 1;
        }

        accesses += 1;
        currentTime += 1;
        current = current->next;        
    }

    printf("Numero de Erros: %d\nTaxa de erros: %.2f%%\n", misses, (100.0 * misses) / accesses);
    fprintf(outputFile, "\n");

    free(frames);
    free(nextUse);
}

int findOldestUsed(int *lastUsed, int framesCount){
    int oldestTime = INT_MAX, oldest = -1;
    for(int i = 0; i < framesCount; i++){
        if(lastUsed[i] < oldestTime){
            oldest = i;
            oldestTime = lastUsed[i];
        }
    }
    return oldest;
}


void runLRU(FILE *outputFile, int pageSize, int avaiableMemorySize, AddressNode *addressList){
    int frameCount = avaiableMemorySize / pageSize;
    int *frames = malloc(sizeof(int) * frameCount); 
    int *lastUsed = malloc(sizeof(int) * frameCount);
    int size = 0;

    int misses = 0, accesses = 0;
    
    fprintf(outputFile, "LRU:\n");
    printf("LRU\n");

    AddressNode *current = addressList;
    int currentTime = 0;
    while (current)
    {
        int page = getPageNumber(current->address, pageSize);
     
        int hit = 0;
        for (int i = 0; i < size; i++)
            if (frames[i] == page){
                lastUsed[i] = currentTime;
                hit = 1;
                break;
            } 

        if(!hit){
            fprintf(outputFile, "erro de pagina endereço :%d pagina: %d\n", current->address, current->address/pageSize);

            if(size < frameCount){
                frames[size] = page;
                lastUsed[size] = currentTime;
                size += 1;
            }
            else{
                int oldest = findOldestUsed(lastUsed, frameCount);
                frames[oldest] = page;
                lastUsed[oldest] = currentTime;
            }

            misses += 1;
        }

        accesses += 1;
        currentTime += 1;
        current = current->next;           
    }

    printf("Numero de Erros: %d\nTaxa de erros: %.2f%%\n", misses, (100.0 * misses) / accesses);
    fprintf(outputFile, "\n");

    free(frames);
    free(lastUsed);
}

void simulatePageReplacementAlgorithms(int pageSize, int avaiableMemorySize, AddressNode *addressList){
    FILE *outputFile = fopen("error.out", "w");

    runFIFO(outputFile, pageSize, avaiableMemorySize, addressList);
    runOPT(outputFile, pageSize, avaiableMemorySize, addressList);
    runLRU(outputFile, pageSize, avaiableMemorySize, addressList);

    fclose(outputFile);
}

int main(int argc, const char *argv[])
{
    if(argc != 4){
        printf("Numero invalido de argumentos!\n");
        return EXIT_FAILURE;
    }

    AddressNode *addressList = loadAddressesList(argv[3]);
    simulatePageReplacementAlgorithms(atoi(argv[1]), atoi(argv[2]), addressList);

    freeAddressList(addressList);

    return 0;
}