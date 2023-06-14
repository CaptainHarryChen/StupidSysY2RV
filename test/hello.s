.data
.globl ball
ball:
	.zero 44
.globl temp
temp:
	.zero 88
.text
.globl dfs
dfs:
	addi sp, sp, -352
	sw ra, 348(sp)

dfs_entry_dfs:

	sw a0, 344(sp)

	lw t0, 344(sp)
	sw t0, 340(sp)

	lw t0, 340(sp)
	li t1, 10
	sgt t0, t0, t1
	sw t0, 336(sp)

	lw t0, 336(sp)
	bnez t0, dfs_true
	j dfs_false

dfs_true:

	li a0, 1
	lw ra, 348(sp)
	addi sp, sp, 352
	ret

dfs_false:

	j dfs_end

dfs_end:

	li t0, 1
	sw t0, 332(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 328(sp)

	addi t0, sp, 328
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 324(sp)

	lw t0, 324(sp)
	lw t0, 0(t0)
	sw t0, 320(sp)

	lw t0, 320(sp)
	li t1, 0
	xor t0, t0, t1
	seqz t0, t0
	sw t0, 316(sp)

	lw t0, 316(sp)
	li t1, 0
	xor t0, t0, t1
	seqz t0, t0
	sw t0, 312(sp)

	lw t0, 312(sp)
	bnez t0, dfs_true_0
	j dfs_end_0

dfs_true_0:

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 308(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 304(sp)

	addi t0, sp, 304
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 300(sp)

	lw t0, 300(sp)
	lw t0, 0(t0)
	sw t0, 296(sp)

	addi t0, sp, 308
	lw t0, 0(t0)
	lw t1, 296(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 292(sp)

	lw t0, 292(sp)
	lw t0, 0(t0)
	sw t0, 288(sp)

	lw t0, 344(sp)
	sw t0, 284(sp)

	la t0, ball
	lw t1, 284(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 280(sp)

	lw t0, 280(sp)
	lw t0, 0(t0)
	sw t0, 276(sp)

	lw t0, 288(sp)
	lw t1, 276(sp)
	slt t0, t0, t1
	sw t0, 272(sp)

	lw t0, 272(sp)
	li t1, 0
	xor t0, t0, t1
	snez t0, t0
	sw t0, 268(sp)

	lw t0, 268(sp)
	sw t0, 332(sp)

	j dfs_end_0

dfs_end_0:

	lw t0, 332(sp)
	sw t0, 264(sp)

	lw t0, 264(sp)
	bnez t0, dfs_true_1
	j dfs_false_0

dfs_true_1:

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 260(sp)

	addi t0, sp, 260
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 256(sp)

	lw t0, 256(sp)
	lw t0, 0(t0)
	sw t0, 252(sp)

	lw t0, 252(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 248(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 244(sp)

	addi t0, sp, 244
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 240(sp)

	lw t1, 240(sp)
	lw t0, 248(sp)
	sw t0, 0(t1)

	lw t0, 344(sp)
	sw t0, 236(sp)

	la t0, ball
	lw t1, 236(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 232(sp)

	lw t0, 232(sp)
	lw t0, 0(t0)
	sw t0, 228(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 224(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 220(sp)

	addi t0, sp, 220
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 216(sp)

	lw t0, 216(sp)
	lw t0, 0(t0)
	sw t0, 212(sp)

	addi t0, sp, 224
	lw t0, 0(t0)
	lw t1, 212(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 208(sp)

	lw t1, 208(sp)
	lw t0, 228(sp)
	sw t0, 0(t1)

	lw t0, 344(sp)
	sw t0, 204(sp)

	lw t0, 204(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 200(sp)

	lw a0, 200(sp)
	call dfs
	sw a0, 196(sp)

	lw t0, 196(sp)
	bnez t0, dfs_true_2
	j dfs_false_1

dfs_false_0:

	j dfs_end_2

dfs_true_2:

	li a0, 1
	lw ra, 348(sp)
	addi sp, sp, 352
	ret

dfs_false_1:

	j dfs_end_1

dfs_end_2:

	li t0, 1
	sw t0, 192(sp)

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 188(sp)

	addi t0, sp, 188
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 184(sp)

	lw t0, 184(sp)
	lw t0, 0(t0)
	sw t0, 180(sp)

	lw t0, 180(sp)
	li t1, 0
	xor t0, t0, t1
	seqz t0, t0
	sw t0, 176(sp)

	lw t0, 176(sp)
	li t1, 0
	xor t0, t0, t1
	seqz t0, t0
	sw t0, 172(sp)

	lw t0, 172(sp)
	bnez t0, dfs_true_3
	j dfs_end_3

dfs_end_1:

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 168(sp)

	addi t0, sp, 168
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 164(sp)

	lw t0, 164(sp)
	lw t0, 0(t0)
	sw t0, 160(sp)

	lw t0, 160(sp)
	li t1, 1
	sub t0, t0, t1
	sw t0, 156(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 152(sp)

	addi t0, sp, 152
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 148(sp)

	lw t1, 148(sp)
	lw t0, 156(sp)
	sw t0, 0(t1)

	j dfs_end_2

dfs_true_3:

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 144(sp)

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 140(sp)

	addi t0, sp, 140
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 136(sp)

	lw t0, 136(sp)
	lw t0, 0(t0)
	sw t0, 132(sp)

	addi t0, sp, 144
	lw t0, 0(t0)
	lw t1, 132(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 128(sp)

	lw t0, 128(sp)
	lw t0, 0(t0)
	sw t0, 124(sp)

	lw t0, 344(sp)
	sw t0, 120(sp)

	la t0, ball
	lw t1, 120(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 116(sp)

	lw t0, 116(sp)
	lw t0, 0(t0)
	sw t0, 112(sp)

	lw t0, 124(sp)
	lw t1, 112(sp)
	slt t0, t0, t1
	sw t0, 108(sp)

	lw t0, 108(sp)
	li t1, 0
	xor t0, t0, t1
	snez t0, t0
	sw t0, 104(sp)

	lw t0, 104(sp)
	sw t0, 192(sp)

	j dfs_end_3

dfs_end_3:

	lw t0, 192(sp)
	sw t0, 100(sp)

	lw t0, 100(sp)
	bnez t0, dfs_true_4
	j dfs_false_2

dfs_true_4:

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 96(sp)

	addi t0, sp, 96
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 92(sp)

	lw t0, 92(sp)
	lw t0, 0(t0)
	sw t0, 88(sp)

	lw t0, 88(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 84(sp)

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 80(sp)

	addi t0, sp, 80
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 76(sp)

	lw t1, 76(sp)
	lw t0, 84(sp)
	sw t0, 0(t1)

	lw t0, 344(sp)
	sw t0, 72(sp)

	la t0, ball
	lw t1, 72(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 68(sp)

	lw t0, 68(sp)
	lw t0, 0(t0)
	sw t0, 64(sp)

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 60(sp)

	la t0, temp
	li t1, 1
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 56(sp)

	addi t0, sp, 56
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 52(sp)

	lw t0, 52(sp)
	lw t0, 0(t0)
	sw t0, 48(sp)

	addi t0, sp, 60
	lw t0, 0(t0)
	lw t1, 48(sp)
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 44(sp)

	lw t1, 44(sp)
	lw t0, 64(sp)
	sw t0, 0(t1)

	lw t0, 344(sp)
	sw t0, 40(sp)

	lw t0, 40(sp)
	li t1, 1
	add t0, t0, t1
	sw t0, 36(sp)

	lw a0, 36(sp)
	call dfs
	sw a0, 32(sp)

	lw t0, 32(sp)
	bnez t0, dfs_true_5
	j dfs_false_3

dfs_false_2:

	j dfs_end_5

dfs_true_5:

	li a0, 1
	lw ra, 348(sp)
	addi sp, sp, 352
	ret

dfs_false_3:

	j dfs_end_4

dfs_end_5:

	li a0, 0
	lw ra, 348(sp)
	addi sp, sp, 352
	ret

dfs_end_4:

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 28(sp)

	addi t0, sp, 28
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 24(sp)

	lw t0, 24(sp)
	lw t0, 0(t0)
	sw t0, 20(sp)

	lw t0, 20(sp)
	li t1, 1
	sub t0, t0, t1
	sw t0, 16(sp)

	la t0, temp
	li t1, 0
	li t2, 44
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 12(sp)

	addi t0, sp, 12
	lw t0, 0(t0)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 8(sp)

	lw t1, 8(sp)
	lw t0, 16(sp)
	sw t0, 0(t1)

	j dfs_end_5
.globl main
main:
	addi sp, sp, -32
	sw ra, 28(sp)

main_entry_main:

	la t0, ball
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 24(sp)

	la t0, ball
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	add t0, t0, t1
	sw t0, 20(sp)

	lw a0, 20(sp)
	call getarray
	sw a0, 16(sp)

	li a0, 1
	call dfs
	sw a0, 8(sp)

	lw t0, 8(sp)
	sw t0, 12(sp)

	lw t0, 12(sp)
	sw t0, 4(sp)

	lw a0, 4(sp)
	call putint

	li a0, 10
	call putch

	li a0, 0
	lw ra, 28(sp)
	addi sp, sp, 32
	ret
