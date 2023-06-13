.data
.globl a
a:
	.word 10
.text
.globl inc
inc:
	addi sp, sp, -16

entry_inc:

	la t0, a
	lw t0, 0(t0)
	sw t0, 12(sp)

	lw t0, 12(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 8(sp)

	la t1, a
	lw t0, 8(sp)
	sw t0, 0(t1)

	la t0, a
	lw t0, 0(t0)
	sw t0, 4(sp)

	lw a0, 4(sp)
	addi sp, sp, 16
	ret
.globl print_a
print_a:
	addi sp, sp, -16
	sw ra, 12(sp)

entry_print_a:

	la t0, a
	lw t0, 0(t0)
	sw t0, 8(sp)

	lw a0, 8(sp)
	call putint

	li a0, 10
	call putch

	lw ra, 12(sp)
	addi sp, sp, 16
	ret
.globl main
main:
	addi sp, sp, -48
	sw ra, 44(sp)

entry_main:

	li t0, 0
	sw t0, 40(sp)

	j while_entry

while_entry:

	lw t0, 40(sp)
	sw t0, 36(sp)

	lw t0, 36(sp)
	li t1, 10
	slt t0, t0, t1
	sw t0, 32(sp)

	lw t0, 32(sp)
	bnez t0, while_body
	j end

while_body:

	call inc
	sw a0, 28(sp)

	li t0, 1
	sw t0, 24(sp)

	lw t0, 24(sp)
	sw t0, 20(sp)

	lw t0, 20(sp)
	li t1, 2
	add t0, t0, t1
	sw t0, 16(sp)

	lw t0, 16(sp)
	sw t0, 24(sp)

	lw t0, 24(sp)
	sw t0, 12(sp)

	lw a0, 12(sp)
	call putint

	li a0, 10
	call putch

	call print_a

	lw t0, 40(sp)
	sw t0, 8(sp)

	lw t0, 8(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 4(sp)

	lw t0, 4(sp)
	sw t0, 40(sp)

	j while_entry

end:

	li a0, 0
	lw ra, 44(sp)
	addi sp, sp, 48
	ret
