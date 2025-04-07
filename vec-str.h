#ifndef __VEC_STR_H__
#define __VEC_STR_H__

#include <stddef.h>

struct VecStr
{
    char **data;
    size_t size;
    size_t capacity;
    size_t total_str_length;
};

void vec_str_init(struct VecStr *self, size_t capacity);
void vec_str_free(struct VecStr *self);
void vec_str_push(struct VecStr *self, const char *elem);
#endif
