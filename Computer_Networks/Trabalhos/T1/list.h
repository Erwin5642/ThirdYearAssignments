#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct GasStationNode {
    double coordinates[2];        
    int dieselPrice;              
    int alcoholPrice;             
    int gasPrice;                 
    struct GasStationNode *next;  
} GasStationNode;

GasStationNode *addGasStation(GasStationNode *head, double coordinates[2], int diesel, int alcohol, int gas);

GasStationNode *freeGasStationList(GasStationNode *head);

#endif