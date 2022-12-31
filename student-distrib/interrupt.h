#ifndef _INTERRUPT_H
#define _INTERRUPT_H


/*
    interrupt.h/.S are used to wrap keyboard and RTC
*/
#ifndef ASM
    extern void irt_kb();
    extern void irt_rtc();
    extern void irt_pit();
#endif

#endif
