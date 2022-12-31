#ifndef _OPS_H
#define _OPS_H

#include "types.h"
#include "keyboard.h"

typedef struct ops_t{
    int32_t (*open)(const uint8_t*);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, uint32_t*, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
} ops_t;



#endif
