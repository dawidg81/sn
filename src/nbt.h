#ifndef NBT_H
#define NBT_H

#define NBT_TAG_END     0x00
#define NBT_TAG_BYTE    0x01
#define NBT_TAG_SHORT   0x02
#define NBT_TAG_INT     0x03
#define NBT_TAG_LONG    0x04
#define NBT_TAG_FLOAT   0x05
#define NBT_TAG_DOUBLE  0x06
#define NBT_TAG_BYTE_ARRAY  0x07
#define NBT_TAG_STRING  0x08
#define NBT_TAG_LIST    0x09
#define NBT_TAG_COMPOUND    0x0A
#define NBT_TAG_INT_ARRAY   0x0B
#define NBT_TAG_LONG_ARRAY  0x0C

#include <stdio.h>

int nbt_write(FILE* file)

#endif
