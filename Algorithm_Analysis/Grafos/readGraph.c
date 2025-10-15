#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct AdjacencyList {
    int label;
    struct AdjacencyList *next;
} AdjacencyList;

typedef struct Vertex {
    int label;
    unsigned color : 2;
    unsigned d;
    unsigned f;
    struct Vertex *parent;
    AdjacencyList *adjList;
} Vertex;

typedef struct {
    Vertex *vertexList;
    int n;
} Graph;

typedef struct QueueNode {
    Vertex *v;
    struct QueueNode *next;
} QueueNode;

// ------------------ Adjacency List -------------------

AdjacencyList *insertAdjacentList(Vertex *head, int label) {
    AdjacencyList *newAdjacentVertex = malloc(sizeof(AdjacencyList));
    newAdjacentVertex->label = label;
    newAdjacentVertex->next = head->adjList;
    head->adjList = newAdjacentVertex;
    return newAdjacentVertex;
}

void deleteAdjacencyList(Vertex *head) {
    AdjacencyList *current = head->adjList;
    while (current) {
        AdjacencyList *next = current->next;
        free(current);
        current = next;
    }
    head->adjList = NULL;
}

void deleteGraph(Graph g) {
    for (int i = 0; i < g.n; i++) {
        deleteAdjacencyList(&g.vertexList[i]);
    }
    free(g.vertexList);
}

// ------------------ Queue (for BFS) -------------------

QueueNode *enqueue(QueueNode *queue, Vertex *v) {
    QueueNode *newNode = malloc(sizeof(QueueNode));
    newNode->v = v;
    newNode->next = NULL;

    if (queue == NULL) {
        return newNode;
    }

    QueueNode *iterator = queue;
    while (iterator->next) {
        iterator = iterator->next;
    }
    iterator->next = newNode;
    return queue;
}

QueueNode *dequeue(QueueNode **queue, Vertex **v) {
    if (*queue == NULL)
        return NULL;

    QueueNode *front = *queue;
    *v = front->v;
    *queue = front->next;
    free(front);
    return *queue;
}

// ------------------ Graph Traversal -------------------

void printGraph(Graph g) {
    for (int i = 0; i < g.n; i++) {
        Vertex *v = &g.vertexList[i];
        printf("Vertex %d (d=%u, f=%u, color=%u, parent=%d):",
               v->label, v->d, v->f, v->color,
               v->parent ? v->parent->label : -1);

        AdjacencyList *adj = v->adjList;
        while (adj != NULL) {
            printf(" -> %d", adj->label);
            adj = adj->next;
        }
        printf("\n");
    }
}

void bfs(Graph g, Vertex *s) {
    for (int i = 0; i < g.n; i++) {
        g.vertexList[i].color = 0;
        g.vertexList[i].d = INT_MAX;
        g.vertexList[i].parent = NULL;
    }

    s->color = 1;
    s->d = 0;
    s->parent = NULL;

    QueueNode *queue = NULL;
    queue = enqueue(queue, s);

    while (queue != NULL) {
        Vertex *u;
        queue = dequeue(&queue, &u);

        AdjacencyList *adj = u->adjList;
        while (adj != NULL) {
            Vertex *v = &g.vertexList[adj->label - 1];
            if (v->color == 0) {
                v->color = 1;
                v->d = u->d + 1;
                v->parent = u;
                queue = enqueue(queue, v);
            }
            adj = adj->next;
        }
        u->color = 2;
    }
}

int time;

void dfsVisit(Graph G, Vertex *u) {
    time++;
    u->d = time;
    u->color = 1;

    AdjacencyList *adj = u->adjList;
    while (adj != NULL) {
        Vertex *v = &G.vertexList[adj->label - 1];
        if (v->color == 0) {
            v->parent = u;
            dfsVisit(G, v);
        }
        adj = adj->next;
    }

    u->color = 2;
    time++;
    u->f = time;
}

void dfs(Graph G) {
    for (int i = 0; i < G.n; i++) {
        G.vertexList[i].color = 0;
        G.vertexList[i].parent = NULL;
    }

    time = 0;

    for (int i = 0; i < G.n; i++) {
        if (G.vertexList[i].color == 0) {
            dfsVisit(G, &G.vertexList[i]);
        }
    }
}

// ------------------ Main -------------------

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
        return 1;

    int n, v1, v2;
    Graph g;

    fscanf(file, "%d", &n);
    g.vertexList = malloc(n * sizeof(Vertex));
    g.n = n;

    for (int i = 0; i < n; i++) {
        g.vertexList[i].label = i + 1;
        g.vertexList[i].adjList = NULL;
        g.vertexList[i].color = 0;
        g.vertexList[i].d = 0;
        g.vertexList[i].f = 0;
        g.vertexList[i].parent = NULL;
    }

    while (fscanf(file, "%d %d", &v1, &v2) != EOF) {
        insertAdjacentList(&g.vertexList[v1 - 1], v2);
    }

    fclose(file);

    bfs(g, &(g.vertexList[2]));
    printGraph(g);
    deleteGraph(g);

    return 0;
}