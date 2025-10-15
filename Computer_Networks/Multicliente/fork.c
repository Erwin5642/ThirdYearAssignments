//Universidade Estadual de Mato Grosso do Sul
//Curso de Ciência da Computação
//Disciplina de Redes de Computadores

//Este servidor utiliza a função fork() para gerenciar múltiplos clientes.
//Ao se fazer a chamada de sistema fork(), cria-se uma duplicata exata do programa e, um novo processo filho é iniciado para essa cópia.

//Inclusão das bibliotecas
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
//A biblioteca <sys/wait.h> e a biblioteca <signal.h> são necessárias para evitar a criação de zombies.
//Zombies são processos filhos que aparecem quando processos pais deixam de existir sem ser feita a chamada wait() ou waitpid() nos processos filhos.
#include <sys/wait.h>
#include <signal.h>

void sigchld_handler(int signo)  //manipulador de sinais. Ele simplesmente faz a chamada waitpid para todo filho que for desconetado.
{
  while (waitpid(-1, NULL, WNOHANG) > 0);  //A ideia de se chamar em um laço é que não se tem certeza que há uma correlação um-para-um entre os filhos desconectados e as chamadas ao manipualador de sinais.
}  //valore lembrar que o posix não permite a criação de filas nas chamadas de sinal. Ou seja, pode acontecer de chamar o manipulador após vários filhos já terem sido desconectados.

//Declaração de variáveis tradicionais
int main(int argc, char *argv[])
{
    struct sockaddr_in sAddr;
    int meu_socket;
    int novo_socket;
    char buffer[25];
    int resultado;
    int leitor;
    int pid;
    int valor;
    
    meu_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //criação do socket que aceitará as chamadas de conexão

    valor = 1;
    //SO_REUSEADDR significa que as regras utilizadas para a validação de endereço feita pelo bind permite a reutilização de endereços locais.
    resultado = setsockopt(meu_socket, SOL_SOCKET, SO_REUSEADDR, &valor, sizeof(valor));
    if (resultado < 0) {
        perror("Erro");
        return 0;
    }
    
    //Uso do bind para associar a porta com todos endereços
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    resultado = bind(meu_socket, (struct sockaddr *) &sAddr, sizeof(sAddr));
    if (resultado < 0) {
        perror("Erro no bind");
        return 0;
    }

    
    //colocando o socket para ouvir a chegada de conexões
    resultado = listen(meu_socket, 5);
    if (resultado < 0) {
        perror("Erro no listen");
        return 0;
    }

    signal(SIGCHLD, sigchld_handler); //Ativando o manipulador de sinais antes de entrar no laço.

    while (1) {
        novo_socket = accept(meu_socket, NULL ,NULL);  //Antes da chamada ser aceita (retornada), chama-se o fork() para a criação de novos processos.
        if ((pid = fork()) == 0) {  //Se retornar zero é porque estamos no processo inicial, caso contrário retornamos o PID do novo processo filho.
            printf("Processo filho %i criado.\n", getpid());
            close(meu_socket);  //Uma vez com o processo filho, fecha-se o processo listen. Lembre-se que todos os são copiados do processo pai para o filho. 
            leitor = recv(novo_socket, buffer, 25, 0); //le alguma coisa do cliente e devolve para o mesmo.
            buffer[leitor] = '\0';
            printf("%s\n", buffer);
            send(novo_socket, buffer, leitor, 0);
            close(novo_socket);  //Esta linha só é alcançada no processo pai. Uma vez que o processo filho tem uma cópia do socket cliente. O processo pai a sua referência e decrementa o contador no kernel.
            printf("Processo filho %i terminado.\n", getpid());
            exit(0);
        }
        close(novo_socket);
    }
}
