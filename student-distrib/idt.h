#ifndef _IDT_H
#define _IDT_H


#include "x86_desc.h"
#include "types.h"
#include "interrupt.h"
#include "sys_w.h"
#include "keyboard.h"
#include "rtc.h"

#define DEVICE_PIT      0x20
#define DEVICE_KEYBOARD 0x21
#define DEVICE_RTC      0x28
#define VECTOR_SYSTEM_CALL 0x80

/*

function SET_IDT_ENTRY is at x86_desc.h
SET_IDT_ENTRY(str, handler) means set str = idt[x]'s handler function = handler

*/
char* err_msg[NUM_VEC];

//void system_call();
void msg_init();
void err_handle(uint8_t err_n);
void idt_init();


#endif

