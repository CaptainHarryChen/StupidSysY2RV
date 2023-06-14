.data
.text
.globl my_getint
my_getint:
	addi sp, sp, -128
	sw ra, 124(sp)

my_getint_entry_my_getint:

	li t0, 0
	sw t0, 120(sp)

	j my_getint_while_entry

my_getint_while_entry:

	li t0, 1
	bnez t0, my_getint_skip0
	j my_getint_while_body
my_getint_skip0:
	j my_getint_end

my_getint_while_body:

	call getch
	sw a0, 112(sp)

	lw t0, 112(sp)
	li t1, 48
	sub t0, t0, t1
	sw t0, 108(sp)

	lw t0, 108(sp)
	sw t0, 116(sp)

	li t0, 1
	sw t0, 104(sp)

	lw t0, 116(sp)
	sw t0, 100(sp)

	lw t0, 100(sp)
	li t1, 0
	slt t0, t0, t1
	sw t0, 96(sp)

	lw t0, 96(sp)
	li t1, 0
	xor t0, t0, t1
	seqz t0, t0
	sw t0, 92(sp)

	lw t0, 92(sp)
	bnez t0, my_getint_skip1
	j my_getint_true
my_getint_skip1:
	j my_getint_end_0

my_getint_end:

	lw t0, 116(sp)
	sw t0, 88(sp)

	lw t0, 88(sp)
	sw t0, 120(sp)

	j my_getint_while_entry_0

my_getint_true:

	lw t0, 116(sp)
	sw t0, 84(sp)

	lw t0, 84(sp)
	li t1, 9
	sgt t0, t0, t1
	sw t0, 80(sp)

	lw t0, 80(sp)
	li t1, 0
	xor t0, t0, t1
	snez t0, t0
	sw t0, 76(sp)

	lw t0, 76(sp)
	sw t0, 104(sp)

	j my_getint_end_0

my_getint_end_0:

	lw t0, 104(sp)
	sw t0, 72(sp)

	lw t0, 72(sp)
	bnez t0, my_getint_skip2
	j my_getint_true_0
my_getint_skip2:
	j my_getint_false

my_getint_while_entry_0:

	li t0, 1
	bnez t0, my_getint_skip3
	j my_getint_while_body_0
my_getint_skip3:
	j my_getint_end_1

my_getint_true_0:

	j my_getint_while_entry

my_getint_false:

	j my_getint_end

my_getint_while_body_0:

	call getch
	sw a0, 68(sp)

	lw t0, 68(sp)
	li t1, 48
	sub t0, t0, t1
	sw t0, 64(sp)

	lw t0, 64(sp)
	sw t0, 116(sp)

	li t0, 0
	sw t0, 60(sp)

	lw t0, 116(sp)
	sw t0, 56(sp)

	lw t0, 56(sp)
	li t1, 0
	slt t0, t0, t1
	xori t0, t0, 1
	sw t0, 52(sp)

	lw t0, 52(sp)
	li t1, 0
	xor t0, t0, t1
	snez t0, t0
	sw t0, 48(sp)

	lw t0, 48(sp)
	bnez t0, my_getint_skip4
	j my_getint_true_1
my_getint_skip4:
	j my_getint_end_2

my_getint_end_1:

	lw t0, 120(sp)
	sw t0, 44(sp)

	lw a0, 44(sp)
	lw ra, 124(sp)
	addi sp, sp, 128
	ret

my_getint_true_1:

	lw t0, 116(sp)
	sw t0, 40(sp)

	lw t0, 40(sp)
	li t1, 9
	sgt t0, t0, t1
	xori t0, t0, 1
	sw t0, 36(sp)

	lw t0, 36(sp)
	li t1, 0
	xor t0, t0, t1
	snez t0, t0
	sw t0, 32(sp)

	lw t0, 32(sp)
	sw t0, 60(sp)

	j my_getint_end_2

my_getint_end_2:

	lw t0, 60(sp)
	sw t0, 28(sp)

	lw t0, 28(sp)
	bnez t0, my_getint_skip5
	j my_getint_true_2
my_getint_skip5:
	j my_getint_false_0

my_getint_true_2:

	lw t0, 120(sp)
	sw t0, 24(sp)

	lw t0, 24(sp)
	li t1, 10
	mul t0, t0, t1
	sw t0, 20(sp)

	lw t0, 116(sp)
	sw t0, 16(sp)

	lw t0, 20(sp)
	lw t1, 16(sp)
	add t0, t0, t1
	sw t0, 12(sp)

	lw t0, 12(sp)
	sw t0, 120(sp)

	j my_getint_end_3

my_getint_false_0:

	j my_getint_end_1

my_getint_end_3:

	j my_getint_while_entry_0
.globl my_putint
my_putint:
	addi sp, sp, -160
	sw ra, 156(sp)

my_putint_entry_my_putint:

	sw a0, 152(sp)

	li t0, 0
	sw t0, 84(sp)

	j my_putint_while_entry

my_putint_while_entry:

	lw t0, 152(sp)
	sw t0, 80(sp)

	lw t0, 80(sp)
	li t1, 0
	sgt t0, t0, t1
	sw t0, 76(sp)

	lw t0, 76(sp)
	bnez t0, my_putint_skip6
	j my_putint_while_body
my_putint_skip6:
	j my_putint_end

my_putint_while_body:

	lw t0, 152(sp)
	sw t0, 72(sp)

	lw t0, 72(sp)
	li t1, 10
	rem t0, t0, t1
	sw t0, 68(sp)

	lw t0, 68(sp)
	li t1, 48
	add t0, t0, t1
	sw t0, 64(sp)

	lw t0, 84(sp)
	sw t0, 60(sp)

	addi t0, sp, 88
	lw t1, 60(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 56(sp)

	lw t1, 56(sp)
	lw t0, 64(sp)
	sw t0, 0(t1)

	lw t0, 152(sp)
	sw t0, 52(sp)

	lw t0, 52(sp)
	li t1, 10
	div t0, t0, t1
	sw t0, 48(sp)

	lw t0, 48(sp)
	sw t0, 152(sp)

	lw t0, 84(sp)
	sw t0, 44(sp)

	lw t0, 44(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 40(sp)

	lw t0, 40(sp)
	sw t0, 84(sp)

	j my_putint_while_entry

my_putint_end:

	j my_putint_while_entry_0

my_putint_while_entry_0:

	lw t0, 84(sp)
	sw t0, 36(sp)

	lw t0, 36(sp)
	li t1, 0
	sgt t0, t0, t1
	sw t0, 32(sp)

	lw t0, 32(sp)
	bnez t0, my_putint_skip7
	j my_putint_while_body_0
my_putint_skip7:
	j my_putint_end_0

my_putint_while_body_0:

	lw t0, 84(sp)
	sw t0, 28(sp)

	lw t0, 28(sp)
	li t1, 1
	sub t0, t0, t1
	sw t0, 24(sp)

	lw t0, 24(sp)
	sw t0, 84(sp)

	lw t0, 84(sp)
	sw t0, 20(sp)

	addi t0, sp, 88
	lw t1, 20(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 16(sp)

	lw t0, 16(sp)
	lw t0, 0(t0)
	sw t0, 12(sp)

	lw a0, 12(sp)
	call putch

	j my_putint_while_entry_0

my_putint_end_0:

	lw ra, 156(sp)
	addi sp, sp, 160
	ret
.globl main
main:
	addi sp, sp, -48
	sw ra, 44(sp)

main_entry_main:

	call my_getint
	sw a0, 36(sp)

	lw t0, 36(sp)
	sw t0, 40(sp)

	j main_while_entry

main_while_entry:

	lw t0, 40(sp)
	sw t0, 32(sp)

	lw t0, 32(sp)
	li t1, 0
	sgt t0, t0, t1
	sw t0, 28(sp)

	lw t0, 28(sp)
	bnez t0, main_skip8
	j main_while_body
main_skip8:
	j main_end

main_while_body:

	call my_getint
	sw a0, 20(sp)

	lw t0, 20(sp)
	sw t0, 24(sp)

	lw t0, 24(sp)
	sw t0, 16(sp)

	lw a0, 16(sp)
	call my_putint

	li a0, 10
	call putch

	lw t0, 40(sp)
	sw t0, 12(sp)

	lw t0, 12(sp)
	li t1, 1
	sub t0, t0, t1
	sw t0, 8(sp)

	lw t0, 8(sp)
	sw t0, 40(sp)

	j main_while_entry

main_end:

	li a0, 0
	lw ra, 44(sp)
	addi sp, sp, 48
	ret
