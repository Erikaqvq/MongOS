#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "control.h"
#include "keyboard.h"
#include "syscall.h"

/* pit_init
    input: -
    output: -
    Description: enable pit works, set working mode
*/
void pit_init(){  
    /* reference: osdev */
    outb(PIT_MODE, CMD_REG);
    outb(PIT_FREQ & LOW_BYTE, DATA_PORT);
    outb((PIT_FREQ >> R_SHIFT) & LOW_BYTE, DATA_PORT);
    enable_irq(PIT_IRQ);
}

/* pit_irq
    input: -
    output: -
    Description: deal with pit interrupt, do process switch
*/
void pit_irq(){
    send_eoi(PIT_IRQ);
    int32_t tmpid = current_tid;
    tmpid = (tmpid + 1) % TERMINAL_SIZE;
    vid_mem(tmpid);
    process_switch(tmpid);
}

