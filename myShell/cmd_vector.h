#pragma once

#include "cmd.h"

typedef struct
{
    Cmd  **data;
    size_t head;
    size_t size;
    size_t capacity;
} Cmd_vector;

int  ctor    (Cmd_vector *vec, size_t capacity);
int  dtor    (Cmd_vector *vec);
int  pushBack(Cmd_vector *vec, Cmd value);
Cmd *popFront(Cmd_vector *vec);
