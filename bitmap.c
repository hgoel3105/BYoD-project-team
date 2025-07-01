#include<stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "page.h"

int set_bit(uint8_t *bitmap, int i) 
{
    if (i < 0 || i >= bitmap_size * 8) 
    {
        printf("Index out of bounds for bitmap.\n");
        return -1;
    }
    bitmap[i / 8] |= (1 << (i % 8));
    return 0;
}

int remove_bit(uint8_t *bitmap, int i) 
{
    if (i < 0 || i >= bitmap_size * 8) 
    {
        printf("Index out of bounds for bitmap.\n");
        return -1;
    }
    bitmap[i / 8] &= ~(1 << (i % 8));
    return 0;
}

int check_bit(uint8_t *bitmap, int i) 
{
    if (i < 0 || i >= bitmap_size * 8) 
    {
        printf("Index out of bounds for bitmap.\n");
        return -1; 
    }
    return (bitmap[i / 8] & (1 << (i % 8))) ;
}