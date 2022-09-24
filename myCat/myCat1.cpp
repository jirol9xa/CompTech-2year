#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const int BUFF_SIZE = 4096;


void myCat(int in_desrc, int out_descr)
{
    char buff[BUFF_SIZE];
    int buff_size = BUFF_SIZE;

    while (buff_size > 0)
    {
        buff_size = read(in_desrc, buff, BUFF_SIZE);
        if (buff_size < 0)
        {
            perror("Can't read from stdin");

            _exit(true);
        }
        write(out_descr, buff, buff_size);
    }

    return;
}

int main()
{
    myCat(0, 1);

    return 0;
}