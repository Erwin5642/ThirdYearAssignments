#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "KdTree.h"
#include "list.h"
#include "utils.h"
#include "gasStationProtocol.h"

#define AUTO_SAVE_INTERVAL 5

static pthread_mutex_t stationsListLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dieselKdTreeLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t alcoholKdTreeLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gasKdTreeLock = PTHREAD_MUTEX_INITIALIZER;

static int dataChanged = 0;

pthread_mutex_t *getTreeLockByFuelType(int fuelType)
{
    switch (fuelType)
    {
    case 0: return &dieselKdTreeLock;
    case 1: return &alcoholKdTreeLock;
    case 2: return &gasKdTreeLock;
    default: return NULL;
    }
}

void acquireTreeLock(int fuelType)
{
    pthread_mutex_t *lock = getTreeLockByFuelType(fuelType);
    if (lock) pthread_mutex_lock(lock);
}

void releaseTreeLock(int fuelType)
{
    pthread_mutex_t *lock = getTreeLockByFuelType(fuelType);
    if (lock) pthread_mutex_unlock(lock);
}

void *autoSaveLoop(void *arg)
{
    GasStationNode **stationsList = (GasStationNode **)arg;
    while (1)
    {
        sleep(AUTO_SAVE_INTERVAL);
        if (dataChanged)
        {
            pthread_mutex_lock(&stationsListLock);
            saveGasStationsToFile("data.txt", *stationsList);
            dataChanged = 0;
            pthread_mutex_unlock(&stationsListLock);
            logMessage(LOG_INFO, "[SERVER][AutoSave] Dados salvos automaticamente");
        }
    }
    return NULL;
}

void receiveClientMessage(int sockfd, struct sockaddr *clientAddress, RequestMessage *request)
{
    socklen_t addressLength = sizeof(*clientAddress);
    AckNakMessage response;

    while (1)
    {
        ssize_t received = recvfrom(sockfd, request, sizeof(RequestMessage), 0, clientAddress, &addressLength);
        if (received < 0)
        {
            logMessage(LOG_WARN, "[SERVER] Falha ao receber dados");
            continue;
        }

        response.requestId = request->requestId;
        if (request->errorFlag)
        {
            logMessage(LOG_WARN, "[MSG id=%d] Mensagem corrompida. Enviando NAK", request->requestId);
            response.messageType = 'N';
        }
        else
        {
            logMessage(LOG_SUCCESS, "[MSG id=%d] Mensagem válida recebida. Enviando ACK", request->requestId);
            logMessage(LOG_INFO, "[MSG id=%d] Mensagem recebida: %c %d %d %lf %lf",
               request->requestId, request->messageType, request->fuelType,
               request->priceOrRadius, request->coordinates[0], request->coordinates[1]);
            response.messageType = 'A';
        }

        sendto(sockfd, &response, sizeof(AckNakMessage), 0, clientAddress, addressLength);
        if (!request->errorFlag) return;
    }
}

void handleInsertRequest(RequestMessage *request, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, GasStationNode **stationList)
{
    logMessage(LOG_INFO, "[MSG id=%d] Processando requisicao de dados", request->requestId);
    if (request->fuelType > 2 || request->fuelType < 0)
    {
        logMessage(LOG_WARN, "[MSG id=%d] Tipo de combustível inválido: %d", request->requestId, request->fuelType);
        return;
    }
    acquireTreeLock(request->fuelType);
    pthread_mutex_lock(&stationsListLock);
    *stationList = updateOrInsertGasStation(*stationList, dieselTree, alcoholTree, gasTree, request->coordinates, request->fuelType, request->priceOrRadius);
    dataChanged = 1;
    pthread_mutex_unlock(&stationsListLock);
    releaseTreeLock(request->fuelType);
    logMessage(LOG_SUCCESS, "[MSG id=%d] Dados armazenados com sucesso", request->requestId);
}

void handleSearchRequest(RequestMessage *request, ResponseMessage *response, KdTree *dieselTree, KdTree *alcoholTree, KdTree *gasTree)
{
    logMessage(LOG_INFO, "[MSG id=%d] Processando requisicao de pesquisa", request->requestId);
    if (request->fuelType > 2 || request->fuelType < 0)
    {
        logMessage(LOG_WARN, "[MSG id=%d] Tipo de combustível inválido: %d", request->requestId, request->fuelType);
        response->minPrice = -1;
        response->coordinates[0] = 0;
        response->coordinates[1] = 0;
        return;
    }

    KdTree *bestStation = NULL;
    double origin[2] = {
        request->coordinates[0],
        request->coordinates[1]};

    response->requestId = request->requestId;
    
    //Secao critica de leitura
    acquireTreeLock(request->fuelType);
    switch (request->fuelType)
    {
    case 0: bestStation = findMinPriceInRadius(dieselTree, origin, request->priceOrRadius, 0); break;
    case 1: bestStation = findMinPriceInRadius(alcoholTree, origin, request->priceOrRadius, 0); break;
    case 2: bestStation = findMinPriceInRadius(gasTree, origin, request->priceOrRadius, 0); break;
    }
    releaseTreeLock(request->fuelType);

    if (!bestStation)
    {
        response->minPrice = -1;
        response->coordinates[0] = 0;
        response->coordinates[1] = 0;
    }
    else
    {
        response->minPrice = bestStation->fuelPrice;
        response->coordinates[0] = bestStation->coordinates[0];
        response->coordinates[1] = bestStation->coordinates[1];
    }
}

void processRequest(RequestMessage request, ResponseMessage *response, KdTree **dieselTree, KdTree **alcoholTree, KdTree **gasTree, GasStationNode **stationList)
{
    switch (request.messageType)
    {
    case 'D': handleInsertRequest(&request, dieselTree, alcoholTree, gasTree, stationList); break;
    case 'P': handleSearchRequest(&request, response, *dieselTree, *alcoholTree, *gasTree); break;
    default: logMessage(LOG_WARN, "[MSG id=%d] Tipo de mensagem desconhecida: %c", request.requestId, request.messageType); break;
    }
}

void sendResponse(int sockfd, struct sockaddr *clientAddress, ResponseMessage *response)
{
    socklen_t len = sizeof(*clientAddress);
    sendto(sockfd, response, sizeof(ResponseMessage), 0, clientAddress, len);
    logMessage(LOG_SUCCESS, "[MSG id=%d] Resposta enviada: preco=%d latitude=%lf longitude=%lf", response->requestId, response->minPrice, response->coordinates[0], response->coordinates[1]);
}

typedef struct {
    int sockfd;
    struct sockaddr_in clientAddress;
    RequestMessage request;
    ResponseMessage response;
    KdTree **dieselTree, **alcoholTree, **gasTree;
    GasStationNode **stationList;
} WorkerArgs;

void *handleClientRequest(void *args)
{
    WorkerArgs *data = (WorkerArgs *)args;
    processRequest(data->request, &(data->response), data->dieselTree, data->alcoholTree, data->gasTree, data->stationList);
    if (data->request.messageType == 'P')
    {
        sendResponse(data->sockfd, (struct sockaddr *)&(data->clientAddress), &(data->response));
    }
    free(data);
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        logMessage(LOG_ERROR, "[USO] Uso: %s <porta>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        logMessage(LOG_ERROR, "[USO] Porta inválida: %d", port);
        exit(EXIT_FAILURE);
    }

    KdTree *dieselTree = NULL, *alcoholTree = NULL, *gasTree = NULL;
    GasStationNode *stationList = loadGasStationsFromFile("data.txt", &dieselTree, &alcoholTree, &gasTree);
    logMessage(LOG_INFO, "[SERVER] Base de dados carregada");

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        logMessage(LOG_ERROR, "[SOCKET] Erro ao criar socket");

        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY};

    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        logMessage(LOG_ERROR, "[SOCKET] Erro no bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pthread_t autoSaver;
    if (pthread_create(&autoSaver, NULL, autoSaveLoop, &stationList) != 0)
    {
        logMessage(LOG_ERROR, "[THREAD] Falha ao iniciar thread de salvamento automático");
        exit(EXIT_FAILURE);
    }
    pthread_detach(autoSaver);

    logMessage(LOG_INFO, "[SERVER] Servidor escutando na porta %d", port);

    struct sockaddr_in clientAddress;
    RequestMessage requestBuffer;
    while (1)
    {
        WorkerArgs *args = malloc(sizeof(WorkerArgs));
        if (!args)
        {
            logMessage(LOG_ERROR, "[THREAD] Falha na alocação de argumentos para thread de atendimento ao cliente");
            continue;
        }

        receiveClientMessage(sockfd, (struct sockaddr *)&clientAddress, &requestBuffer);
        
        *args = (WorkerArgs){
            .sockfd = sockfd,
            .clientAddress = clientAddress,
            .request = requestBuffer,
            .dieselTree = &dieselTree,
            .alcoholTree = &alcoholTree,
            .gasTree = &gasTree,
            .stationList = &stationList};

        pthread_t workerThread;
        if (pthread_create(&workerThread, NULL, handleClientRequest, args) != 0)
        {
            logMessage(LOG_ERROR, "[THREAD] Erro ao criar thread de atendimento ao cliente");
            free(args);
            continue;
        }
        pthread_detach(workerThread);
    }

    freeGasStationList(stationList);
    close(sockfd);
    freeTree(dieselTree);
    freeTree(alcoholTree);
    freeTree(gasTree);

    return 0;
}
