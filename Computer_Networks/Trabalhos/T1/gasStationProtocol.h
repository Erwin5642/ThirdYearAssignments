#ifndef GAS_STATION_PROTOCOL_H
#define GAS_STATION_PROTOCOL_H

#define MAX_USER_INPUT 128

// Mensagems de requisicao de busca ou insercao de dados sobre postos
typedef struct {
    char messageType;    // Tipo de mensagem (Pesquisa (P), Dados (D))
    char errorFlag;      // Indicador de erro na mensagem
    double coordinates[2];  // Coordenadas
    int requestId;       // Identificador da mensagem de requisicao
    int fuelType;        // Tipo de combustivel: 0=diesel, 1=alcool, 2=gasolina
    int priceOrRadius;   // Preco (para operacoes de dados) ou raio (para operacoes de pesquisas)
} RequestMessage;

// Mensagems de Ack ou Nak
typedef struct {
    char messageType;    // Tipo da mensagem (ACK (A), NAK (N))
    int requestId;       // Identificador da mensagem de requisicao
} AckNakMessage;

// Mensagem de resposta incluindo os resultado de uma pesquisa
typedef struct {
    int requestId;       // Identificador da respectiva mensagem de requisicao
    int minPrice;        // Menor preco encontrado na pesquisa
    double coordinates[2];  // Coordenadas do melhor posto encontrado
} ResponseMessage;

typedef enum {
    LOG_INFO,
    LOG_SUCCESS,
    LOG_WARN,
    LOG_ERROR,
    LOG_LEVEL_LEN
} LOG_LEVEL;

void logMessage(LOG_LEVEL level, const char *fmt, ...);

char simulateError();

#endif
