Escreva um programa em MIPS (p/ MARS), sem a utilização de pseudo instruções, que utilize como interface a ferramenta Digital Lab Sim (tools → Digital Lab Sim).

Considere que as entradas sempre são dadas números decimais (0 à 99), com as teclas de 0 à 9, e as funções pelas teclas de a à f.

As saídas são dadas pelo display de 7 segmentos (saídas sempre serão decimais de até dois digitos e, caso tenha um ponto decimal, terá um digito a direita e um à esquerda do ponto). 

Sempre considere que se o número não “cabe” no display, então a mensagem EE deve ser mostrada (no display).

Seu programa deve se manter executando (recebendo entrada e gerando saída) até a tecla designada para função de fechamento seja acionada (assim como em uma calculadora comum).

As funções das teclas de a à f são:

a) Acumulador de pilha – quando um número é digitado e a tecla a é precionada, o número é empilhado (considere uma pilha de no máximo 10 elementos). Obs: esta pilha é apenas um array, não é a pilha do programa.

b) Média aritmética dos elementos armazenados na pilha - perceba que esta média pode ser um número não inteiro, nesse caso, mostre um arredondamento do número com um digito a esquerda e outro a direita do ponto, no display.

c) Desvio padrão dos elementos armazenados na pilha - perceba que esta média pode ser um número não inteiro, nesse caso, mostre um arredondamento do número com um digito a esquerda e outro a direita do ponto, no display.

d) Sequência de Van Eck – Dado um inteiro n não negativo, no display, precionar a tecla c resulta no calculo do n-ésimo termo da sequência de Van Eck (deve ser calculado recursivamente).

e) Sequência de Fibbonacci – Dado um inteiro n não negativo, no display, precionar a tecla c resulta no calculo do n-ésimo termo da sequência de Fibbonacci (deve ser calculado recursivamente).

f) Clear – desempilha todos os elementos da pilha, zera o display, etc.