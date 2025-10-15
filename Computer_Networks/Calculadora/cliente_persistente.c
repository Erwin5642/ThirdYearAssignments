#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORTA 8080
#define MESSAGE_LEN_MAX 1024

int main() {
    int cliente_fd;
    struct sockaddr_in server_addr;
    char buffer[MESSAGE_LEN_MAX];
    ssize_t valor_lido;

    // 1) Cria o socket
    if ((cliente_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Falha ao criar socket");
        exit(EXIT_FAILURE);
    }

    // 2) Preenche estrutura do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORTA);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Endereço inválido");
        exit(EXIT_FAILURE);
    }

    // 3) Conecta
    if (connect(cliente_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Falha ao conectar");
        exit(EXIT_FAILURE);
    }

    // 4) Loop de envio/recebimento
    while (1) {
        printf("Digite uma mensagem (ou 'exit' para sair): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;  // EOF ou erro

        // remove '\n' do final
        buffer[strcspn(buffer, "\n")] = '\0';

        // detecta comando de saída
        if (strcmp(buffer, "exit") == 0) {
            printf("Fechando conexão...\n");
            break;
        }

        // envia para o servidor
        if (send(cliente_fd, buffer, strlen(buffer), 0) < 0) {
            perror("Erro ao enviar");
            break;
        }

        // recebe resposta
        memset(buffer, 0, sizeof(buffer));
        valor_lido = read(cliente_fd, buffer, sizeof(buffer) - 1);
        if (valor_lido < 0) {
            perror("Erro ao receber");
            break;
        } else if (valor_lido == 0) {
            printf("Servidor fechou a conexão\n");
            break;
        }

        printf("Recebeu %d bytes: %s\n", valor_lido, buffer);
    }

    close(cliente_fd);
    return 0;
}
