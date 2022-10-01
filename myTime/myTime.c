#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

void timeDiff(struct timeval *begin,
              struct timeval *end,
              struct timeval *res)
{
    long usec = end->tv_usec;
    long sec  = end->tv_sec;

    if (usec < begin->tv_usec)
    {
        usec += 1e6;
        sec--;
    }

    usec -= begin->tv_usec;
    sec  -= begin->tv_sec;

    res->tv_sec  = sec;
    res->tv_usec = usec;
}

int main(const int argc, char *const argv[])
{        
    struct timeval  begin;
    gettimeofday(&begin, NULL);

    int pid = fork();
    if (pid == 0)
        execvp(argv[1], (argv + 1));

    int status;
    while (wait(&status) != -1)
        continue;

    if (!(WIFEXITED(status)))
    {
        printf("Error in execvp\n");
        return 0;
    }

    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval res;
    timeDiff(&begin, &end, &res);

    printf("time = %ld sec, %ld microsec\n", res.tv_sec, res.tv_usec);

    return 0;
}