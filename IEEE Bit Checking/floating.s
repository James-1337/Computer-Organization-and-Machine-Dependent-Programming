
	# Constants for system calling, for the print functions
	# and the like 
	.equ PRINT_DEC 0
	.equ PRINT_STR 4
	.equ PRINT_HEX 1
	.equ READ_HEX 11
	.equ EXIT 20

	# Data section messages.
	.data
newline:   .asciz "\n"
welcome:   .asciz "Welcome to Floating in Assembly\n"
invalid:   .asciz "Invalid hexidecimal\n"
argument:  .asciz "Argument: "
ashex:	   .asciz "As hex:          "
as16:      .asciz "To 16b Floating: "
	
	## Code section
	.text
	.globl main

main:
	# Preamble for main:
	# s0 = argc
	# s1 = argv
	# s2 = loop index i
	# s3 = A callee saved temporary
	# that is used to cross some call boundaries
	addi sp sp -20
	sw ra 0(sp)
	sw s0 4(sp)
	sw s1 8(sp)
	sw s2 12(sp)
	sw s3 16(sp)

	# Keep argc and argv around, and initialize i to 1
	mv s0 a0
	mv s1 a1
	
	# Print the welcome message
	la a0 welcome
	jal printstr

	# for i = 1, i < argc, ++i
	li s2 1
loop_start:	
	bge s2 s0 loop_exit

	li a0 argument
	jal printstr
	
	slli t0  s2 2  # t0 = i * 4
	add  t0  t0 s1 # t0 = argv + (4 * i)
	lw   s3 0(t0)  # s3 = argv[i]

	mv a0 s3
	jal printstr
	la a0 newline
	jal printstr

	la a0 ashex
	jal printstr
	
	mv a0 s3       
	jal parsehex
	mv s3 a0       # s3 = parsehex(argv[i])
        jal printhex
	la a0 newline
	jal printstr

	la a0 as16
	jal printstr

	# Do the actual conversion, and print it out.
	mv a0 s3
	jal as_ieee_16
	jal printhex
	la a0 newline
	jal printstr

	la a0 newline
	jal printstr
	
	addi s2 s2 1	
	j loop_start
loop_exit:

	lw s0 4(sp)
	lw s1 8(sp)
	lw s2 12(sp)
	lw s3 16(sp)
	lw ra 0(sp)
	addi sp sp 20
	ret

	# Function for parsing a hexidecimal string
	# given as a string.  In C its declaration would
	# be
	# uint32_t parsehex(char * str)

	# We need this because although the simulator has
	# a built in "read number in hex", THAT is reading
	# from the console and we want to read from the command line.

	# This is not a leaf function becaues it will print an error
	# if the item is not well formed.
parsehex:
	addi sp sp -12
	sw ra 0(sp) # We need some saved variables
	sw s0 4(sp) # str
	sw s1 8(sp) # the return value
	mv s0 a0    # Save str in s0
	li s1 0     # Return value starts at 0
	li t1 '0'   # Temporary values for ASCII character
	li t2 '9'   # constants that are compared against.
	li t3 'A'
	li t4 'F'
	li t5 'a'
	li t6 'f'

	# This takes advantage that "0-9" < "A-F" < "a-f" so
	# we can add/subtract the values and compare on the
	# range

	# while (*str) != 0
parsehex_loop:       
	lbu t0 0(s0) 		     # t0 = *str
	beqz t0 parsehex_exit
	
	blt t0 t1 parsehex_error     # if(*str < '0') -> error
	bgt t0 t2 parsehex_not_digit # if(*str > '9') -> not digit
	sub t0 t0 t1                 # to = *str - '0'
	j parsehex_loop_end

parsehex_not_digit:
	blt t0 t3 parsehex_error     # if(*str < 'A') -> error
	bgt t0 t4 parsehex_lower     # if(*str > 'F') -> not upper
	sub t0 t0 t3                 # t0 = *str - 'A' + 10
	addi t0 t0 10
	j parsehex_loop_end

parsehex_lower:
	blt t0 t5 parsehex_error     # if(*str < 'a') -> error
	bgt t0 t6 parsehex_error     # if(*str > 'f') -> error
	sub t0 t0 t5                 # to = *str - 'a' + 10
	addi t0 t0 10

parsehex_loop_end:
	slli s1 s1 4                 # ret = ret << 4 | t0
	or s1 s1 t0
	addi s0 s0 1                 # str++
	j parsehex_loop

parsehex_error:
	la a0 invalid
	jal printstr
	li s0 0xFFFFFFFF
	j parsehex_exit
	
parsehex_exit:
	mv a0 s1                     # set return value and cleanup
	lw ra 0(sp)
	lw s0 4(sp)
	lw s1 8(sp)
	addi sp sp 12
	ret

	# This is an example of using ecall to call
	# one of the built-in system routines
printhex:	
	li a7 PRINT_HEX
	ecall
	ret
	
printstr:
	li a7 PRINT_STR
	ecall
	ret


# DO NOT CHANGE ANY CODE ABOVE THIS LINE!

	# This is the function you need to complete,
	# It is the same as the C version.  It accepts
	# a 32b value in IEEE floating point, and returns
	# a 16b value that is the IEEE half-precision floating
	# point number.  The upper 16b of the returned data
	# should be 0

	
	# This is a leaf function so we don't need
	# to save any caller saved registers (e.g. ra)
	# UNLESS you want to call other functions	
as_ieee_16:
	# Set some values
    lw  t0, 0(a0)				# Load the 32b word into t0
    srli  t1, t0, 31			# Shift the word right by 31 to get the sign bit to t1
	slli  t2, t0, 1				# Shift the word left by 1 and then right by 24 to get the exponent byte to t2
    srli  t2, t2, 24
    slli  t3, t0, 9				# Shift the word left by 9 and then right by 9 to get the mantissa to t3
	srli  t3, t3, 9

	# Add back in explicit 1
	srli  t4, t3, 13			# Shift right by 13 to get left 10 bits
	ori   t4, t4, 0x400			# add back in the explicit 1 to the left
	li    t6, 0x1FFF			# Load 0x1FFF into t6
	and   t5, t4, t6			# Sets the remainder

	# Check for NaN
	li    t6, 0xFF 				# Set t6 to 0xFF
	bne   t2, t6, notNaN		# Move on if the exponent is not all 1
	beq	  t3, x0, notNaN		# Move on if the mantissa is equal to 0
	li    a0, 0xFFFF			# Set the returned value to 0xFFFF
	ret							# return

notNaN:
	# Check if subnormal
	addi  t2, t2, -127			# Adjust exponent bias
	li    t6, -14				# Set t6 to -14
	bge   t2, t6, remainder		# Move on if the exponent is more than -14
	addi  t6, t2, 14			# add 14 to the exponent
	sub   t6, x0, t6 			# change it to negative
	li    t0, 0x800000			# set t0 to 0x800000
	or    t3, t3, t0			# add the leftmost bit to the mantissa
	srl   t3, t3, t6			# Shift mantissa right by t6
	srli  t4, t3, 13			# Shift the mantissa right by 13 and assign it to t4
	li    t5, 0x1FFF 			# Sets the remainder
	li    t2, -14				# Sets the exponent to -14


remainder:
	# Check if remainder is greater than 0x1000
	li    t6, 0x1000			# Set t6 to 0x1000
	blt   t5, t6, remai 			# Move on if the remainder is less than 0x1000
	andi  s0, t4, 1				# Bitwise & main and 1
	sub   s1, t6, t5			# subtract the remainder from 0x1000 (should be 0 if they are the same)
	add	  s0, s0, s1			# add the two values together
	blt   x0, s0, remai			# Move on if the two values added together is more than 0
	addi  t4, t4, 1				# Add 1 to t4

remai:
	# Check if main > 0x7FF
	li    t6, 0x7FF				# Load 0x7FF into t6
	bge   t6, t4, posit			# Move on if main is less than or equal to 0x7FF
	addi  t2, t2, 1				# add 1 to exponent
	srli  t4, t4, 1				# bitshift main to the right by 1

posit:	
	# Check if exponent > 15
	li    t6, 15				# Load 15 into t6
	bge   t6, t2, subaa			# Move on if exponent is less than or equal to 15
	slli  a0, t1, 15			# Set a0 to have just the sign at the front	
	li    t6, 0x7c00
	add  a0, a0, t6				# add 0x7c00 to a0
	ret							# Return

subaa:
	# Check if main < 0x400 and exponent == -14
	li    t6, 0x400				# Load 0x400 into t6
	bge   t4, t6, final			# Branch if main is greater than or equal to 0x400
	li    t6, -14				# Load -14 into t6
	bne   t2, t6, final			# Branch if exponent is not equal to -14
	slli  a0, t1, 15			# Set a0 to have just the sign at the front
	andi  t6, t4, 0x3FF			# Bitwise & main and 0x3FF
	or    a0, a0, t6			# add t6 to a0
	ret							# Return

final:
	slli  a0, t1, 15			# Set a0 to have just the sign at the front
	andi  t6, t4, 0x3FF			# Bitwise & main and 0x3FF
	or    a0, a0, t6			# add t6 to a0
	addi  t2, t2, 15			# Bias the exponent by 15
	andi  t2, t2, 0x1F			# have only 5 bits
	slli  t2, t2, 10			# Shift to have them in the right place
	or    a0, a0, t2			# Merge everything together
	ret