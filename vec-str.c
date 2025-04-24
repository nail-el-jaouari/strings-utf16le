#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec-str.h"
#include "unicode-length-utf8.h"

void vec_str_init(struct VecStr *self, size_t capacity)
{
    self->data = malloc(capacity * sizeof *self->data);
    self->size             = 0;
    self->total_str_length = 0;

    if (self->data == NULL)
    {
        self->capacity         = 0;    
    }
    else
    {
        self->capacity = capacity;
    }
}

void vec_str_free(struct VecStr *self)
{
    for (size_t i = 0; i < self->size; i++)
    {
        free(self->data[i]);
    }

    free(self->data);

    self->data = NULL;
    self->size = 0;
    self->total_str_length = 0;
    self->capacity = 0;
}

void vec_str_push(struct VecStr *self, const char *elem)
{
    if (self->size == self->capacity)
    {
        self->capacity *= 2;
        self->data = reallocarray(self->data, self->capacity, sizeof *self->data);
    }

    self->data[self->size] = (char *)elem;
    self->size++;
    //self->total_str_length += strlen(elem);
    self->total_str_length += unicode_length_u8string(elem);
}

void vec_str_print(const struct VecStr *self)
{
    for (size_t i = 0; i < self->size; i++)
    {
        printf("%s", self->data[i]);
    }
    printf("\n");
}
