#include "cursor.h"
#define VGA_WIDTH 80

/* enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
   input: cursor_start, cursor_end(highest and lowest scanline)
   output: -
   return: -
   description: from osdev, enabling the cursor. Reverse the order in outb
*/
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
   /* Enable cursor by setting bit5(CD) of 0x0A to 0   */
   /* bit7 and bit6 does not change by using 0xC0 mask */
   /* set low 5 bits to cursor scanline start value    */
   outb(VGA_CUR_START, VGA_CRT_ADDR);                            
   outb((inb(VGA_CRT_DATA) & 0xC0) | cursor_start, VGA_CRT_DATA); // 0xC0: mask

   /* high 3 bits does not change by using 0xE0 mask */
   /* set low 5 bits to cursor scanline end value    */
   outb(VGA_CUR_END, VGA_CRT_ADDR);                            
   outb((inb(VGA_CRT_DATA) & 0xE0) | cursor_end, VGA_CRT_DATA); // 0xE0: mask
}

/* disable_cursor(void)
   input: -
   output: -
   return: -
   description: from osdev, disabling the cursor. Reverse the order in outb
*/
void disable_cursor()
{
   /* set bit5 of 0x0A CRT to 1 to disable cursor */
   outb(0x20, VGA_CRT_DATA); /* 0x20 is CD variable mask */
   outb(VGA_CUR_START, VGA_CRT_ADDR); 
}

/* update_cursor(int x, int y)
   input: x, y - the new positon of cursor
   output: -
   return: -
   description: from osdev, update the cursor pos - don't need to
   update the cursor's location every time a new character is displayed.
*/
void update_cursor(int x, int y)
{
   /* position value from matrix to index */
   uint16_t pos = y * VGA_WIDTH + x;

   /* write low 8 bit of pos into 0x0F CRT */
   outb(VGA_CUR_LOCL, VGA_CRT_ADDR);
   outb((uint8_t)( pos       & BYTE_MASK), VGA_CRT_DATA);
   /* write high 8 bit of pos into 0x0E CRT */
   outb(VGA_CUR_LOCH, VGA_CRT_ADDR);
   outb((uint8_t)((pos >> 8) & BYTE_MASK), VGA_CRT_DATA); /* 8 offset for high bits */
}

/* get_cursor_position(void)
   input: -
   output: -
   return: the current position of the cursor
   description: from osdev, update the cursor pos - don't need to
   update the cursor's location every time a new character is displayed.
*/
uint16_t get_cursor_position(void)
{
   uint16_t pos = 0;

   /* get low 8 bit of position from 0x0F CRT */
   outb(VGA_CUR_LOCL, VGA_CRT_ADDR);
   pos |= inb(VGA_CRT_DATA);
   /* get high 8 bit of position from 0x0E CRT */
   outb(VGA_CUR_LOCH, VGA_CRT_ADDR);
   pos |= ((uint16_t)inb(VGA_CRT_DATA)) << 8; /* 8 offset for high bits */

   return pos;
}
