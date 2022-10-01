#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(const int argc, char * const argv[])
{
    int number, pid, curr_num, status;

    for (int i = 1; i < argc; ++i)
    {
        number = atoi(argv[i]);
        pid = fork();    
        
        if (!pid)
            break;
    }

    usleep(10000 * number);
    if (!pid)
    {
        printf("%d ", number);
        fflush(stdout);
        _exit(0);
    }

    while (wait(&status) != -1)
        continue;

    printf("\n");

    return 0;
}