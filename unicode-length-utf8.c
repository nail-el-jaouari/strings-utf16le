#include "unicode-length-utf8.h"

size_t unicode_length_u8string(const unsigned char *u8string)
{
    size_t length = 0;

    while (*u8string != '\0')
    {
        if (*u8string <= 0x7F)
        {
            length++;            
            u8string++;
        }
        else if (*u8string >= 0xC0 && *u8string <= 0xDF)
        {
            length++;
            u8string += 2;
        }
        else if (*u8string >= 0xE0 && *u8string <= 0xEF)
        {
            length++;
            u8string += 3;
        }
        else if (*u8string >= 0xF0 && *u8string <= 0xF7)
        {
            length++;
            u8string += 4;
        }
        else
        {
            u8string++;
        }
    }

    return length;
}