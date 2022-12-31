#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "lib.h"
#include "i8259.h"
#include "x86_desc.h"
#include "control.h"

#define KEYBOARD_PICNUM 0x01 /* corresponding PIC connection */
#define KEYBOARD_PORT 0x60 /* keyboard IO port */
#define KEYBOARD_SIZE 128

#define NUM_COLS 80     /* column number in text mode */
#define NUM_ROWS 25     /* row number in text mode */
#define LOG_ROWS 128    /* keep log for 128 line cmds */
#define ATTRIB 0x7      /* unknown, copied from lib.c */
#define CMD_MAX_LEN 128 /* maxinum cmd length */

#define K_TRUE 1
#define K_FALSE 0
#define T_SUCC 0
#define T_FAIL -1
#define MAX_BUFFER_SIZE 128
#define VID_MEM       0xB8000
#define KB            1024
#define numfour       4
#define offset        12

void keyboard_init();
void keyboard_irq();

int32_t get_command_char(uint8_t input);

void init_terminal();
int32_t open_terminal(const uint8_t* filename);
int32_t close_terminal(int32_t fd);
int32_t read_terminal(int32_t inode, uint32_t* offse, void* buf, int32_t nbytes);
int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes);

void terminal_switch(int32_t tid);
void vid_mem(int32_t tid);
void display_putc(uint8_t key);
void display_printf(int8_t* key);

#endif
