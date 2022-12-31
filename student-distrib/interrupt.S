#define ASM 1

#include "interrupt.h"

#define INTERRUPT_WRAP(tagg, f)    \
    .globl tagg                   ;\
    tagg:                         ;\
        pushal                    ;\
        pushfl                    ;\
        call f                    ;\
        popfl                     ;\
        popal                     ;\
        iret
        

INTERRUPT_WRAP(irt_kb, keyboard_irq);
INTERRUPT_WRAP(irt_rtc, RTC_irq);
INTERRUPT_WRAP(irt_pit, pit_irq);
