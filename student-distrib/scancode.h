#ifndef _SCANCODE_H
#define _SCANCODE_H

#define TYPED_MAX 0x100 /* only use part of it for characters */
#define CHARACTER_MAX 0x59 /* larger than 0x58 are all release signals */

#define keyL 0x26 /* key value of 'L' */

 /* https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html */
 /* https://wiki.osdev.org/PS/2_Keyboard */

unsigned char scancode_table[CHARACTER_MAX][2] = {
   {   0,0   } , //0x00             Error
   {   0,0   } , //0x01    press    Esc         
   { '1','!' } , //0x02    press    '1'           
   { '2','@' } , //0x03    press    '2'           
   { '3','#' } , //0x04    press    '3'           
   { '4','$' } , //0x05    press    '4'           
   { '5','%' } , //0x06    press    '5'           
   { '6','^' } , //0x07    press    '6'           
   { '7','&' } , //0x08    press    '7'           
   { '8','*' } , //0x09    press    '8'           
   { '9','(' } , //0x0A    press    '9'           
   { '0',')' } , //0x0B    press    '0'           
   { '-','_' } , //0x0C    press    '-'           
   { '=','+' } , //0x0D    press    '='           
   {   8,8   } , //0x0E    press    Backspace   
   {   9,9   } , //0x0F    press    Tab         
   { 'q','Q' } , //0x10    press    'Q'           
   { 'w','W' } , //0x11    press    'W'           
   { 'e','E' } , //0x12    press    'E'           
   { 'r','R' } , //0x13    press    'R'           
   { 't','T' } , //0x14    press    'T'           
   { 'y','Y' } , //0x15    press    'Y'           
   { 'u','U' } , //0x16    press    'U'           
   { 'i','I' } , //0x17    press    'I'           
   { 'o','O' } , //0x18    press    'O'           
   { 'p','P' } , //0x19    press    'P'           
   { '[','{' } , //0x1A    press    '['           
   { ']','}' } , //0x1B    press    ']'           
   {  13,13  } , //0x1C    press    Enter
   {   0,0   } , //0x1D    press    Ctrl(left)
   { 'a','A' } , //0x1E    press    'A'
   { 's','S' } , //0x1F    press    'S'
   { 'd','D' } , //0x20    press    'D'
   { 'f','F' } , //0x21    press    'F'
   { 'g','G' } , //0x22    press    'G'
   { 'h','H' } , //0x23    press    'H'
   { 'j','J' } , //0x24    press    'J'
   { 'k','K' } , //0x25    press    'K'
   { 'l','L' } , //0x26    press    'L'
   { ';',':' } , //0x27    press    ';'
   {  39,34  } , //0x28    press    '''
   { '`','~' } , //0x29    press    '`'
   {   0,0   } , //0x2A    press    Shift(left)
   { '\\','|'} , //0x2B    press    '\'
   { 'z','Z' } , //0x2C    press    'Z'
   { 'x','X' } , //0x2D    press    'X'
   { 'c','C' } , //0x2E    press    'C'
   { 'v','V' } , //0x2F    press    'V'
   { 'b','B' } , //0x30    press    'B'
   { 'n','N' } , //0x31    press    'N'
   { 'm','M' } , //0x32    press    'M'
   { ',','<' } , //0x33    press    ','
   { '.','>' } , //0x34    press    '.'
   { '/','?' } , //0x35    press    '/'
   {   0,0   } , //0x36    press    Shift(right)
   {   0,0   } , //0x37    press    Keypad '*'
   {   0,0   } , //0x38    press    Alt(left)
   { ' ',' ' } , //0x39    press    Space
   {   0,0   } , //0x3A    press    CapsLock
   {   0,0   } , //0x3C    press    F1
   {   0,0   } , //0x3D    press    F2
   {   0,0   } , //0x3E    press    F3
   {   0,0   } , //0x3B    press    F4
   {   0,0   } , //0x3F    press    F5
   {   0,0   } , //0x40    press    F6
   {   0,0   } , //0x41    press    F7
   {   0,0   } , //0x42    press    F8
   {   0,0   } , //0x43    press    F9
   {   0,0   } , //0x44    press    F10
   {   0,0   } , //0x45    press    NumLock
   {   0,0   } , //0x46    press    ScrollLock
   { '7',0   } , //0x47    press    Keypad '7'
   { '8',0   } , //0x48    press    Keypad '8'
   { '9',0   } , //0x49    press    Keypad '9'
   { '-','-' } , //0x4A    press    Keypad '-'
   { '4',0   } , //0x4B    press    Keypad '4'
   { '5',0   } , //0x4C    press    Keypad '5'
   { '6',0   } , //0x4D    press    Keypad '6'
   { '+','+' } , //0x4E    press    Keypad '+'
   { '1',0   } , //0x4F    press    Keypad '1'
   { '2',0   } , //0x50    press    Keypad '2'
   { '3',0   } , //0x51    press    Keypad '3'
   { '0',0   } , //0x52    press    Keypad '0'
   { '.','.' } , //0x53    press    Keypad '.'
   {   0,0   } , //0x54    Reserved
   {   0,0   } , //0x55    Reserved
   {   0,0   } , //0x56    Reserved
   {   0,0   } , //0x57    press    F11
   {   0,0   } , //0x58    press    F12

};

#endif
