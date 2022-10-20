#pragma once

typedef struct
{
    char   *cmd;
    char **argv;
} Cmd;

Cmd *newCmd(void);
