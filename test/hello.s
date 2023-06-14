.data
.text
.globl init
init:
	addi sp, sp, -32

init_entry_init:

	sw a0, 28(sp)

	lw t0, 28(sp)
	sw t0, 24(sp)

	addi t0, sp, 24
	lw t0, 0(t0)
	li t1, 1
	li t2, 400
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 20(sp)

	addi t0, sp, 20
	lw t0, 0(t0)
	li t1, 2
	li t2, 40
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 16(sp)

	addi t0, sp, 16
	lw t0, 0(t0)
	li t1, 3
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 12(sp)

	lw t1, 12(sp)
	li t0, 3
	sw t0, 0(t1)

	addi sp, sp, 32
	ret
.globl main
main:
	li t0, -4032
	add sp, sp, t0
	li t0, 4028
	add t0, sp, t0
	sw ra, 0(t0)

main_entry_main:

	li t0, 0
	sw t0, 24(sp)

	addi t0, sp, 28
	li t1, 0
	li t2, 400
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 20(sp)

	addi t0, sp, 28
	li t1, 0
	li t2, 400
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 16(sp)

	lw a0, 16(sp)
	call init

	lw t0, 24(sp)
	sw t0, 12(sp)

	lw a0, 12(sp)
	call putint

	li a0, 10
	call putch

	li a0, 0
	li t6, 4028
	add t6, t6, sp
	lw ra, 0(t6)
	li t0, 4032
	add sp, sp, t0
	ret
