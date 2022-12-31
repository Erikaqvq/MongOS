#ifndef _CURSOR_H
#define _CURSOR_H

#include "lib.h"

/* reference : https://wiki.osdev.org/Text_Mode_Cursor */
/* reference : http://www.osdever.net/FreeVGA/vga/textcur.htm */

/*
Cursor Start Register (Index 0Ah)
  ┏━━━┳━━━┳━━━━━┳━━━━━┳━━━━━┳━━━━━┳━━━━━┳━━━━━┓
  ┃ 7 ┃ 6 ┃  5  ┃  4  ┃  3  ┃  2  ┃  1  ┃  0  ┃
  ┣━━━╋━━━╋━━━━━╋━━━━━┻━━━━━┻━━━━━┻━━━━━┻━━━━━┫
  ┃   ┃   ┃  CD ┃    Cursor Scan Line Start   ┃
  ┗━━━┻━━━┻━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
    CD - 0 Cursor Enabled
         1 Cursor Disabled

Cursor End Register (Index 0Bh)
  ┏━━━┳━━━━━━━┳━━━━━━━┳━━━━━┳━━━━━┳━━━━━┳━━━━━┳━━━━━┓
  ┃ 7 ┃   6   ┃   5   ┃  4  ┃  3  ┃  2  ┃  1  ┃  0  ┃
  ┣━━━╋━━━━━━━┻━━━━━━━╋━━━━━┻━━━━━┻━━━━━┻━━━━━┻━━━━━┫
  ┃   ┃  Cursor Skew  ┃     Cursor Scan Line End    ┃
  ┗━━━┻━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

Cursor Location High Register (Index 0Eh)
  ┏━━━┳━━━┳━━━┳━━━┳━━━┳━━━┳━━━┳━━━┓
  ┃ 7 ┃ 6 ┃ 5 ┃ 4 ┃ 3 ┃ 2 ┃ 1 ┃ 0 ┃
  ┣━━━┻━━━┻━━━┻━━━┻━━━┻━━━┻━━━┻━━━┫
  ┃  Cursor Location High 8 bit   ┃
  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

Cursor Location Low Register (Index 0Fh)
  ┏━━━┳━━━┳━━━┳━━━┳━━━┳━━━┳━━━┳━━━┓
  ┃ 7 ┃ 6 ┃ 5 ┃ 4 ┃ 3 ┃ 2 ┃ 1 ┃ 0 ┃
  ┣━━━┻━━━┻━━━┻━━━┻━━━┻━━━┻━━━┻━━━┫
  ┃   Cursor Location Low 8 bit   ┃
  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
*/

/* VGA Control Register portals*/
#define VGA_CRT_ADDR  0x3D4
#define VGA_CRT_DATA  0x3D5

/* VGA Control Register Index */
#define VGA_CUR_START 0x0A  /* Cursor Start Register */
#define VGA_CUR_END   0x0B  /* Cursor End Register */
#define VGA_CUR_LOCH  0x0E  /* Cursor Location High Register */
#define VGA_CUR_LOCL  0x0F  /* Cursor Location Low Register */

/* masks */
#define BYTE_MASK 0xFF /* mask out one byte */

/* Enabling the Cursor:
Enabling the cursor also allows you to set the start and end scanlines,
the rows where the cursor starts and ends.
The highest scanline is 0 and the lowest scanline is the maximum scanline (usually 15). */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
/* Disabling the Cursor, also from OSDEV */
void disable_cursor();
/* Moving the Cursor, also from OSDEV */
void update_cursor(int x, int y);
/* Get Cursor Position, also from OSDEV */
uint16_t get_cursor_position(void);

#endif
