#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "gasStationProtocol.h"
void sendRequest(int sockfd, struct sockaddr *serverAddress, RequestMessage *request)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);
    AckNakMessage ackNakResponse;
    static int requestId;
    request->requestId = requestId++;
    logMessage(LOG_INFO, "[MSG id=%d] Mensagem a ser enviada: %c %d %d %lf %lf",
               request->requestId, request->messageType, request->fuelType,
               request->priceOrRadius, request->coordinates[0], request->coordinates[1]);
    while (1)
    {
        request->errorFlag = simulateError();
        if (sendto(sockfd, request, sizeof(RequestMessage), 0, serverAddress, serverAdressLength) < 0)
        {
            logMessage(LOG_WARN, "[CLIENT] Erro ao enviar dados");
        }
        else if (recvfrom(sockfd, &ackNakResponse, sizeof(AckNakMessage), 0, serverAddress, &serverAdressLength) < 0)
        {
            logMessage(LOG_WARN, "[CLIENT] Erro ao receber dados");
        }
        else if (ackNakResponse.requestId == request->requestId)
        {
            logMessage(LOG_INFO, "[MSG id=%d] Resposta recebida: %c",       
                ackNakResponse.requestId, ackNakResponse);
            if (ackNakResponse.messageType == 'A')
            {
                logMessage(LOG_SUCCESS, "[MSG id=%d] Mensagem recebida pelo server com sucesso", request->requestId);
                return;
            }
            if (ackNakResponse.messageType == 'N')
            {
                logMessage(LOG_WARN, "[MSG id=%d] Mensagem recebida pelo server com erro", request->requestId);
            }
        }
        else{
            logMessage(LOG_ERROR, "[MSG id=%d] Mensagem perdida", request->requestId);
        }
        logMessage(LOG_INFO, "[MSG id=%d] Enviando novamente a mensangem", request->requestId);
    }
    return;
}

void getResponse(int sockfd, struct sockaddr *serverAddress, ResponseMessage *response)
{
    socklen_t serverAdressLength = sizeof(*serverAddress);

    while (recvfrom(sockfd, response, sizeof(ResponseMessage), 0, serverAddress, &serverAdressLength) < 0)
    {
        logMessage(LOG_WARN, "[CLIENT] Erro ao receber dados");
        logMessage(LOG_INFO, "[CLIENT] Esperando novamente uma mensangem");
    }
    return;
}

void printSearchResult(const RequestMessage *request, const ResponseMessage *response) {
    logMessage(LOG_INFO, "[MSG id=%d] Resposta recebida: %d %lf %lf",       
                response->requestId, response->minPrice,
                response->coordinates[0], response->coordinates[1]);
    if (response->minPrice == -1) {
        logMessage(LOG_INFO, "[MSG id=%d] Nenhum posto encontrado num raio de %d km a partir de (%lf, %lf) para combustível tipo %d",
            request->requestId, request->priceOrRadius, request->coordinates[0], request->coordinates[1], request->fuelType);
    } else {
        logMessage(LOG_INFO, "[MSG id=%d] Melhor posto encontrado para combustível tipo %d num raio de %d km:\n\tPreço: %d\n\tLocalização: (%lf, %lf)",
            request->requestId, request->fuelType, request->priceOrRadius, response->minPrice, response->coordinates[0], response->coordinates[1]);
    }
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

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        logMessage(LOG_ERROR, "[SOCKET] Erro ao criar socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in serverAddr;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status;
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        logMessage(LOG_ERROR, "[DNS] Falha ao resolver endereço '%s': %s", argv[1], gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    memcpy(&serverAddr, res->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(res);

    char userInput[MAX_USER_INPUT];
    RequestMessage request;
    ResponseMessage response;
    int continueSending = 1;

    while (continueSending) {
        printf("Insira a mensagem (<tipo> <tipo_combustível> <preço/raio(km)> <latitude> <longitude>): ");
        if (!fgets(userInput, sizeof(userInput), stdin)) {
            logMessage(LOG_WARN, "Erro na leitura de entrada do usuario");
            continue;
        }

        if (sscanf(userInput, " %c %d %d %lf %lf", &request.messageType, &request.fuelType,
                   &request.priceOrRadius, &request.coordinates[0], &request.coordinates[1]) != 5) {
            logMessage(LOG_WARN, "Entrada invalida. Tente novamente");
            continue;
        }

        sendRequest(sockfd, (struct sockaddr *)&serverAddr, &request);

        if (request.messageType == 'P') {
            getResponse(sockfd, (struct sockaddr *)&serverAddr, &response);
            printSearchResult(&request, &response);
        }

        printf("\nDeseja enviar outra solicitação? (1=Sim, 0=Não): ");
        if (!fgets(userInput, sizeof(userInput), stdin)) break;
        sscanf(userInput, "%d", &continueSending);
    }

    close(sockfd);
    return 0;
}