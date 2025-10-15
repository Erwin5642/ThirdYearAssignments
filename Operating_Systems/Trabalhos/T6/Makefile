all: 
	gcc -g -o main main.c -Wall -Wextra -pedantic -lm


debug:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main 10 30 seq-20-paginas-10.txt

clear:
	rm main