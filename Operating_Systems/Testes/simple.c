#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum;                   /* esses dados são compartilhados pelo(s) thread(s) */
void *runner(void *param); /* os threads chamam essa função */

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,"usage : a.out<integer value>\n");
        return -1;
    }
    if (atoi(argv[1]) < 0)
    {
        fprintf(stderr,"%d must be >= 0\n", atoi(argv[1]));
        return -1;
    }
    runner(argv[1]);
    printf("sum = % d\n", sum);
}

/* O thread assumirá o controle nessa função */
void *runner(void *param)
{
    int i, upper = atoi(param);
    sum = 0;
    for (i = 1; i <= upper; i++)
        sum += i;
    return NULL;
}