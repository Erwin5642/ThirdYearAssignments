#include "list.h"

// Adiciona um novo no na frente da lista encadeada
GasStationNode *addGasStation(GasStationNode *head, double coordinates[2], int diesel, int alcohol, int gas) {
    GasStationNode *newNode = malloc(sizeof(GasStationNode));
    if (!newNode) return NULL;

    newNode->coordinates[0] = coordinates[0];
    newNode->coordinates[1] = coordinates[1];
    newNode->dieselPrice = diesel;
    newNode->alcoholPrice = alcohol;
    newNode->gasPrice = gas;
    newNode->next = head;

    return newNode;
}

// Libera a memoria da lista encadeada
GasStationNode *freeGasStationList(GasStationNode *head) {
    GasStationNode *temp;
    while (head) {
        temp = head->next;
        free(head);
        head = temp;
    }
    return NULL;
}
