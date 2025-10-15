#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv){
    struct addrinfo hints, *result, *rp;

    if(argc != 2){
        fprintf(stderr, "Uso: %s host", argv[0]);
        return -1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DA; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;  

    getaddrinfo();
    

}
