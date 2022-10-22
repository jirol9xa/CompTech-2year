#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "cmd.h"
#include "cmd_vector.h"
#include "parser.h"

int main(void)
{
    Cmd_vector vector;
    ctor(&vector, 10);

    // Fill the vector with parsed cmds
    parseFile(0, &vector);

    int pid;
    int fds[2];
    int prev_out_fds = 0;
    //Now we need just process all cmds one by one
    for (size_t i = 0; i < vector.size; ++i)
    {
        pipe(fds);
        pid = fork();
        if (pid == 0)
        {
            if (i != vector.size - 1)
            {
                // Stdout for new proc is fds[1]
                if (dup2(fds[1], 1) == -1)
                {
                    perror("Dup2 error");
                    return -1;
                }
                close(fds[1]);
            }

            if (dup2(prev_out_fds, 0) == -1)
            {
                perror("Dup2 error");
                return -1;
            }
            close(prev_out_fds);        
     
            if (execvp(vector.data[i]->cmd, vector.data[i]->argv) == -1)
            {
                perror("Execvp error");
                return -1;
            }

            break;
        }
        else
        {
            prev_out_fds = fds[0];
            close(fds[1]);
        }
    }

    if (pid != 0)
    {
        int status;

        for (int i = 0; i < vector.size; ++i)
        {
            while (wait(&status) != -1)
                continue;
            close(prev_out_fds);
        }
        
        dtor(&vector);
    }
    return 0;
}
