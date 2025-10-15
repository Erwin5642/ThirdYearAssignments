#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORTA 8080

int main(int argc, char *argv[]){
    int servidor_fd, novo_socket;
    struct sockaddr_in addres;
    ssize_t valor_lido;
    int opt = 1;
    socklen_t addrlen = sizeof addres;
    char buffer[1024] = {0};
    char *msg = "Quale rapa\n";

    // File descriptor é um inteiro que se refere a um arquivo
    if((servidor_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Falha no socket\n");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        fprintf(stderr, "Falha no setsockpot\n");
        exit(EXIT_FAILURE);
    }

    addres.sin_family = AF_INET;
    addres.sin_addr.s_addr = INADDR_ANY;
    addres.sin_port = htons(PORTA);

    // Anexando o socket a porta 
    if(bind(servidor_fd, (struct sockaddr*)&addres, addrlen)){
        fprintf(stderr, "Falha no bind\n");
        exit(EXIT_FAILURE);
    }

    if(listen(servidor_fd, 3) < 0){
        fprintf(stderr, "Erro no listen\n");
        exit(EXIT_FAILURE);
    }

    if((novo_socket = accept(servidor_fd, (struct sockaddr*)&addres, &addrlen)) < 0){
        fprintf(stderr, "Erro no accept\n");
        exit(EXIT_FAILURE);
    }
    valor_lido = read(novo_socket, buffer, 1024 - 1);
    printf("%s\n", buffer);
    send(novo_socket, msg, strlen(msg), 0);
    close(novo_socket);
    close(servidor_fd);

    return 0;
}

