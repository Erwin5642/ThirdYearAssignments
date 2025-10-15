#include "utils.h"
#include "gasStationProtocol.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Carrega os dados sobre postos de um arquivo para uma lista encadeada e tres arvores KD (uma para cada combustivel)
GasStationNode *loadGasStationsFromFile(const char *filename, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return NULL;

    GasStationNode *head = NULL;
    double coordinates[2];
    int dieselPrice, alcoholPrice, gasPrice;

    while (fscanf(file, "%lf %lf %d %d %d", &coordinates[0], &coordinates[1], &dieselPrice, &alcoholPrice, &gasPrice) != EOF)
    {
        head = addGasStation(head, coordinates, dieselPrice, alcoholPrice, gasPrice);

        // Converte as coordenadas em latitude e longitude para km de forma aproximada
        double projectedPoint[2] = {coordinates[0], coordinates[1]};

        if (dieselPrice != -1)
        {
            *dieselTree = insertKdTreeNode(*dieselTree, projectedPoint, dieselPrice, 0);
        }
        if (alcoholPrice != -1)
        {
            *alcoholTree = insertKdTreeNode(*alcoholTree, projectedPoint, alcoholPrice, 0);
        }
        if (gasPrice != -1)
        {
            *gasTree = insertKdTreeNode(*gasTree, projectedPoint, gasPrice, 0);
        }
    }

    fclose(file);
    return head;
}

// Insere ou atualiza um dado sobre um posto (novo ou antigo) em uma lista encadeada e nas respectivas arvores KDs
GasStationNode *updateOrInsertGasStation(GasStationNode *head, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, double coordinates[2], int fuelType, int price)
{
    GasStationNode *current = head;
    double projectedPoint[2] = {coordinates[0], coordinates[1]};

    while (current)
    {
        // Se um posto for encontrado (dada uma margem de erro pequena), atualiza os dados dele
        if (fabs(current->coordinates[0] - coordinates[0]) < 0.0001 && fabs(current->coordinates[1] - coordinates[1]) < 0.0001)
        {
            switch (fuelType)
            {
            case 0:
                current->dieselPrice = price;
                *dieselTree = insertKdTreeNode(*dieselTree, projectedPoint, price, 0);
                break;
            case 1:
                current->alcoholPrice = price;
                *alcoholTree = insertKdTreeNode(*alcoholTree, projectedPoint, price, 0);
                break;
            case 2:
                current->gasPrice = price;
                *gasTree = insertKdTreeNode(*gasTree, projectedPoint, price, 0);
                break;
            }
            return head;
        }
        current = current->next;
    }

    // Se o posto for novo, insere o dado na sua respectiva arvore Kd e na lista encadeada
    switch (fuelType)
    {
    case 0:
        *dieselTree = insertKdTreeNode(*dieselTree, projectedPoint, price, 0);
        break;
    case 1:
        *alcoholTree = insertKdTreeNode(*alcoholTree, projectedPoint, price, 0);
        break;
    case 2:
        *gasTree = insertKdTreeNode(*gasTree, projectedPoint, price, 0);
        break;
    default:
        return head;
        break;
    }
    return addGasStation(
        head,
        coordinates,
        (fuelType == 0) ? price : -1,
        (fuelType == 1) ? price : -1,
        (fuelType == 2) ? price : -1);
}

// Salva os dados presentes na lista encadeada em um arquivo
void saveGasStationsToFile(const char *filename, GasStationNode *head)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        logMessage(LOG_ERROR, "[FILE] Erro ao abrir arquivo para escrita\n");
        return;
    }

    GasStationNode *current = head;
    while (current)
    {
        fprintf(file, "%lf %lf %d %d %d\n",
                current->coordinates[0],
                current->coordinates[1],
                current->dieselPrice,
                current->alcoholPrice,
                current->gasPrice);
        current = current->next;
    }

    fclose(file);
}