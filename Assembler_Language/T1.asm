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
MAIN:
# ===	=====	=====	=====	=====	#
# Definição do registro da pilha:	#
# 0 - ponteiro para topo da pilha	#
# 4 - ponteiro para base da pilha	#
# 8 - contador de elementos da pilha	#
# ===	=====	=====	=====	=====	#

#Aloca espaço para a pilha de numeros
addi $v0, $zero, 9    
addi $a0, $zero, 10
syscall		

# Salvar registro da pilha na pilha
addi $sp, $sp, -12

add $t0, $zero, $v0		
add $t1, $zero, $v0		
add $t2, $zero, $zero

sw $t0, 0($sp)
sw $t1, 4($sp)
sw $t2, 8($sp)		

# ===	=====	=====	=====	=====	=====	#
# $s0 - ponteiro para pilha			#
# $s1 - ponteiro para a tecla pressionada	#
# $s2 - tecla pressionada			#
# $s3 - valor no visor				#
# ===	=====	=====	=====	=====	=====	#

add $s0, $zero, $t0
lui $s1, 0x1001
ori $s1, $s1, 0x000b		
add $s3, $zero, $zero

# Inicializa as interrupções do teclado hexadecimal
lui $t0, 0xFFFF     		
ori $t0, $t0, 0x0012
addi $t1, $zero, 0x80
sb $t1, 0($t0)

# Loop principal do programa
MAIN_LOOP:
	# Verifica se alguma tecla foi pressionada
	lbu $s2, 0($s1)
	beq $s2, $zero, MAIN_LOOP 
	# Converte a tecla para o valor que ela representa
	add $a0, $zero, $s2
	jal CONVERT_KEY_TO_NUMBER
	# Se for um digito, concatena ele ao valor decimal
	addi $t0, $zero, 10
	mul $s3, $s3, $t0	# n * 10
	add $s3, $s3, $v0	# n * 10 + d
	addi $t0, $zero, 100
	div $s3, $t0
	mfhi $s3		# (n * 10 + d) % 100

	# Mostra o número no display
	add $a0, $zero, $s3	
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
	# Convertemos as coordenadas da tecla para valor usando: x = 4 * log_2(linha) + log_2(coluna)

	# Separamos as coordenadas: linha (bits menos significativos) e coluna (bits mais significativos)
	andi $t0, $a0, 0x0F # t0 -> linha
	srl $t1, $a0, 4		# t1 -> coluna

	# log_2(x) = 31 - numero de zeros a esquerda; se x = 2^n
	addi $t2, $zero, 31
	# log(linha)
	clz $t0, $t0
	sub $t0, $t2, $t0
	# log(coluna)
	clz $t1, $t1
	sub $t1, $t2, $t1
	# 4 * log(linha)
	sll $v0, $t0, 2
	# 4 * log(linha) + log(coluna)
	add $v0, $v0, $t1

	jr $ra

# ===	=====	=====	=====	=====	#
# Função DISPLAY_NUMBER			#
# Descrição: mostra número no display	#
# Input: $a0 - numero a ser mostrado	#
# Output: mostra numero no display	#
# ===	=====	=====	=====	=====	#	
DISPLAY_NUMBER:
	# Prologo
	# Salva o endereço de retorno do procedimento
	addi $sp, $sp, -8
	sw $s0, 4($sp)
	sw $ra, 0($sp)

	# Verifica se o número é válido
	addi $t0, $zero, 99
	sub $t0, $t0, $a0
	bgez $t0, ELSE_ERROR
	# Se não for válido, exibe um erro
	IF_ERROR:
		lui $t0, 0x1001
		ori $t0, $t0, 0x000a
		lbu $s0, 0($t0)
		
		add $a0, $zero, $s0
		j END_ERROR
	# Se for válido, exibe os digitos esquerdo e direitos
	ELSE_ERROR:
		addi $t0, $zero, 10
		div $a0, $t0
		mflo $a0
		mfhi $s0
	END_ERROR:
	# Exibe o digito esquerdo e direito
	jal DISPLAY_LEFT_DIGIT
	add $a0, $zero, $s0
	jal DISPLAY_RIGHT_DIGIT
	
	# Epilogo
	# Recarrega o endereço de retorno do procedimento
	lw $ra, 0($sp)
	lw $s0, 4($sp)
	addi $sp, $sp, 8
		
	jr $ra

# ===	=====	=====	=====	=====	=====	#
# Função DISPLAY_RIGHT_DIGIT			#
# Descrição: mostra digito no display direito	#
# Input: $a0 - digito a ser mostrado		#
# Output: mostra digito no display direito	#
# ===	=====	=====	=====	=====	=====	#	
DISPLAY_RIGHT_DIGIT:	
	# Ponteiro para do display direito
	lui $t0, 0xFFFF
	ori $t0, $t0, 0x0010
	# Ponteiro para do código do digito
	lui $t1, 0x1001
	or $t1, $t1, $a0
	# Salva o código no display direito
	lbu $t1, 0($t1)
	sb $t1, 0($t0)

	jr $ra

# ===	=====	=====	=====	=====	=====	#
# Função DISPLAY_LEFT_DIGIT			#
# Descrição: mostra digito no display esquerdo	#
# Input: $a0 - digito a ser mostrado		#
# Output: mostra digito no display esquerdo	#
# ===	=====	=====	=====	=====	=====	#	
DISPLAY_LEFT_DIGIT:
	# Ponteiro para do display esquerdo
	lui $t0, 0xFFFF
	ori $t0, $t0, 0x0011
	# Ponteiro para do código do digito
	lui $t1, 0x1001
	or $t1, $t1, $a0
	# Salva o código no display esquerdo
	lbu $t1, 0($t1)
	sb $t1, 0($t0)
	
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
	# $t0 - ponteiro para o topo da pilha		#
	# $t1 - numero de elementos na pilha		#
	# $t2 - maximo da piha				#
	# ===	=====	=====	=====	=====	=====	#
	lw $t0, 0($a0)
	lw $t1, 8($a0)
	addi $t2, $zero, 10
	
	beq $t1, $t2, END_IF_STACK_NOT_FULL
	# Se a pilha não estiver cheia insere o elemento e atualiza a pilha
	IF_STACK_NOT_FULL:
		# Incrementa ponteiro para próxima posição e aumenta número de elementos
		addi $t0, $t0, 1
		addi $t1, $t1, 1
		# Salva o elemento no novo topo da pilha
		sb $a1, 0($t0) 
		# Atualiza a pilha com os seus novos valores
		sw $t0, 0($a0)
		sw $t1, 8($a0)
	END_IF_STACK_NOT_FULL:
	
	jr $ra
	
# ===	=====	=====	=====	=====	#
# Função: CLEAR_STACK			#
# Descrição: zera a pilha		#
# Input: $a0 - ponteiro para pilha	#
# Output: pilha é esvaziada		#
# ===	=====	=====	=====	=====	#	
CLEAR_STACK:
	lw $t0, 4($a0)		# ponteiro para a base
	sw $t0, 0($a0)		# faz o ponteiro do topo apontar para base
	sw $zero, 8($a0)	# zera o número de elementos

	jr $ra
	
ARITHMETIC_MEAN:
	add $t0, $zero, $s0	
	add $t1, $zero, $s2	
	add $t2, $zero, $zero	
	
	WHILE_ARITHMETIC_MEAN:
		blez $t1, END_WHILE_ARITHMETIC_MEAN
	
		lbu $t3, 0($t0) 
		add $t2, $t2, $t3
	
		addi $t0, $t0, -1
		addi $t1, $t1, -1
		j WHILE_ARITHMETIC_MEAN
	END_WHILE_ARITHMETIC_MEAN:
	
	div $t2, $s2
	mflo $v0
	
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
	# precisamos salva-los para grantir o funcionamento do programa
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

		j END_HANDLER	 
		
	
END_HANDLER:
	#Epílogo
	lw $s0, 0($sp)
	lw $s1, 4($sp)
	lw $s2, 8($sp)
	lw $s3, 12($sp)
	addi $sp, $sp, 16

	eret
