#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum;                   /* esses dados são compartilhados pelo(s) thread(s) */
void *runner(void *param); /* os threads chamam essa função */

int main(int argc, char *argv[])
{
    pthread_t tid;       /* o identificador do thread */
    pthread_attr_t attr; /* conjunto de atributos do thread */
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
    /* obtém os atributos default */
    pthread_attr_init(&attr);
    /* cria o thread */
    pthread_create(&tid, &attr, runner, argv[1]);

    /* espera o thread ser encerrado */
    pthread_join(tid, NULL);
    printf("sum = % d\n", sum);
}

/* O thread assumirá o controle nessa função */
void *runner(void *param)
{
    int i, upper = atoi(param);
    sum = 0;
    for (i = 1; i <= upper; i++)
        sum += i;
    pthread_exit(0);
}