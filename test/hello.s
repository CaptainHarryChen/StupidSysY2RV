.text
.globl main
main:
	addi sp, sp, -16

entry_main:

	lw t0, 8(sp)
	sw t0, 12(sp)

	lw t0, 12(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 4(sp)

	lw a0, 4(sp)
	addi sp, sp, 16
	ret
