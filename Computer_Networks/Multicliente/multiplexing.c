//servidor usando multiplexação

#include<stdio.h>
#include<sys/ioctl.h> //funções de I/O de alto nível.
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	//Este programa usa a função select()
	//A função select() é uma função do sistema que permite especificar um conjunto de descritores
	struct sockaddr_in sAddr;
	fd_set readset, testset;
	int listensock;
	int novosocket;
	char buffer[25];
	int resposta;
	int nread;
	int x;
	int val;

	//ouvir chamadas de conexao dos clientes
	listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	val = 1;
	resposta = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); 
	//o linux costuma reservar a porta e o endereço durante seu uso. SO_REUSEADDR evita isso.
	//Ao recuperar uma opção do socket ou defini-la, você especifica o nome da opção, bem como o nível. 
	//Quando level = SOL_SOCKET, o item será procurado no próprio socket.
	//Por exemplo, suponha que queremos definir a opção de socket para reutilizar o endereço como 1 (on/true), passamos no "nível" SOL_SOCKET e o valor que queremos definir.

	if(resposta < 0)
	{
		perror("servidor");
		return 0;
	}

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(1972);
	sAddr.sin_addr.s_addr = INADDR_ANY;

	resposta = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
	if(resposta < 0)
	{
		perror("Servidor");
		return 0;
	}

	//ouvindo conexoes
	resposta = listen(listensock, 5);
	if(resposta < 0)
	{
		perror("servidor");
		return 0;
	}
		
	//manipulador de conexões
	FD_ZERO(&readset);
	FD_SET(listensock, &readset);

	//São disponíveis 4 macros para ajudar com o conjunto de descritores. 
	//FD_CLR reseta o conjunto de flags do descritor.
	//FD_ISSET determina quando um descritor está com a flag ativa ou não.
	//FD_SET ativa uma flag para observar um descritor.
	//FD_ZERO limpa (zera) o conjunto de descritores que estão sendo observados.

	while(1) {
		testset =  readset;
		resposta = select(FD_SETSIZE, &testset, NULL, NULL, NULL);
		if(resposta < 1)
		{
			perror("Servidor");
			return 0;
		}

		for(x = 0; x < FD_SETSIZE; x++)
		{
			if(FD_ISSET(x, &testset))
			{
				if(x == listensock)
				{
					novosocket = accept(listensock, NULL, NULL);
					FD_SET(novosocket, &readset);
				}
				else
				{
					nread = recv(x, buffer, 25, 0);
					if(nread <= 0)
					{
						close(x);
						FD_CLR(x, &readset);
						printf("Cliente do manipulador #%i desconectado\n", x);
					}
					else
					{
						buffer[nread] = '\0';
						printf("%s\n", buffer);
						send(x, buffer, nread, 0);
					}
				}
			}
		}
	}
}





