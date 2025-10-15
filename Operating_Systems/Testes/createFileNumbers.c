#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    FILE *filetxt = fopen("random_numbers.txt", "w");  // Open file for writing
    
    if(argc != 2){
        printf("Numero invalido de argumentos\n");
        return 1;
    }

    if (filetxt == NULL) {
        perror("Failed to open file");
        return 1;
    }
    unsigned long size = atoi(argv[1]);
    // Seed the random number generator
    srand(time(NULL));

    int sum = 0;  // Initialize the sum variable

    // Generate and write 200 random numbers between 1 and 100 to the file
    for (int i = 0; i < size; i++) {
        int random_number = rand() % 100 + 1;  // Random number between 1 and 100
        fprintf(filetxt, "%d\n", random_number);
        sum += random_number;  // Add the random number to the sum
    }

    fclose(filetxt);

    // Print the total sum of all the numbers
    printf("Total sum of all random numbers: %d\n", sum);
    printf("File 'random_numbers.txt' created with %d random numbers.\n", size);

    return 0;
}
