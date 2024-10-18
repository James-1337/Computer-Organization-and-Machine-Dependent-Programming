/*  
 * We are using GCC's assembler for this, which
 * allow C style comments.  GCC defaults to AT&T syntax (op src, dest)
 * but we'd much rather use Intel syntax (op dest, src) because it is
 * more similar to RISC-V and Intel's reference material uses Intel 
 * syntax (naturally).  So we have the following directive.
 *
 * Intel syntax is better because it also automatically infers the types
 * based on the register specifier, eliminating the need to include types
 * in most operations which would otherwise prove tedious and annoying.
 */
.intel_syntax noprefix

/*
 * However, compiler directives remain in the GNU format.
 */
.file "hashtable.s"
.text
.section .rodata

todo:	.string "Need to implement!"

.text
	.globl createHashTable
	.globl insertData
	.globl findData


createHashTable:
    # Initialization
	sub rsp, 56
	mov [rsp], r12d         # 32b size
	mov [rsp+4], r13        # 64b hash function
	mov [rsp+12], r14       # 64b equal function pointer
	mov [rsp+20], r15       # hashtable

	mov r12d, edi           # Put the size argument into r12
	mov r13, rsi			# Put the hash function pointer into r13
	mov r14, rdx			# Put the equal function pointer into r14

	mov edi, 32 			# set the arguments to call calloc for the hash table
	mov esi, 1
	call calloc             # Space allocation
	mov r15, rax			# Put the pointer to the allocated space in r15
	mov [r15+0], r13		# hash function = r13
	mov [r15+8], r14		# equal function = r14
	mov [r15+24], r12d 		# hash table size = r12d
	xor r10d, r10d          # zero out r10
    mov [r15+28], r10d      # used = 32b 0
	mov edi, r12d			# Set the arguments to call calloc for the data
	mov esi, 8
	call calloc             # Space allocation (zeroes out all data)
	mov [r15+16], rax		# Put the pointer to the allocated space in the data section of the hashtable
	mov rax, r15			# return value = hash table

    # Restoration
	mov r12d, [rsp]
	mov r13, [rsp+4]
	mov r14, [rsp+12]
	mov r15, [rsp+20]
	add rsp, 56
	ret

insertData:
    # Initialization
	sub rsp, 56
	mov [rsp], r12          # 64b hashtable pointer
	mov [rsp+8], r13        # 64b key pointer
	mov [rsp+16], r14       # 64b data pointer
	mov [rsp+24], r15       # hash bucket

	mov r12, rdi			# Put the table pointer into r12
	mov r13, rsi			# Put the key pointer into r13
	mov r14, rdx			# Put the data pointer into r14

	mov edi, 24             # set the arguments to call calloc for the hash bucket
	mov esi, 1
	call calloc             # Space allocation
	mov r15, rax			# Put the hash bucket pointer into r15

	mov rdi, r13            # Set the argument to call the hash function
	call [r12]				# hash function call on the key pointer
	mov	r10d, [r12+24]		# r10d = hash table size
	xor rdx, rdx				# zero out upper bits for division
	div r10d				# divide the hash by size, remainder goes into rdx
	mov r10, [r12+16]		# r10 = data address
	mov r11, [(8*rdx)+r10]	# r11 = data pointer

	mov [r15], r13			# put the key pointer in
	mov [r15+8], r14		# put the data pointer in
	mov [r15+16], r11		# Put data address into the next
	mov [r10+8*rdx], r15	# Put the hash bucket into the hashtable

	mov r10d, [r12+28]		# r10d = used
	add r10d, 1			    # used++
	mov [r12+28], r10d		# put r10 back in after

	# Restoration
	mov r12, [rsp]
	mov r13, [rsp+8]
	mov r14, [rsp+16]
	mov r15, [rsp+24]
	add rsp, 56
	ret

findData:
    # Initialization
	sub rsp, 24
	mov [rsp], r12          # 64b hashtable pointer
	mov [rsp+8], r13        # 64b key pointer
	mov [rsp+16], r14       # hash table

	mov r12, rdi			# Put the hash table pointer into r12
	mov r13, rsi			# Put the key pointer into r13

	mov rdi, r13			# Set the argument to call the hash function
	call [r12]				# hash function call on the key pointer
	mov	r10d, [r12+24]		# r10d = hash table size
	xor rdx, rdx			# zero out upper bits for division
	div r10d				# divide the hash by size, remainder goes into rdx
	mov r10, [r12+16]		# r10 = data address
	mov r14, [(8*rdx)+r10]	# r14 = temp hash bucket

whileloop:
	cmp r14, 0              # Move on if the temp hash bucket is 0
	je next

	mov rdi, r13			# Set the argument to call the equal function
	mov rsi, [r14]			# rdi and rsi are keys
	call [r12+8]			# equal function call on the two keys
	cmp rax, 0              # compare the result to 0
	cmove r14, [r14+16]		# go to the next hash bucket if not equal (result is 0)
	je whileloop

	mov rax, [r14+8]		# Move on if the keys are equal (result is not 0)
	jmp restoration
next:
    # Restoration after finding nothing
	mov rax, 0              # return value = 0
	mov r12, [rsp]
	mov r13, [rsp+8]
	mov r14, [rsp+16]
	add rsp, 24
	ret
restoration:
    # Restoration
	mov r12, [rsp]
	mov r13, [rsp+8]
	mov r14, [rsp+16]
	add rsp, 24
	ret
