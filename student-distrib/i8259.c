/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = INIT_MASK; /* IRQs 0-7  */
uint8_t slave_mask = INIT_MASK;  /* IRQs 8-15 */

/* i8259_init()
    input: -
    output: -
    Description: Initialize the 8259 PIC 
*/
void i8259_init(void) {
    outb(INIT_MASK, MASTER_8259_DATA);
    outb(INIT_MASK, SLAVE_8259_DATA);

    /* Reference: lecture notes */
    outb(ICW1, MASTER_8259_CMD);
    outb(ICW1,SLAVE_8259_CMD); 
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);              
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    /* finally, enable slave using enable_irq func */
    enable_irq(SLAVE_POS);
}


/* enable_irq
    input: IRQ number
    output: -
    Description: Enable (unmask) the specified IRQ
*/
void enable_irq(uint32_t irq_num) {
    int new_mask;
    if(irq_num >= 2 * TOT_CNT) return; /* out of range */
    if(irq_num >= TOT_CNT) { // 8-15: slave's IRQ num
        irq_num -= TOT_CNT; // correspongding num on SLAVE
        new_mask = ~(1 << irq_num); // only unmask the input bit
        slave_mask &= new_mask;
        outb(slave_mask, SLAVE_8259_DATA);
    } else { // master
        new_mask = ~(1 << irq_num);
        master_mask &= new_mask;
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* enable_irq
    input: IRQ number
    output: -
    Description: Disable (mask) the specified IRQ
*/
void disable_irq(uint32_t irq_num) {
    int new_mask;
    if(irq_num >= 2 * TOT_CNT) return;  /* out of range */
    if(irq_num >= TOT_CNT) { // slave
        irq_num -= TOT_CNT; // correspongding num on SLAVE
        new_mask = 1 << irq_num; 
        slave_mask |= new_mask; 
        outb(slave_mask, SLAVE_8259_DATA);
    } else { // master
        new_mask = 1 << irq_num; 
        master_mask |= new_mask;
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* send_eoi
    input: IRQ number
    output: -
    Description: Send end-of-interrupt signal for the specified IRQ
*/
void send_eoi(uint32_t irq_num) {
    int op_s;
    if(irq_num >= 2 * TOT_CNT) return;  /* out of range */
    op_s = EOI;
    if(irq_num >= TOT_CNT) { // slave
        op_s |= irq_num - TOT_CNT; // correspongding num on SLAVE
        outb(op_s, SLAVE_8259_CMD);
        op_s = EOI;
        op_s |= SLAVE_POS;
        outb(op_s, MASTER_8259_CMD);
    } else { // master
        op_s |= irq_num;
        outb(op_s, MASTER_8259_CMD);
    }
}
