CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic

test_IP = 127.0.0.1
test_PORT = 8888
LOG = Estatisticas/pidstat_log.txt
PIDSTAT_OPTS = -u -d

CLIENT_SOURCES = cliente.c gasStationProtocol.c
SERVER_SOURCES = servidor.c KdTree.c list.c utils.c gasStationProtocol.c

CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)

all: cliente servidor test

cliente: $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

servidor: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

test: test.o gasStationProtocol.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o cliente servidor test

run:
	./test $(test_IP) $(test_PORT)

debug: servidor
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./servidor $(test_PORT)

stats:
	./run_pidstat.sh $(test_PORT) $(LOG) "$(PIDSTAT_OPTS)"

.PHONY: all clean run debug
