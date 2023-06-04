.text
.globl main
main:
	addi sp, sp, 16

	li t0, 1
	sw t0, 12(sp)

	li t0, 2
	sw t0, 12(sp)

	li t0, 3
	sw t0, 8(sp)

	lw t0, 12(sp)
	sw t0, 4(sp)

	lw a0, 4(sp)
	addi sp, sp, 16
	ret
