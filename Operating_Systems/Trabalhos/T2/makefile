THREADS_FILE = T2_Threads.c
PROCESS_FILE = T2_Process.c
THREADS_EXEC = T2_Threads
PROCESS_EXEC = T2_Process

all: $(THREADS_EXEC) $(PROCESS_EXEC)

$(THREADS_EXEC): $(THREADS_FILE)
	gcc -g -o $(THREADS_EXEC) $(THREADS_FILE) -Wall -Wextra -pedantic -lm -pthread

$(PROCESS_EXEC): $(PROCESS_FILE)
	gcc -g -o $(PROCESS_EXEC) $(PROCESS_FILE) -Wall -Wextra -pedantic -lm -lrt

clean:
	rm -f $(THREADS_EXEC) $(PROCESS_EXEC)
