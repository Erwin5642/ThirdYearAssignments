#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "gasStationProtocol.h"

#define NUM_THREADS 10
#define REQUEST_PER_CLIENT 9000

void sendRequest(int sockfd, struct sockaddr *serverAddress, RequestMessage *request)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    AckNakMessage ackNakResponse;
    static __thread int requestId;
    request->requestId = requestId++;
    logMessage(LOG_INFO, "[THREAD id=%d][MSG id=%d] Mensagem a ser enviada: %c %d %d %lf %lf",
                pthread_self(), request->requestId, request->messageType, request->fuelType,
               request->priceOrRadius, request->coordinates[0], request->coordinates[1]);
    while (1)
    {
        request->errorFlag = simulateError();
        if (sendto(sockfd, request, sizeof(RequestMessage), 0, serverAddress, serverAdressLength) < 0)
        {
            logMessage(LOG_WARN, "[THREAD id=%d][CLIENT] Erro ao enviar dados", pthread_self());
        }
        else if (recvfrom(sockfd, &ackNakResponse, sizeof(AckNakMessage), 0, serverAddress, &serverAdressLength) < 0)
        {
            logMessage(LOG_WARN, "[THREAD id=%d][CLIENT] Erro ao receber dados", pthread_self());
        }
        else if (ackNakResponse.requestId == request->requestId)
        {
            if (ackNakResponse.messageType == 'A')
            {
                logMessage(LOG_SUCCESS, "[THREAD id=%d][MSG id=%d] Mensagem recebida pelo server com sucesso", pthread_self(), request->requestId);
                return;
            }
            if (ackNakResponse.messageType == 'N')
            {
                logMessage(LOG_WARN, "[THREAD id=%d][MSG id=%d] Mensagem recebida pelo server com erro", pthread_self(), request->requestId);
            }
        }
        else{
            logMessage(LOG_ERROR, "[THREAD id=%d][MSG id=%d] Mensagem perdida", pthread_self(), request->requestId);
        }
        logMessage(LOG_INFO, "[THREAD id=%d][MSG id=%d] Enviando novamente a mensangem", pthread_self(), request->requestId);
    }
    return;
}

void getResponse(int sockfd, struct sockaddr *serverAddress, ResponseMessage *response)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    while (recvfrom(sockfd, response, sizeof(ResponseMessage), 0, serverAddress, &serverAdressLength) < 0)
    {
        logMessage(LOG_WARN, "[THREAD id=%d][CLIENT] Erro ao receber dados", pthread_self());
    }
    return;
}

void printSearchResult(const RequestMessage *request, const ResponseMessage *response)
{
    if (response->minPrice == -1)
    {
        logMessage(LOG_INFO, "[THREAD id=%d][MSG id=%d] Nenhum posto encontrado num raio de %d km a partir de (%lf, %lf) para combustível tipo %d",
                   pthread_self(),request->requestId, request->priceOrRadius, request->coordinates[0], request->coordinates[1], request->fuelType);
    }
    else
    {
        logMessage(LOG_INFO, "[THREAD id=%d][MSG id=%d] Melhor posto encontrado para combustível tipo %d num raio de %d km:\n\tPreço: %d\n\tLocalização: (%lf, %lf)",
                   pthread_self(), request->requestId, request->fuelType, request->priceOrRadius, response->minPrice, response->coordinates[0], response->coordinates[1]);
    }
}

typedef struct {
    int sockfd;
    struct sockaddr_in serverAddr;
} TestArgs;

double generateRandomCoord(double min, double max) {
    double scale = rand() / (double) RAND_MAX;
    return min + scale * (max - min);
}

void *threadTest(void *args) {
    TestArgs test = *(TestArgs *)args;
    struct sockaddr_in serverAddr = test.serverAddr;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        logMessage(LOG_ERROR, "[THREAD id=%d] Erro ao criar socket", pthread_self());
        pthread_exit(NULL);
    }

    RequestMessage request;
    ResponseMessage response;

    static __thread int requestId = 0;

    for (int i = 0; i < REQUEST_PER_CLIENT; i++) {
        request.messageType = (rand() % 2 == 0) ? 'D' : 'P';
        request.fuelType = rand() % 3;
        request.priceOrRadius = rand() % (9000 - i*i) + 1000;
        request.coordinates[0] = generateRandomCoord(-90.0 * i/REQUEST_PER_CLIENT, 90.0 * i/REQUEST_PER_CLIENT);
        request.coordinates[1] = generateRandomCoord(-180.0 * i/REQUEST_PER_CLIENT, 180.0 * i/REQUEST_PER_CLIENT);

        request.errorFlag = simulateError();
        request.requestId = requestId++;

        sendRequest(sockfd, (struct sockaddr *)&serverAddr, &request);

        if (request.messageType == 'P') {
            getResponse(sockfd, (struct sockaddr *)&serverAddr, &response);
            printSearchResult(&request, &response);
        }
    }

    close(sockfd);
    return NULL;
}

int main(int argc, char **argv) {
    srand(time(NULL));

    if (argc != 3) {
        logMessage(LOG_ERROR, "[USO] Uso: %s <endereco_ip> <porta>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    if (port < 0 || port > 65535) {
        logMessage(LOG_ERROR, "[USO] Porta inválida: %d", port);
        exit(EXIT_FAILURE);
    }

    // Prepare server address once
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    // Prepare thread arguments
    pthread_t threads[NUM_THREADS];
    TestArgs args;
    args.serverAddr = serverAddr;

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, threadTest, &args) != 0) {
            logMessage(LOG_ERROR, "[THREAD] Erro ao criar thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}