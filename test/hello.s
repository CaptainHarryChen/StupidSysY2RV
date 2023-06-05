.text
.globl main
main:
	addi sp, sp, 48

entry_main:

	li t0, 1
	sw t0, 44(sp)

	li t0, 2
	sw t0, 40(sp)

	li t0, 1
	sw t0, 36(sp)

	lw t0, 44(sp)
	sw t0, 32(sp)

	lw t0, 32(sp)
	li t1, 0
	xor t0, t0, t1
	seqz t0, t0
	sw t0, 28(sp)
	lw t0, 28(sp)
	bnez t0, true
	j end

true:

	lw t0, 40(sp)
	sw t0, 24(sp)

	lw t0, 24(sp)
	li t1, 0
	xor t0, t0, t1
	snez t0, t0
	sw t0, 20(sp)

	lw t0, 20(sp)
	sw t0, 36(sp)
	j end

end:

	lw t0, 36(sp)
	sw t0, 16(sp)
	lw t0, 16(sp)
	bnez t0, true_0
	j false

true_0:

	lw t0, 44(sp)
	sw t0, 12(sp)

	lw t0, 40(sp)
	sw t0, 8(sp)

	lw t0, 12(sp)
	lw t1, 8(sp)
	add t0, t0, t1
	sw t0, 4(sp)

	lw t0, 4(sp)
	sw t0, 44(sp)
	j end_0

false:
	j end_0

end_0:

	lw t0, 44(sp)
	sw t0, 0(sp)

	lw a0, 0(sp)
	addi sp, sp, 48
	ret
