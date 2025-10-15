#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int is_port_open(char *ip, int port){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        fprintf(stderr, "Erro ao criar socket\n");
        return 0;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    int conected = connect(sock, (struct sockaddr *)&addr, sizeof addr);
    close(sock);
    return conected == 0;
}

int main(int argc, char **argv){
    if(argc != 4){
        fprintf(stderr, "Erro: Uso: <IP> <inicio_intervalo> <final_intervalo>\n");
        return -1;
    }
    int port_begin = atoi(argv[2]), port_end = atoi(argv[3]);;

    for(int port = port_begin; port <= port_end; port++){
        if(is_port_open(argv[1], port)){
            printf("Porta %d aberta\n", port);
        }
        else{
            printf("Porta %d fechada\n", port);
        }
    }

    return 0;
}
