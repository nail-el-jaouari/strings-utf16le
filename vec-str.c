#include <stdlib.h>
#include <string.h>
#include "vec-str.h"

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
    self->capacity = MIN_CAP;
}

void vec_str_push(struct VecStr *self, const char *elem)
{
    if (self->size == self->capacity)
    {
        self->capacity *= 2;
        self->data = realloc(self->data, self->capacity);
    }
    self->data[self->size] = elem;
    self->size++;
    self->total_str_length += strlen(elem);
}
