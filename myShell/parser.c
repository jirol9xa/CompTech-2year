#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "parser.h"

static size_t skipSpace(char *buff)
{
    size_t offset = 0;
    while (isspace(buff[offset]))
    {
        buff[offset] = '\0';
        ++offset;
    }
    return offset;
}

static size_t skipWord(char *buff)
{
    size_t offset = 0;
    while (isalnum(buff[offset]) || buff[offset] == '-')
        ++offset;

    return offset;
}

static int parseCmd(Cmd *cmd, char *buff)
{
    assert(cmd);
    assert(buff);
    
    size_t offset = skipSpace(buff);
    cmd->argv = (char **) malloc(10 * sizeof(char *));
    // We can't get more, than 10 args for one cmd, that's sad, but true

    cmd->cmd     = buff + offset;
    cmd->argv[0] = cmd->cmd;
    offset      += skipWord(buff + offset);
    offset      += skipSpace(buff + offset);

    for (int i = 1; i < 10 && buff[offset] != '\0'; ++i)
    {
        cmd->argv[i] = buff + offset;

        offset += skipWord(buff + offset);
        offset += skipSpace(buff + offset);
    }

    return 0;
}

int parseFile(const int in_decr, Cmd_vector *vec)
{
    assert(vec);

    char *buff     = (char *) malloc(BUFF_SIZE * sizeof(char));
    int  buff_size = read(in_decr, buff, BUFF_SIZE); 
    if (buff_size < 0)
    {
        perror("Can't read from istream\n");
        return -1;
    }
    buff[buff_size - 1] = '\0';

    Cmd *cmd       = newCmd();
    char *curr_pos = strtok(buff, "|");

    do
    {
        parseCmd(cmd, curr_pos);
        pushBack(vec, *cmd);
        
        curr_pos = strtok(NULL, "|");
        if (!curr_pos)
            break;
    } 
    while (1);

    free(cmd);
    return 0;
}
