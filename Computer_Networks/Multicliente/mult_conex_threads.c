//Rodar este programa com mais 3 terminal, como foi feito no telnet
//Compilar este programa com a diretiva posix, isto é, -lpthread
// gcc programa.c -o programa -lpthread

#include<stdio.h>
#include<string.h> //strlen
#include<stdlib.h> //strlen
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>  //para usar o write

#include<pthread.h> //para usar as threads. linkar usando o cabeçalho acima

void *manipulador_conexao(void *);

int main(int argc, char** argv)
{
	int meu_socket, novo_socket, c, *novo_sock;
	struct sockaddr_in servidor, cliente;
	char *mensagem;

	//Criar um socket
	meu_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(meu_socket == -1)
	{
		printf("Erro ao Criar o socket!");
	}

	//preparando a estrutura sockaddr_in
	servidor.sin_family = AF_INET;
	servidor.sin_addr.s_addr = INADDR_ANY;
	servidor.sin_port = htons(8888);

	//bind
	if( bind(meu_socket, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
	{
		puts("Falha no bind!");
		return 1;
	}

	puts("Bind executado corretamente!\n");

	//listen
	listen(meu_socket, 3);
	
	//Aguardando por conexões
	puts("Aguardando por chegada de conexoes....\n");
	c = sizeof(struct sockaddr_in);
	
	while( (novo_socket = accept( meu_socket, (struct sockaddr *)&cliente, (socklen_t*)&c)) )
	{
		puts("Conexao Aceita!\n");
		
		//Respondendo ao cliente
		mensagem = "Ae mano, Acabei de receber a parada da conexao. E agora e nois na fita, ficaremos conectados\n";
		write(novo_socket, mensagem, strlen(mensagem));

		pthread_t sniffer_thread;
		novo_sock = malloc(1);
		*novo_sock = novo_socket;

		if( pthread_create( &sniffer_thread, NULL, manipulador_conexao, (void*) novo_sock) < 0)
		{
			perror("Nao foi possivel criar a thread!!");
			return 1;
		}

		//Nessa parte a thread é colocada no conjunto de conexoes ativas.
		//A conexao não é desfeita antes da thread.
		//pthread_join(sniffer_thread, NULL)
		puts("Identificador de conexao atribuido!\n");
	}

	if(novo_socket < 0)
	{
		perror("Falha na conexao do novo socket");
		return 1;
	}
} //fecha o int main.

/*
Esta parte do código manterá a conexão ativa para cada cliente.
*/
	
void *manipulador_conexao(void *meu_socket)
{
	//Obter o descritor (identificador) do socket
	int sock = *(int*)meu_socket;
	int tamanho_dado_lido;

	char *mensagem, mensagem_cliente[2000];

	//enviar mensagem para o cliente
	mensagem = "Ai brou, e da hora esse nosso manipulador de conexoes\n";
	write(sock, mensagem, strlen(mensagem));

	mensagem = "Como sou um servidor, vamos nos comunicar. Digita ai mano!\n";
	write(sock, mensagem, strlen(mensagem));

	//receber mensagem do cliente
	while((tamanho_dado_lido = recv(sock, mensagem_cliente, 2000, 0)) > 0)
	{
		//envia a mesma mensagem de volta para o cliente.
		write(sock, mensagem_cliente, strlen(mensagem_cliente));
	}

	if(tamanho_dado_lido == 0)
	{
		puts("Cliente Desconectado!");
		fflush(stdout);
	}
	else if(tamanho_dado_lido == -1)
	{
		perror("Funcao receive falhou!");
	}

	//liberar o ponteiro, memória
	free(meu_socket);

	return 0;
}	