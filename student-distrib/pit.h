#ifndef _PIT_H
#define _PIT_H

/* reference: osdev */
#define PIT_IRQ      0x0
#define CMD_REG      0x43
#define DATA_PORT    0x40
#define PIT_MODE     0x37
#define PIT_FREQ     11932 // 1.193182 MHz
#define LOW_BYTE     0xFF
#define R_SHIFT      8

void pit_init();
void pit_irq();

#endif
