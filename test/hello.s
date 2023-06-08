.text
.globl main
main:
	addi sp, sp, 16

entry_main:
	j while_entry

while_entry:
	li t0, 1
	bnez t0, while_body
	j end

while_body:
	j end

end:

	li a0, 0
	addi sp, sp, 16
	ret
