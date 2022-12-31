#include "idt.h"
#include "syscall.h"

#define EXCEPTION 0xFF
/* Generalized Exception handling(more elegant) */
#define IDT_FUNCS(ERR_NAME, ERR_NUM)    \
void ERR_NAME(){                        \
    err_handle(ERR_NUM);                \
} 

/* err_handle(uint8_t err_n)
    input: error number
    output: -
    Description: when occuring Exception, call this function to specify the problem
*/
void err_handle(uint8_t err_n){
   /* design error report message here */
    cli();
	//clear();
	printf("  num=%d, error = %s\n", err_n, err_msg[err_n]);
	//asm volatile(".lp:hlt; jmp .lp;"); /* infinate loop */
	halt(EXCEPTION);

}

// according to ia32 manual vol3 5.14
/* use IDT_FUNCS to declare Exception handler functions */
IDT_FUNCS(divide_error_exception,                0x00);
IDT_FUNCS(debug_exception,                       0x01);
IDT_FUNCS(NMI_interrupt_exception,               0x02);
IDT_FUNCS(breakpoint_exception,                  0x03);
IDT_FUNCS(overflow_exception,                    0x04);
IDT_FUNCS(bound_range_exception,                 0x05);
IDT_FUNCS(invalid_opcode_exception,              0x06);
IDT_FUNCS(device_not_available_exception,        0x07);
IDT_FUNCS(double_fault_exception,                0x08);
IDT_FUNCS(coprocessor_segment_overrun_exception, 0x09);
IDT_FUNCS(invalid_TSS_exception,                 0x0A);
IDT_FUNCS(segment_not_present_exception,         0x0B);
IDT_FUNCS(stack_fault_exception,                 0x0C);
IDT_FUNCS(general_protection_exception,          0x0D);
IDT_FUNCS(page_fault_exception,                  0x0E);// 15 is reserved 
IDT_FUNCS(x87_FPU_error_exception,               0x10);
IDT_FUNCS(alignment_check_exception,             0x11);
IDT_FUNCS(machine_check_exception,               0x12);
IDT_FUNCS(SIMD_floating_point_exception,         0x13);

/*  msg_init()
    input: -
    output: -
    Description: set message string for each Exceptions
*/
void msg_init(){
    /* follow the idt table, initialize them (fill the msg table: num->string) */
    err_msg[0]="divide by zero";
    err_msg[1]="debug";
    err_msg[2]="NMI_interrupt";
    err_msg[3]="breakpoint";
    err_msg[4]="overflow";
    err_msg[5]="BOUND Range Error";
    err_msg[6]="Invalid Opcode";
    err_msg[7]="Device Not Available";
    err_msg[8]="Double Fault";
    err_msg[9]="Coprocessor Segment Overrun";
    err_msg[10]="DInvalid TSS";
    err_msg[11]="Segment Not Present";
    err_msg[12]="Stack Fault";
    err_msg[13]="General Protection";
    err_msg[14]="Page-Fault";
    err_msg[15]="Reserved";
    err_msg[16]="x87 FPU Floating-Point Error";
    err_msg[17]="Alignment Check";
    err_msg[18]="Machine-Check";
    err_msg[19]="SIMD Floating-Point";
}


/*  idt_init()
    input: -
    output: -
    Description: initialize idt arguments and call SET_IDT_ENTRY to make a connection
*/
void idt_init() {
	msg_init();
    int i;
    for(i = 0; i < NUM_VEC; i++){
        /* first initialize all the variables in idt structure */ 
        // acording to https://wiki.osdev.org/IDT and ia32 manual vol3
        idt[i].present=1; // must to be set to 1
        if (i == 0x80){  // 0x80 is system call
            idt[i].dpl = 3; // for system call, need to set dpl to 3(from instruction appendix d)
        }
        else{
            idt[i].dpl = 0; //  for other situations, set dp1 to 0
        }
        idt[i].reserved0 = 0; // must to set to 0
        idt[i].size = 1; // set each handler to 32 bits
        // determine gate type here
        //110 for 32bits-interrupt; 111 for 32bits-trap
        idt[i].reserved1=1;
        idt[i].reserved2=1;
        if ((i >= 0x20) && (i <= 0x2F)){ // 0x20-0x2F are for interrupts
            idt[i].reserved3=0;
        }
        else{
            idt[i].reserved3=1;
        }
        idt[i].reserved4=0; //have to set to 0
        idt[i].seg_selector = KERNEL_CS; //kernel code mode   
    }

   /* use SET_IDT_ENTRY to link idt[] and handler functions */
    SET_IDT_ENTRY(idt[0],divide_error_exception);
    SET_IDT_ENTRY(idt[1],debug_exception);
    SET_IDT_ENTRY(idt[2],NMI_interrupt_exception);
    SET_IDT_ENTRY(idt[3],breakpoint_exception);
    SET_IDT_ENTRY(idt[4],overflow_exception);
    SET_IDT_ENTRY(idt[5],bound_range_exception);
    SET_IDT_ENTRY(idt[6],invalid_opcode_exception);
    SET_IDT_ENTRY(idt[7],device_not_available_exception);
    SET_IDT_ENTRY(idt[8],double_fault_exception);
    SET_IDT_ENTRY(idt[9],coprocessor_segment_overrun_exception);
    SET_IDT_ENTRY(idt[10],invalid_TSS_exception);
    SET_IDT_ENTRY(idt[11],segment_not_present_exception);
    SET_IDT_ENTRY(idt[12],stack_fault_exception);
    SET_IDT_ENTRY(idt[13],general_protection_exception);
    SET_IDT_ENTRY(idt[14],page_fault_exception);  //15 is reserved
    SET_IDT_ENTRY(idt[16],x87_FPU_error_exception);
    SET_IDT_ENTRY(idt[17],alignment_check_exception);
    SET_IDT_ENTRY(idt[18],machine_check_exception);
    SET_IDT_ENTRY(idt[19],SIMD_floating_point_exception);

    /* also keyboard and RTC */
    SET_IDT_ENTRY(idt[DEVICE_PIT], irt_pit);
    SET_IDT_ENTRY(idt[DEVICE_KEYBOARD], irt_kb);
    SET_IDT_ENTRY(idt[DEVICE_RTC], irt_rtc);
	
	/* add system call */
	SET_IDT_ENTRY(idt[VECTOR_SYSTEM_CALL],syscall_w); // 0x80 is system call
    idt[VECTOR_SYSTEM_CALL].dpl = 3;
}
