#include <stdlib.h>
#include <stdio.h>
#include "KdTree.h"
#include <math.h>

#define EARTH_RADIUS_KM 6371.0

double degreesToRadians(double deg) {
    return deg * (M_PI / 180.0);
}

// Calcula a distancia entre dois pontos 
static double getHaversineDistance(double pointA[2], double pointB[2]) {
    double dLatitude = degreesToRadians(pointB[0] - pointA[0]);
    double dLongitude = degreesToRadians(pointB[1] - pointA[1]);
    double latitude1 = degreesToRadians(pointA[0]);
    double latitude2 = degreesToRadians(pointB[0]);

    double a = sin(dLatitude / 2) * sin(dLatitude / 2) +
               cos(latitude1) * cos(latitude2) *
               sin(dLongitude / 2) * sin(dLongitude / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return EARTH_RADIUS_KM * c;
}

// Aloca memória para um no da arvore Kd com as coordenadas e preco como parametro
KdTree *createKdTreeNode(double x, double y, int fuelPrice) {
    KdTree *node = malloc(sizeof(KdTree));
    *node = (KdTree){
        .coordinates[0] = x,
        .coordinates[1] = y,
        .fuelPrice = fuelPrice,
        .left = NULL,
        .right = NULL
    };
    return node;
}

// Libera a memoria ocupada por uma arvore Kd
KdTree *freeTree(KdTree *root) {
    if (!root) return NULL;
    root->left = freeTree(root->left);
    root->right = freeTree(root->right);
    free(root);
    return NULL;
}

// Encontra o no com o menor preco entre tres opcoes
KdTree *getMinPriceNode(KdTree *a, KdTree *b, KdTree *c) {
    KdTree *minNode = NULL;
    if (a) minNode = a;
    if (b && (!minNode || b->fuelPrice < minNode->fuelPrice)) minNode = b;
    if (c && (!minNode || c->fuelPrice < minNode->fuelPrice)) minNode = c;
    return minNode;
}

// Insere um novo no na arvore Kd baseado em coordenadas 2D
KdTree *insertKdTreeNode(KdTree *root, double coordinates[2], int fuelPrice, unsigned currentDim) {
    if (!root) return createKdTreeNode(coordinates[0], coordinates[1], fuelPrice);
    
    if (fabs(root->coordinates[0] - coordinates[0]) < 1e-4 &&
        fabs(root->coordinates[1] - coordinates[1]) < 1e-4) {
        root->fuelPrice = fuelPrice;
        return root;
    }
    
    if (root->coordinates[currentDim] > coordinates[currentDim]) {
        root->left = insertKdTreeNode(root->left, coordinates, fuelPrice, (currentDim + 1) % 2);
    } else{
        root->right = insertKdTreeNode(root->right, coordinates, fuelPrice, (currentDim + 1) % 2);
    }

    return root;
}

// Encontra o na arovre Kd o no com o menor preco dentro de uma regiao ciruclar
KdTree *findMinPriceInRadius(KdTree *root, double center[2], int radius, unsigned currentDim) {
    if (!root) return NULL;

    KdTree *best, *leftBest, *rightBest;
    best = leftBest = rightBest = NULL;

    double distanceToNode = getHaversineDistance(root->coordinates, center);
    if (distanceToNode <= radius) best = root;
    if (root->left && root->coordinates[currentDim] >= center[currentDim] - radius) {
        leftBest = findMinPriceInRadius(root->left, center, radius, (currentDim + 1) % 2);
    }
    if (root->right && root->coordinates[currentDim] <= center[currentDim] + radius) {
        rightBest = findMinPriceInRadius(root->right, center, radius, (currentDim + 1) % 2);
    }
    return getMinPriceNode(best, leftBest, rightBest);
}
