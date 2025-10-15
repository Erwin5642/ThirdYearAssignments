#ifndef UTILS_H
#define UTILS_H

#include "list.h"
#include "KdTree.h"

GasStationNode *loadGasStationsFromFile(const char *filename, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree);
GasStationNode *updateOrInsertGasStation(GasStationNode *head, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, double coordinates[2], int fuelType, int price);
void saveGasStationsToFile(const char *filename, GasStationNode *head);

#endif