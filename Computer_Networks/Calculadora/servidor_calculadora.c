#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>

#define PORTA 8080
#define MENSAGEM_LEN_MAX 1024

// Função que executa o avaliador e retorna a resposta via pipe
int avaliar_expressao(const char *input, char *resultado, size_t tamanho) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // Processo filho
        close(pipefd[0]); // fecha leitura
        dup2(pipefd[1], STDOUT_FILENO); // redireciona stdout
        close(pipefd[1]);

        execlp("python3", "python3",
               "-c",
               "import mymodule, sys; print(mymodule.evaluate(sys.argv[1]))",
               input,
               (char*)NULL);

        perror("execlp");
        _exit(1);
    }

    // Processo pai
    close(pipefd[1]); // fecha escrita
    ssize_t n = read(pipefd[0], resultado, tamanho - 1);
    if (n < 0) {
        perror("read");
        close(pipefd[0]);
        return -1;
    }

    resultado[n] = '\0'; // finaliza string
    close(pipefd[0]);
    int status;
    waitpid(pid, &status, 0);
    return 0;
}

int main() {
    int servidor_fd, novo_socket;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[MENSAGEM_LEN_MAX];
    char resultado[512];
    ssize_t valor_lido;
    int opt = 1;

    if ((servidor_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Falha ao criar socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORTA);
    if (bind(servidor_fd, (struct sockaddr *)&addr, addrlen) < 0) {
        perror("Falha no bind");
        exit(EXIT_FAILURE);
    }

    if (listen(servidor_fd, 3) < 0) {
        perror("Erro no listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Servidor aguardando conexões na porta %d...\n", PORTA);
        if ((novo_socket = accept(servidor_fd, (struct sockaddr *)&addr, &addrlen)) < 0) {
            perror("Erro no accept");
            exit(EXIT_FAILURE);
        }
        printf("Cliente conectado!\n");

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            valor_lido = read(novo_socket, buffer, sizeof(buffer) - 1);
            if (valor_lido < 0) {
                perror("Erro ao receber");
                break;
            } else if (valor_lido == 0) {
                printf("Cliente desconectou.\n");
                close(novo_socket);
                break;
            }

            buffer[strcspn(buffer, "\r\n")] = '\0';
            printf("Recebido %d bytes: %s\n", valor_lido, buffer);

            if (avaliar_expressao(buffer, resultado, sizeof(resultado)) == 0) {
                if (send(novo_socket, resultado, strlen(resultado), 0) < 0) {
                    perror("Erro ao enviar resposta");
                    break;
                }
                printf("Resposta enviada: %s", resultado);
            } else {
                char *erro = "Erro ao avaliar expressão.\n";
                send(novo_socket, erro, strlen(erro), 0);
            }
        }
    }

    close(servidor_fd);
    return 0;
}
