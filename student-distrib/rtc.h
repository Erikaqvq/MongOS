#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "ops.h"


#define RTC_IND 0x70 /* Port 0x70 is used to specify an index or "register number" */
#define RTC_CMOS 0x71 /* ort 0x71 is used to read or write from/to that byte of CMOS configuration space */
#define RTC_PICNUM 0x08 /* corresponding pos on PIC */
#define RTC_REGA 0x0A /* reference: osdev */
#define RTC_REGB 0x0B /* reference: osdev */
#define RTC_REGC 0x0C /* reference: osdev */
/* When programming the RTC, it is extremely imperative that the NMI and other interrupts are disabled. */
#define NMI_VAL 0x80 /* reference: osdev */
#define MAX_FREQ        1024     
#define MIN_FREQ        2
#define RTC_nbytes_write 4



/* initialize RTC and set default frequency to 2 */
void RTC_init();               
void RTC_irq();

/* set rtc (v)frequency */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
/* this function is blocked until counter reaches MAX_count */
int32_t rtc_read(int32_t inode, uint32_t* offst, void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);



#endif

