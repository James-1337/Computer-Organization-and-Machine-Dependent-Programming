	# Data section messages.
	.data
message:   .asciz "Need To Implement\n"
	
	## Code section
	.text
	.globl createHashTable
	.globl insertData
	.globl findData

#struct HashBucket {
#  void *key;
#  void *data;
#  struct HashBucket *next;
#};

#typedef struct HashTable {
#  unsigned int (*hashFunction)(void *);
#  int (*equalFunction)(void *, void *);
#  struct HashBucket **data;
#  int size;
#  int used;
#} HashTable;


#HashTable *createHashTable(int size,
#                           unsigned int (*hashFunction)(void *),
#                           int (*equalFunction)(void *, void *));
createHashTable:
    # Initialization
	addi sp sp -20
	sw ra 0(sp)
	sw s0 4(sp)
	sw s1 8(sp)
	sw s2 12(sp)
	sw s3 16(sp)

	# assign arguments
	addi s0 a0 0  	# s0 = size
	addi s1 a1 0    # s1 = hash function pointer
	addi s2 a2 0    # s2 = equal function pointer

	li a0 20        # hashtable size = 20
	call malloc     # malloc
	addi s3 a0 0    # s3 = hashtable pointer
	sw s0 12(s3)    # hashtable size = s0
	sw x0 16(s3)    # hash table used = 0
	li t4 4         # t4 = 4 = size of pointer
	mul t4 t4 s0    # t4 = 4 x size
	addi a0 t4 0    # a0 = t4
	call malloc     # malloc
	sw a0 8(s3)     # hash bucket pointer allocation
	li t3 4         # t3 = 4
	li t5 0         # int i = 0

forloop:
    bge t5 s0 next  # Move on if t1 is bigger than or equal to size
    mul t2 t5 t3    # t2 = i x 4
    add t2 a0 t2    # put in offset
    sw x0 0(t2)     # store null in every hash bucket
    addi t5 t5 1    # i++
    j forloop       # go back

next:
    sw s1 0(s3)     # hash function
    sw s2 4(s3)     # equal function
    addi a0 s3 0    # put the table into a0

    # Restoration
    lw ra 0(sp)
    lw s0 4(sp)
    lw s1 8(sp)
    lw s2 12(sp)
    lw s3 16(sp)
    addi sp sp 20
	ret

# void insertData(HashTable *table, void *key, void *data);
insertData:
    # Initialization
	addi sp sp -24
	sw ra 0(sp)
	sw s0 4(sp)
	sw s1 8(sp)
	sw s2 12(sp)
	sw s3 16(sp)
	sw s4 20(sp)

	# assign arguments
	addi s0 a0 0    # s0 = hashtable
	addi s1 a1 0    # s1 = key
	addi s2 a2 0    # s2 = data

	li a0 12        # hashbucket size = 12
	call malloc     # malloc
	addi s3 a0 0    # s3 = hashbucket pointer
	addi a0 s1 0    # a0 = key
	lw s4 0(s0)     # load the hashtable into s4
	jalr ra 0(s4)   # go to the hash function
	addi t3 a0 0    # t3 = hash
	lw t4 12(s0)    # t4 = size
	rem t1 t3 t4    # location = t3 % t4
	li t0 4         # t0 = 4 = size of pointer
	mul t5 t1 t0    # t5 = location x 4
	lw t6 8(s0)     # t6 = data
	add t6 t6 t5    # t6 = data + (location x 4)
	lw t2 0(t6)     # t2 = location of data in the table
	sw t2 8(s3)     # hash bucket next = t2
	sw s2 4(s3)     # hash bucket data = s2
	sw s1 0(s3)     # hash bucket key = s1
	sw s3 0(t6)     # data[location] in the table = hash bucket
	lw t0 16(s0)    # t0 = used size
	addi t0 t0 1    # used size += 1
	sw t0 16(s0)    # add 1 to used size

	# Restoration
    lw ra 0(sp)
    lw s0 4(sp)
    lw s1 8(sp)
    lw s2 12(sp)
    lw s3 16(sp)
	lw s4 20(sp)
    addi sp sp 24
	ret

# void *findData(HashTable *table, void *key);
findData:
    # Initialization
	addi sp sp -20
	sw ra 0(sp)
	sw s0 4(sp)
	sw s1 8(sp)
	sw s2 12(sp)
	sw s3 16(sp)

	# assign arguments
	addi s0 a0 0    # s0 = hash table
	addi s1 a1 0    # s1 = key

	addi a0 s1 0    # a0 = key
	lw t2 0(s0)     # load the hash function into t2
	jalr ra 0(t2)   # go to the hash function
	addi t3 a0 0    # t3 = hash
	lw t4 12(s0)    # t4 = size
	rem t1 t3 t4    # location = t3 % t4
	lw t5 8(s0)     # t5 = data pointer
	li t0 4         # t0 = 4 = size of pointer
	mul t1 t1 t0    # t1 = location x 4
	add t5 t5 t1    # t5 = data pointer real location
	lw s2 0(t5)     # s2 = data t5 pointed at

whileloop:
    beq s2 x0 nextt # Move on if hash bucket is null
    addi a0 s1 0    # a0 = key
    lw a1 0(s2)     # a1 = key to be compared
    lw t6 4(s0)     # t6 = equal function
    jalr ra 0(t6)   # compare a0 and a1
    addi s3 a0 0    # s3 = result

    beq s3 x0 back  # Move on if the result is 0
    lw a0 4(s2)     # a0 = data
    lw ra 0(sp)
    lw s0 4(sp)
    lw s1 8(sp)
    lw s2 12(sp)
    lw s3 16(sp)
    addi sp sp 20
    ret

back:
	lw s2 8(s2)     # s2 = next hash bucket
	j whileloop

nextt:
	# Restoration
    addi a0 x0 0
    lw ra 0(sp)
    lw s0 4(sp)
    lw s1 8(sp)
    lw s2 12(sp)
    lw s3 16(sp)
    addi sp sp 20
	ret