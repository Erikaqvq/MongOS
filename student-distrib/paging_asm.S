.globl set_regs
.globl page_directory, page_table

.align 4

set_regs:

    pushl %ebp
	movl %esp, %ebp
	movl 8(%esp), %eax
	movl %eax, %cr3 # load dir_pointer in cr3
	
	movl %cr4, %eax
	orl $0x00000010, %eax # from osdev, enable 4mb size page in cr4
	movl %eax, %cr4
	
	movl %cr0, %eax
	orl $0x80000000, %eax # from osdev, open paging enable bit in cr0
	movl %eax, %cr0
	
	movl %ebp, %esp
	popl %ebp
	ret
	
	
	
