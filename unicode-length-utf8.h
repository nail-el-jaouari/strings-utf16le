#ifndef __UNICODE_LENGTH_UTF8__
#define __UNICODE_LENGTH_UTF8__

#include <stddef.h>
/*
    Calculate the number of valid unicode code points in a
    utf8 string.
*/
size_t unicode_length_u8string(const unsigned char *u8string);

#endif