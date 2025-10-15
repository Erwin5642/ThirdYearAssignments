#include <arpa/inet.h> // Trata de todo ipv4
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORTA 8080
#define MESSAGE_LEN_MAX 1024

int main(int argc, char *argv[]){
    int cliente_fd, valor_lido, status;
    struct sockaddr_in server_addr;
    char msg[MESSAGE_LEN_MAX];
    char buffer[MESSAGE_LEN_MAX] = {0};

    // File descriptor é um inteiro que se refere a um arquivo
    if((cliente_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Falha no socket\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTA);

    //Converte os endereços de ipv4 e ipv6 de texto para formato binário
    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) < 0){
        fprintf(stderr, "Erro na conversao de endereços\n");
        exit(EXIT_FAILURE);
    }

    if((status = connect(cliente_fd, (struct sockaddr*)&server_addr, sizeof server_addr)) < 0){
        fprintf(stderr, "Erro ao conectar\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        fgets(msg, MESSAGE_LEN_MAX, stdin);
        if(strncmp(msg, "exit", 5)){
            close(cliente_fd);
            return 0;       
        }
        send(cliente_fd, msg, strlen(msg), 0);
        printf("Solicitacao enviada: %S\n", msg);
        valor_lido = read(cliente_fd, buffer, 1024 - 1);
        printf("Mensagem recebida: %s/n", buffer);    
    }
}
