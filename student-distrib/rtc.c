#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#include "syscall.h"
#include "control.h"



/* RTC_init()
    input: -
    output: -
    Description: You will have to enable interrupt generating mode,and set the init frequency
You will have to select registers(CMD port),send data to registers with data port
*/
void RTC_init(){
    /* When programming the RTC, it is extremely imperative that the NMI and other interrupts are disabled. */
    /* first initialize the registers https://wiki.osdev.org/RTC */
    outb(RTC_REGB|NMI_VAL, RTC_IND);  // select register B, and disable NMI
    char prev = inb(RTC_CMOS);	      // read the current value of register B
    outb(RTC_REGB|NMI_VAL, RTC_IND);  // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_CMOS);      // write the previous value ORed with 0x40. This turns on bit 6 of register B

    /* see https://wiki.osdev.org/RTC#Changing_Interrupt_Rate */
    outb(RTC_REGA | NMI_VAL, RTC_IND);  // set index to register A, disable NMI
    prev=inb(RTC_CMOS);	// get initial value of register A
    outb(RTC_REGA | NMI_VAL, RTC_IND);		// reset index to A
    outb((prev & 0xF0) | 0x06, RTC_CMOS); //write only our rate to A. Note, rate is the bottom 4 bits. 0x06 means 1024hz
    enable_irq(RTC_PICNUM);
}


/*  
    int32_t rtc_write
    input: int32_t fd, const void* buf, int32_t nbytes
    output: -1 for fail
             0 for success
    Description: set the frequency of virtualized RTC
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t pid = multi_terminal[current_tid].running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    // sanity checks
    if(buf == NULL || (nbytes != sizeof(int32_t))) {
        return -1;
    }
    int32_t set_freq;

    set_freq = *((int *) buf);

    // set to boundary frequency if out of boundary
    if(set_freq > MAX_FREQ) {
        set_freq = MAX_FREQ;
    }
    if(set_freq < MIN_FREQ) {
        set_freq = MIN_FREQ;
    }

    // enter critical section and set parameters
    cur_pcb->rtc_maxcount = MAX_FREQ / set_freq;
    cur_pcb->rtc_counter = 0;

    return RTC_nbytes_write;
}

/*  
    int32_t rtc_open
    input: const uint8_t* filename
    output: 0
    Description:this funtion does not do anything
*/
int32_t rtc_open(const uint8_t* filename) {
    int32_t pid = multi_terminal[current_tid].running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    uint32_t default_freq = MIN_FREQ;  
    cur_pcb->rtc_counter = 0;
    cur_pcb->rtc_maxcount = MAX_FREQ / default_freq;
    cur_pcb->rtc_trigger = 0;
    return 0; // success
}

/*  
    int32_t rtc_close
    input: int32_t fd
    output: 0
    Description:this funtion does not do anything
*/
int32_t rtc_close(int32_t fd) {
    int32_t pid = multi_terminal[current_tid].running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    cur_pcb->rtc_counter = 0;
    cur_pcb->rtc_maxcount = 0;
    cur_pcb->rtc_trigger = 0;
    return 0;
}

/*  
    int32_t rtc_read
    input: int32_t fd, void* buf, int32_t nbytes
    output: 0
    Description:this funtion is blocked until rtc_counter reaches MAX_COUNT, at this point, rtc_trigger is set to 1 and this function returns
*/
int32_t rtc_read(int32_t inode, uint32_t* offst, void* buf, int32_t nbytes) {
    int32_t pid = multi_terminal[current_tid].running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    cli();
    cur_pcb->rtc_trigger = 0;
    sti();
    while(cur_pcb->rtc_trigger == 0); // wait for signal from interrupt
    return 0;
}


/* RTC_irq()
    input: -
    output: -
    Description: set rtc_trigger to 1 and reset rtc_trigger when rtc_trigger reaches MAX_COUNT
*/
void RTC_irq(){
   int i;
   Pcb_t* cur_pcb;
   for(i = 1; i <= MAX_PROCESS; i++){
    cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * i);
    if(cur_pcb->rtc_maxcount == 0) continue; // no rtc in this process
    cur_pcb->rtc_counter++;
    if(cur_pcb->rtc_counter >= cur_pcb->rtc_maxcount){
        cli();
        cur_pcb->rtc_trigger = 1; // wakeup
        cur_pcb->rtc_counter = 0;
        sti();
    }
   }
    outb(RTC_REGC| NMI_VAL,RTC_IND);	// select register C  ******bug here(see buglog)******
    inb(RTC_CMOS);		       // just throw away contents
    /* Send EOl-PIC will not handle another interrupt until then */
    send_eoi(RTC_PICNUM);
}


