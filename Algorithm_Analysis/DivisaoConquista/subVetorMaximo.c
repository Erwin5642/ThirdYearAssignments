#include <stdio.h>

int maxSubArray(int vector[], size_t n){
    int sum = vector[0], max = vector[0];
    for(size_t i = 1; i < n; i++){
        if(sum + vector[i] > sum){
            sum += vector[i];
        }
        else{
            sum = vector[i];
        }
        if(sum > max){
            max = sum;
        }
    }

    return max;
}

int main(){
    int vector[] = {6, -10, 2, 3, 2};

    printf("O resultado encontrado foi %d\n", maxSubArray(vector, 5));

    return 0;
}
