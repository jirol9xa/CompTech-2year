#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cmd_vector.h"

int ctor(Cmd_vector *vec, size_t capacity)
{
    assert(vec);
    assert(capacity > 0);

    void *temp_ptr = malloc(sizeof(Cmd*) * (capacity + !capacity));
    assert(temp_ptr);

    vec->data     = (Cmd **) temp_ptr;
    vec->size     = 0;
    vec->head     = 0;
    vec->capacity = capacity;

    return 0;
}

int resize(Cmd_vector *vec)
{
    assert(vec);

    vec->capacity = vec->capacity * 2 + !vec->capacity;

    void *temp_ptr = realloc(vec->data, vec->capacity * sizeof(Cmd *));
    assert(temp_ptr);

    vec->data = (Cmd **) temp_ptr;
    return 0;
}

int pushBack(Cmd_vector *vec, Cmd value)
{
    assert(vec);

    if (vec->size >= vec->capacity)
        resize(vec);

    Cmd *new_cmd  = newCmd();
    new_cmd->argv = value.argv;
    new_cmd->cmd  = value.cmd;

    vec->data[vec->size++] = new_cmd;

    return 0;
}

int dtor(Cmd_vector *vec)
{
    assert(vec);

    // Free whole text buff
    free(vec->data[0]->cmd);

    for (size_t i = 0; i < vec->size; ++i)
    {
        free(vec->data[i]->argv);
        free(vec->data[i]);
    }
    
    free(vec->data);
    return 0;
}
