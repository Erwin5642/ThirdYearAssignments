.data
# Codigos para digitos no display de 7 segmentos
zero: .byte 63
um: .byte 6
dois: .byte 91
tres: .byte 79
quatro: .byte 102
cinco: .byte 109
seis: .byte 125
sete: .byte 7
oito: .byte 127
nove: .byte 111
# Codigo para erro no display de 7 segmentos
error: .byte 121
# Espaço para armazenar qual tecla foi pressionada
keyPressed: .byte 0

.text
# ===	=====	=====	=====	=====	#
# Definição do registro da pilha:	#
# 0 - ponteiro para topo da pilha	#
# 4 - contador de elementos da pilha	#
# ===	=====	=====	=====	=====	#

MAIN:
# ===	=====	=====	=====	=====	=====	#
# $s0 - ponteiro para pilha (p)			#
# $s1 - ponteiro para a tecla pressionada (key)	#
# $s2 - acumulador (acc)			#
# $s3 - digito (d)				#
# $s4 - numero do display (dsp)			#
# $s5 - flag para float (isf)
# ===	=====	=====	=====	=====	=====	#

	#Aloca espaço para a pilha de numeros
	addi $v0, $zero, 9    
	addi $a0, $zero, 11
	syscall		

	# Salvar registro da pilha na pilha
	addi $sp, $sp, -8	
	add $s0, $zero, $sp	# p = new struct pilha	
			
	add $t1, $zero, $zero	
	
	sw $v0, 0($sp)		# p.data = char[10]
	sw $t1, 4($sp)		# p.n = 0
	
	lui $s1, 0x1001		
	ori $s1, $s1, 0x000b	# key = char[1]		
	add $s2, $zero, $zero	# acc = 0
	
	# Inicializa as interrupções do teclado hexadecimal
	lui $t0, 0xFFFF     		
	ori $t0, $t0, 0x0012
	addi $t1, $zero, 0x80
	sb $t1, 0($t0)

	# Loop principal do programa
	MAIN_LOOP:
		# Verifica se alguma tecla foi pressionada
		lbu $t0, 0($s1)
		beq $t0, $zero, MAIN_LOOP 	# if key[0] != 0
		
		# Converte a tecla para o valor que ela representa
		add $a0, $zero, $t0	 
		jal CONVERT_KEY_TO_NUMBER
		add $s3, $zero, $v0		# d = convert_key(key[0])
		
		SWITCH_CASE:
			addi $t0, $zero, 10
			beq $s3, $t0, CASE_A
			addi $t0, $zero, 11
			beq $s3, $t0, CASE_B
			addi $t0, $zero, 12
			beq $s3, $t0, CASE_C
			addi $t0, $zero, 13
			beq $s3, $t0, CASE_D	
			addi $t0, $zero, 14
			beq $s3, $t0, CASE_E
			addi $t0, $zero, 15
			beq $s3, $t0, CASE_F
		
			DEFAULT:
				# Se for um digito, concatena ele ao valor decimal
				addi $t0, $zero, 10
				mul $s2, $s2, $t0	# acc = acc * 10
				add $s2, $s2, $s3	# acc = acc + d
				addi $t0, $zero, 100	
				div $s2, $t0
				mfhi $s2		# acc = acc % 100
				add $s4, $zero, $s2	# dsp = acc
				add $s5, $zero, $zero	# isf = 0
		
				j END_SWITCH_CASE
			CASE_A:
				add $a0, $zero, $s0
				add $a1, $zero, $s2	
				jal PUSH_STACK		# push_stack(p, acc)
		
				add $s2, $zero, $zero	# acc = 0
				add $s4, $zero, $s2	# dsp = 0
				add $s5, $zero, $zero	# isf = 0
				
				j END_SWITCH_CASE
			CASE_B:
				add $a0, $zero, $s0
				jal ARITHMETIC_MEAN	
		
				add $s2, $zero, $zero	# acc = 0
				mfc1 $s4, $f0	# dsp = arithmetic_mean(p)	
				addi $s5, $zero, 1	# isf = 1
	
				j END_SWITCH_CASE
			CASE_C:
				add $a0, $zero, $s0
				jal STANDART_DEVIATION
		
				add $s2, $zero, $zero	# acc = 0
				mfc1 $s4, $f0	# dsp = standart_deviation(p)	
				addi $s5, $zero, 1	# isf = 1
	
				j END_SWITCH_CASE	
			CASE_D:
				add $a0, $zero, $s2
				jal VAN_ECK		
		
				add $s2, $zero, $zero	# acc = 0
				add $s4, $zero, $v0	# dsp = van_eck(acc)	
				add $s5, $zero, $zero	# isf = 0
			
				j END_SWITCH_CASE
			CASE_E:
				add $a0, $zero, $s2
				jal FIBONACCI
		
				add $s2, $zero, $zero	# acc = 0
				add $s4, $zero, $v0	# dsp = fibonacci(acc)	
				add $s5, $zero, $zero	# isf = 0
			
				j END_SWITCH_CASE
			CASE_F:
				add $a0, $zero, $s0
				jal CLEAR_STACK
	
				add $s2, $zero, $zero	# acc = 0
				add $s4, $zero, $zero	# dsp = 0
				add $s5, $zero, $zero	# isf = 0	
		END_SWITCH_CASE:
		
		# Mostra o número no display
		add $a0, $zero, $s4	
		add $a1, $zero, $s5
		jal DISPLAY_NUMBER
		# Reseta tecla pressionada
		sb $zero, 0($s1)
	
		j MAIN_LOOP
	END_MAIN_LOOP:
	
	# Encerra programa
	addi $v0, $zero, 10
	syscall

# ===	=====	=====	=====	=====	=====	=====	=====	#
# Função CONVERT_KEY_TO_NUMBER					#
# Descrição: converte tecla para valor que ela representa	#
# Input: $a0 - tecla para ser convertida			#
# Output: $v0 - valor da tecla					#
# ===	=====	=====	=====	=====	=====	=====	=====	#	
CONVERT_KEY_TO_NUMBER:					
	# $s0 - linha 
	# $s1 - coluna

	# CONVERT_KEY_TO_NUMBER_PROLOGUE
	addi $sp, $sp, -16
	sw $ra, 12($sp)
	sw $fp, 8($sp)
	sw $s0, 4($sp)
	sw $s1, 0($sp)
	add $fp, $zero, $sp
	
	# CONVERT_KEY_TO_NUMBER_BODY
	# Convertemos as coordenadas da tecla para valor usando: x = 4 * log_2(linha) + log_2(coluna)

	# Separamos as coordenadas: linha (bits menos significativos) e coluna (bits mais significativos)
	andi $s0, $a0, 0x0F	# linha  
	srl $s1, $a0, 4		# coluna 

	# log_2(x) = 31 - numero de zeros a esquerda; se x = 2^n
	addi $t0, $zero, 31

	# log(linha)
	clz $t1, $s0
	sub $t1, $t0, $t1

	# log(coluna)
	clz $t2, $s1
	sub $t2, $t0, $t2

	# 4 * log(linha)
	sll $t1, $t1, 2
	# 4 * log(linha) + log(coluna)

	add $v0, $t1, $t2	# return 4 * log(linha) + log(coluna)

	# CONVERT_KEY_TO_NUMBER_EPILOGUE	
	add $sp, $zero, $fp
	lw $ra, 12($sp)
	lw $fp, 8($sp)
	lw $s0, 4($sp)
	lw $s1, 0($sp)
	addi $sp, $sp, 16

	jr $ra
 
# ===	=====	=====	=====	=====	#
# Função DISPLAY_NUMBER			#
# Descrição: mostra número no display	#
# Input: $a0 - numero a ser mostrado	#
# Input: $a1 - flag para float		#
# Output: mostra numero no display	#
# ===	=====	=====	=====	=====	#
DISPLAY_NUMBER:
	# $s0 - define se é float (isf)
	# $s1 - indice do digito da esquerda (esq)
	# $s2 - indice do digito da direita (dir)
	
	# DISPLAY_NUMBER_PROLOGUE
	# Salva o endereço de retorno do procedimento
	addi $sp, $sp, -20
	sw $ra, 16($sp)
	sw $fp, 12($sp)
	sw $s0, 8($sp)
	sw $s1, 4($sp)
	sw $s2, 0($sp)
	add $fp, $zero, $sp

	add $s0, $zero, $zero
	DISPLAY_NUMBER_IF_FLOAT:
		beq $a1, $zero, DISPLAY_NUMBER_END_IF_FLOAT
		mtc1 $a0, $f4
		addi $t0, $zero, 10
		mtc1 $t0, $f6
		cvt.s.w $f6, $f6
		mul.s $f4, $f4, $f6
		cvt.w.s $f4, $f4
		mfc1 $a0, $f4
		addi $s0, $zero, 1
	DISPLAY_NUMBER_END_IF_FLOAT:

	# Verifica se o número é válido
	addi $t0, $zero, 99
	sub $t0, $t0, $a0
	bgez $t0, ELSE_ERROR	# if n < 99
	# Se não for válido, exibe um erro
	IF_ERROR:
		addi $s1, $zero, 0x0a
		addi $s2, $zero, 0x0a
		
		j END_ERROR 
	# Se for válido, exibe os digitos esquerdo e direitos
	ELSE_ERROR:
		addi $t0, $zero, 10
		div $a0, $t0
		mflo $s1
		mfhi $s2
	END_ERROR:
	# Exibe o digito esquerdo e direito
	add $a0, $zero, $s1
	add $a1, $zero, $s0
	jal DISPLAY_LEFT_DIGIT
	add $a0, $zero, $s2
	jal DISPLAY_RIGHT_DIGIT
	
	# Epilogo
	# Recarrega o endereço de retorno do procedimento
	add $sp, $zero, $fp
	lw $ra, 16($sp)
	lw $fp, 12($sp)
	lw $s0, 8($sp)
	lw $s1, 4($sp)
	lw $s2, 0($sp)
	addi $sp, $sp, 20	
	
	jr $ra

# ===	=====	=====	=====	=====	=====	#
# Função DISPLAY_RIGHT_DIGIT			#
# Descrição: mostra digito no display direito	#
# Input: $a0 - digito a ser mostrado		#
# Output: mostra digito no display direito	#
# ===	=====	=====	=====	=====	=====	#
DISPLAY_RIGHT_DIGIT:	
	addi $sp, $sp, -16
	sw $ra, 12($sp)
	sw $fp, 8($sp)
	sw $s0, 4($sp)
	sw $s1, 0($sp)
	add $fp, $zero, $sp
	
	# Ponteiro para do display direito
	lui $s0, 0xFFFF
	ori $s0, $s0, 0x0010
	# Ponteiro para do código do digito
	lui $s1, 0x1001
	or $s1, $s1, $a0
	# Salva o código no display direito
	lb $s1, 0($s1)
	sb $s1, 0($s0)

	add $sp, $zero, $fp
	lw $ra, 12($sp)
	lw $fp, 8($sp)
	lw $s0, 4($sp)
	lw $s1, 0($sp)
	addi $sp, $sp, 16

	jr $ra

# ===	=====	=====	=====	=====	=====	#
# Função DISPLAY_LEFT_DIGIT			#
# Descrição: mostra digito no display esquerdo	#
# Input: $a0 - digito a ser mostrado		#
# Input: $a1 - flag para float			#
# Output: mostra digito no display esquerdo	#
# ===	=====	=====	=====	=====	=====	#	
DISPLAY_LEFT_DIGIT:
	addi $sp, $sp, -16
	sw $ra, 12($sp)
	sw $fp, 8($sp)
	sw $s0, 4($sp)
	sw $s1, 0($sp)
	add $fp, $zero, $sp
	
	# Ponteiro para do display esquerdo
	lui $s0, 0xFFFF
	ori $s0, $s0, 0x0011
	# Ponteiro para do código do digito
	lui $s1, 0x1001
	or $s1, $s1, $a0
	# Salva o código no display esquerdo
	lb $s1, 0($s1)
	sll $t0, $a1, 7
	add $s1, $s1, $t0
	sb $s1, 0($s0)
	
	add $sp, $zero, $fp
	lw $ra, 12($sp)
	lw $fp, 8($sp)
	lw $s0, 4($sp)
	lw $s1, 0($sp)
	addi $sp, $sp, 16
	
	jr $ra

# ===	=====	=====	=====	=====	#
# Função PUSH_STACK			#
# Descrição: salva um número na pilha	#
# Input: $a0 - ponteiro para pilha	#
# Input: $a1 - numero a ser inserido	#
# Output: numero inserido na pilha	#
# ===	=====	=====	=====	=====	#	
PUSH_STACK:
	# ===	=====	=====	=====	=====	=====	#
	# $s0 - ponteiro para o topo da pilha		#
	# $s1 - numero de elementos na pilha		#
	# $s2 - maximo da piha				#
	# ===	=====	=====	=====	=====	=====	#
	addi $sp, $sp, -16
	sw $ra, 12($sp)
	sw $fp, 8($sp)
	sw $s0, 4($sp)
	sw $s1, 0($sp)
	add $fp, $zero, $sp

	lw $s0, 0($a0)
	lw $s1, 4($a0)
	addi $s2, $zero, 10

	# Se a pilha não estiver cheia insere o elemento e atualiza a pilha
	beq $s1, $s2, END_IF_STACK_NOT_FULL
	IF_STACK_NOT_FULL:
		# Salva o elemento no topo da pilha
		add $t0, $s0, $s1
		sb $a1, 0($t0) 

		# Incrementa número de elementos
		addi $s1, $s1, 1

		# Atualiza a pilha com o novo tamanho
		sw $s1, 4($a0)
	END_IF_STACK_NOT_FULL:
	
	add $sp, $zero, $fp
	lw $ra, 12($sp)
	lw $fp, 8($sp)
	lw $s0, 4($sp)
	lw $s1, 0($sp)
	addi $sp, $sp, 16
	
	jr $ra
	
# ===	=====	=====	=====	=====	#
# Função: CLEAR_STACK			#
# Descrição: zera a pilha		#
# Input: $a0 - ponteiro para pilha	#
# Output: pilha é esvaziada		#
# ===	=====	=====	=====	=====	#	
CLEAR_STACK:
	sw $zero, 4($a0)       # p.n = 0
	jr $ra

	
# ===	=====	=====	=====	=====	=====	=====	=====	#
# Função: VAN_ECK						#
# Descrição: calcula o enesimo número de sequencia de van eck	#
# Input: $a0 - n						#
# Output: VAN_ECK(n)						#
# ===	=====	=====	=====	=====	=====	=====	=====	#
VAN_ECK:
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	
	# last_seen = [-1] * (n + 1)
	addi $t0, $a0, 1
	sll $t0, $t0, 2
	sub $sp, $sp, $t0
	
	addi $t0, $zero, -1
	add $t1, $zero, $sp
	addi $t2, $zero, -1
	VAN_ECK_INIT:
	beq $t0, $a0, VAN_ECK_INIT_END
	
	sw $t2, 0($t1)
	
	addi $t0, $t0, 1
	addi $t1, $t1, 4
	
	j VAN_ECK_INIT
	VAN_ECK_INIT_END:
	
	add $a1, $zero, $sp
	jal VAN_ECK_REC

	addi $t0, $a0, 1
	sll $t0, $t0, 2
	add $sp, $sp, $t0
			
	lw $ra, 0($sp)
	addi $sp, $sp, 4	
	
	jr $ra

# ===	=====	=====	=====	=====	=====	=====	=====	#
# Função: VAN_ECK_REC						#
# Descrição: auxilia no calculo do enesimo número de van eck	#
# Input: $a0 - n						#
# Input: $a1 - last_seen					#
# Output: VAN_ECK(n)						#
# ===	=====	=====	=====	=====	=====	=====	=====	#		
VAN_ECK_REC:
	addi $sp, $sp, -8
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	
	# if n == 0: last_seen[0] = 0; return 0
	VAN_ECK_REC_BASE_CASE:
		bne $a0, $zero, VAN_ECK_REC_INDUCTIVE_CASE	

		add $v0, $zero, $zero
	
		j VAN_ECK_REC_EPILOGUE
	VAN_ECK_REC_INDUCTIVE_CASE:
		# prev = van_eck_rec(n - 1, last_seen)
		addi $a0, $a0, -1
		jal VAN_ECK_REC

		# val = 0		
		add $t0, $zero, $zero
		
		sll $t3, $v0, 2
		add $t1, $a1, $t3
		lw $t2, 0($t1)
		bltz $t2, VAN_ECK_REC_IF_LAST_SEEN_END
		# if last_seen[prev] != -1
		VAN_ECK_REC_IF_LAST_SEEN:
			# val = (n - 1) - last_seen[prev]
			sub $t0, $a0, $t2
		VAN_ECK_REC_IF_LAST_SEEN_END:
		# last_seen[prev] = n - 1
		sw $a0, 0($t1)
		
		#return val
		add $v0, $zero, $t0
	VAN_ECK_REC_EPILOGUE:
	
	lw $a0, 4($sp)
	lw $ra, 0($sp)
	addi $sp, $sp, 8
			
	jr $ra
	
# ===	=====	=====	=====	=====	=====	=====	#
# Função: FIBONACCI					#
# Descrição: calcula o enesimo número de fibonacci	#
# Input: $a0 - n					#
# Output: Fibonacci(n)					#
# ===	=====	=====	=====	=====	=====	=====	#		
FIBONACCI:
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	
	# dp = [0] * (n + 1)
	addi $t0, $a0, 1
	sll $t0, $t0, 2
	sub $sp, $sp, $t0
	
	add $t1, $zero, $zero
	add $t2, $zero, $sp
	addi $t3, $zero, -1
	FIBONACCI_INIT_DP:
	beq $t1, $t0, FIBONACCI_INIT_DP_END
	sw $t3, 0($t2)
	
	addi $t1, $t1, 4
	addi $t2, $t2, 4
	j FIBONACCI_INIT_DP
	FIBONACCI_INIT_DP_END:
	
	# fib(n, dp)
	add $a1, $zero, $sp
	jal FIBONACCI_REC
	
	# desempilha dp
	addi $t0, $a0, 1
	sll $t0, $t0, 2
	add $sp, $sp, $t0	
						
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	
	jr $ra

# ===	=====	=====	=====	=====	=====	=====	=====	#
# Função: FIBONACCI_REC						#
# Descrição: auxilia no calculo do enesimo número de fibonacci	#
# Input: $a0 - n						#
# Input: $a1 - dp						#
# Output: Fibonacci(n)						#
# ===	=====	=====	=====	=====	=====	=====	=====	#		
FIBONACCI_REC:
	addi $sp, $sp, -12
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $s0, 8($sp)
	
	FIBONACCI_REC_BASE_CASE:
	# if a0 == 0 || a0 == 1 return a0
	slti $t0, $a0, 2
	beq $t0, $zero, FIBONACCI_REC_DP
		
	sll $t0, $a0, 2
	add $t0, $t0, $a1
	sw $a0, 0($t0)
	add $v0, $zero, $a0
	j FIBONACCI_REC_EPILOGUE
	
	FIBONACCI_REC_DP:
	# if dp[n] != -1 return dp[n]
	sll $t0, $a0, 2
	add $t0, $t0, $a1
	lw $t1, 0($t0)
	addi $t2, $zero, -1
	beq $t1, $t2, FIBONACCI_REC_INDUCTIVE_STEP
		
	add $v0, $zero, $t1
	j FIBONACCI_REC_EPILOGUE
	
	FIBONACCI_REC_INDUCTIVE_STEP:
	# return fib(n- 1) + fib(n - 2)
	
	addi $a0, $a0, -1	
	jal FIBONACCI_REC
	add $s0, $zero, $v0
	
	addi $a0, $a0, -1
	jal FIBONACCI_REC
	
	add $s0, $v0, $s0
	
	addi $a0, $a0, 2
	sll $t0, $a0, 2
	add $t0, $t0, $a1
	sw $s0, 0($t0)
	
	add $v0, $zero, $s0
	
	FIBONACCI_REC_EPILOGUE:
	lw $s0, 8($sp)
	lw $a0, 4($sp)	
	lw $ra, 0($sp)
	addi $sp, $sp, 12
	
	jr $ra

# ===	=====	=====	=====	=====	=====	=====	=====	#
# Função: ARITHMETIC_MEAN					#
# Descrição: calcula a média dos valores na pilha da calculadora #
# Input:  $a0 - ponteiro para o registro de pilha		#
# Output: média aritmética (float em $f0)			#
# ===	=====	=====	=====	=====	=====	=====	=====	#			
ARITHMETIC_MEAN:
	# PROLOGUE
	addi $sp, $sp, -20
	sw $ra, 16($sp)
	sw $s0, 12($sp)
	sw $s1, 8($sp)
	sw $s2, 4($sp)
	sw $s3, 0($sp)

	# BODY
	lw $s0, 0($a0)		# s0 = p.data
	lw $s1, 4($a0)		# s1 = p.n

	# Se a pilha estiver vazia, retorna 0.0
	beq $s1, $zero, ARITHMETIC_MEAN_EMPTY_STACK

	# soma = 0
	add $s3, $zero, $zero
	add $s2, $zero, $zero	# i = 0

	ARITHMETIC_MEAN_LOOP:
		beq $s2, $s1, ARITHMETIC_MEAN_LOOP_END

		add $t0, $s0, $s2	# endereço do elemento
		lbu $t0, 0($t0)		# x = p.data[i]

		add $s3, $s3, $t0	# soma += x
		addi $s2, $s2, 1
		j ARITHMETIC_MEAN_LOOP

	ARITHMETIC_MEAN_LOOP_END:
		# converter soma e n para float e dividir
		mtc1 $s3, $f4
		cvt.s.w $f4, $f4
		mtc1 $s1, $f6
		cvt.s.w $f6, $f6
		div.s $f0, $f4, $f6	# f0 = soma / n

		j ARITHMETIC_MEAN_END

	ARITHMETIC_MEAN_EMPTY_STACK:
		mtc1 $zero, $f0
		cvt.s.w $f0, $f0	# f0 = 0.0

	ARITHMETIC_MEAN_END:
	# EPILOGUE
	lw $ra, 16($sp)
	lw $s0, 12($sp)
	lw $s1, 8($sp)
	lw $s2, 4($sp)
	lw $s3, 0($sp)
	addi $sp, $sp, 20
	jr $ra

# ===	=====	=====	=====	=====	=====	=====	=====	#
# Função: STANDART_DEVIATION					#
# Descrição: calcula o desvio padrão dos números na pilha	#
# Input:  $a0 - ponteiro para o registro de pilha		#
# Output: desvio padrão (float em $f0)				#
# ===	=====	=====	=====	=====	=====	=====	=====	#
STANDART_DEVIATION:
	# v		- $s0 (p.data)
	# i 		- $s1 (índice)
	# n 		- $s2 (tamanho)
	# soma²		- $s3 (acumulador em float)
	# media		- $s4 (float)
	# variancia	- $s5 (float)
	
	# --- PROLOGUE ---
	addi $sp, $sp, -40
	sw $ra, 36($sp)
	sw $fp, 32($sp)
	sw $s0, 28($sp)
	sw $s1, 24($sp)
	sw $s2, 20($sp)
	sw $s3, 16($sp)
	sw $s4, 12($sp)
	sw $s5, 8($sp)
	add $fp, $zero, $sp
	
	# --- BODY ---
	lw $s0, 0($a0)		# s0 = p.data
	lw $s2, 4($a0)		# s2 = p.n
	add $s1, $zero, $zero	# i = 0
	
	# se pilha vazia, retorna 0.0
	beq $s2, $zero, STANDART_DEVIATION_EMPTY
	
	# calcula a média
	jal ARITHMETIC_MEAN
	mov.s $f2, $f0		# f2 = média
	
	# inicializa variância parcial = 0.0
	mtc1 $zero, $f6
	cvt.s.w $f6, $f6	# f6 = 0.0 (acumulador)
	
STANDART_DEVIATION_SUM:
	beq $s1, $s2, STANDART_DEVIATION_SUM_END
	
	add $t1, $s0, $s1	# endereço do elemento (byte array)
	lb $t2, 0($t1)		# t2 = x[i] (signed byte)
	
	mtc1 $t2, $f8
	cvt.s.w $f8, $f8	# f8 = float(x[i])
	
	sub.s $f10, $f8, $f2	# (x[i] - média)
	mul.s $f10, $f10, $f10	# (x[i] - média)^2
	
	add.s $f6, $f6, $f10	# soma acumulada += diferença^2
	
	addi $s1, $s1, 1
	j STANDART_DEVIATION_SUM

STANDART_DEVIATION_SUM_END:
	# variância = soma / n
	mtc1 $s2, $f12
	cvt.s.w $f12, $f12
	div.s $f6, $f6, $f12
	
	# desvio padrão = sqrt(variância)
	sqrt.s $f0, $f6
	j STANDART_DEVIATION_EPILOGUE

STANDART_DEVIATION_EMPTY:
	mtc1 $zero, $f0
	cvt.s.w $f0, $f0

STANDART_DEVIATION_EPILOGUE:
	add $sp, $zero, $fp
	lw $fp, 32($sp)
	lw $s0, 28($sp)
	lw $s1, 24($sp)
	lw $s2, 20($sp)
	lw $s3, 16($sp)
	lw $s4, 12($sp)
	lw $s5, 8($sp)
	lw $ra, 36($sp)
	addi $sp, $sp, 40
	jr $ra

# ===	=====	=====	=====	=====	=====	=====	#
# Função: HANDLER					#
# Descrição: lida com as interrupções do programa	#
# Input: interrupção					#
# Output: resoluçao da interrupção			#
# ===	=====	=====	=====	=====	=====	=====	#	
.ktext 0x80000180
HANDLER:		
	# Prologo
	addi $sp, $sp, -16
	sw $s0, 0($sp)
	sw $s1, 4($sp)
	sw $s2, 8($sp)
	sw $s3, 12($sp)
	
	mfc0 $s0, $13
	addi $s1, $zero, 0x800	# Se for interrupção de teclado, redireciona para a rotina de resolção de interrupção de teclado
	and $s0, $s0, $s1
	beq $s0, $zero, END_HANDLER

	KEYBOARD_INTERRUPTION_HANDLER:
		# Ponteiro para tecla pressionada do teclado hexadecimal
		lui $s0, 0xFFFF
		ori $s0, $s0, 0x0014
		# Ponteiro para o controle do teclado hexadecimal
		lui $s1, 0xFFFF     		
		ori $s1, $s1, 0x0012	
		# Linha para scan
		addi $s3, $zero, 8
		
		SCAN_LOOP: 
			# Checa se ainda existem linhas para checar
			blez $s3, END_SCAN_LOOP

			# Solicita pela linha atual sem modificar o bit de interrupção
			addi $s2, $s3, 0x80
			sb $s3, 0($s1)

			# Checa se existe alguma tecla pressionada nessa linha
			lbu $s2, 0($s0)

			beq $s2, $zero, END_KEY_FOUND
			IF_KEY_FOUND:
				# Ponteiro para o tecla pressionada global
				lui $s0, 0x1001
				ori $s0, $s0, 0x000b
				# Salva a tecla pressionada na região global com a tecla pressionada	
				sb $s2, 0($s0)
								
				j END_SCAN_LOOP
			END_KEY_FOUND:

			# Avança para próxima linha 
			srl $s3, $s3, 1
			j SCAN_LOOP	
		END_SCAN_LOOP:
		
		# Reabilita interruções
		addi $s2, $zero, 0x80
		sb $s2, 0($s1)

		mtc0 $zero, $13

		j END_HANDLER	 
	
END_HANDLER:
	#Epílogo
	lw $s0, 0($sp)
	lw $s1, 4($sp)
	lw $s2, 8($sp)
	lw $s3, 12($sp)
	addi $sp, $sp, 16

	eret
