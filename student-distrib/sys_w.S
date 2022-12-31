#define ASM 1

.globl syscall_w

syscall_w:
    pushl %edx    
    pushl %ecx     
    pushl %ebx
    pushl %edi
    pushl %ebp
    pushl %esi
    # push all parameters
    pushl %edx    
    pushl %ecx     
    pushl %ebx     

    cmp $0, %eax # syscall number should be greater than 0
    jle invalid_call
    cmp $10, %eax # syscall number should be less than 11
    jg invalid_call
    call *syscall_jumptable(,%eax,4) # 4:for jumptable, long = 4 bytes
    jmp syscall_done

invalid_call:
    movl $-1, %eax

syscall_done:
    add $12, %esp # 12: pop the 3 parameters (3*4)

    popl %esi
    popl %ebp
    popl %edi
    popl %ebx
    popl %ecx
    popl %edx
    IRET

syscall_jumptable:
    .long 0x0
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn



