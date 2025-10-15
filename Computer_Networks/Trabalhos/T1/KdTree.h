#ifndef KDTREE_H
#define KDTREE_H

typedef struct KdTree {
    double coordinates[2]; // Coordenadas (x, y)
    int fuelPrice; // Preco associado
    struct KdTree *left, *right; // Filhos
}KdTree;

KdTree* createKdTreeNode(double xCoord, double yCoord, int nodePrice);
KdTree* insertKdTreeNode(KdTree* root, double point[2], int price, unsigned currentDim);
KdTree* findMinPriceInRadius(KdTree* root, double center[2], int radius, unsigned currentDim);
KdTree* freeTree(KdTree* root);

#endif
