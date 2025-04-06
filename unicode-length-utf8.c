#include "unicode-length-utf8.h"

size_t unicode_length_u8string(const char *u8string)
{
    size_t length = 0;

    while (*u8string != '\0')
    {
        const unsigned char c = (const unsigned char)*u8string;

        if (c <= 0x7F)
        {
            length++;            
            u8string++;
        }
        else if (c >= 0xC0 && c <= 0xDF)
        {
            length++;
            u8string += 2;
        }
        else if (c >= 0xE0 && c <= 0xEF)
        {
            length++;
            u8string += 3;
        }
        else if (c >= 0xF0 && c <= 0xF7)
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