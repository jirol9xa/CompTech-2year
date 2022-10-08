#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>

const int BUFF_SIZE = 4096;

typedef struct Buffer
{
    char buff[BUFF_SIZE];
    unsigned buff_size;
} Buffer;

int countElems(int *Str_amnt, int *Wrd_amnt, struct Buffer *buffer)
{
    if (!Str_amnt || !Wrd_amnt)
    {
        fprintf(stderr, "Nullptr int countElems\n");
        return -1;
    }

    int is_word = 0, str_amnt = 0, wrd_amnt = 0;
    char *buff = buffer->buff;
    for (int i = 0; i < buffer->buff_size; ++i)
    {
        if (!isspace(buff[i]))
        {
            wrd_amnt += (is_word == 0);
            is_word = 1;
            continue;
        }

        is_word = 0;
        str_amnt += (buff[i] == '\n');
    }

    *Wrd_amnt = wrd_amnt;
    *Str_amnt = str_amnt;

    return 0;
}

int main(const int argc, char *const argv[])
{        
    int fds[2];
    pipe(fds);

    int pid = fork();
    if (pid == 0)
    {
        // Перепривязать stdout к stdin родительского процесса 
        if (dup2(fds[1], 1) == -1)
        {
            perror("Dup2 error");
            return -1;
        }
        execvp(argv[1], (argv + 1));
    }
    else
    {
        if (dup2(fds[0], 0) == -1)
        {
            perror("Dup2 error");
            return -1;
        }
    }

    int status;
    while (wait(&status) != -1)
        continue;

    if (pid != 0)
    {
        Buffer buffer;
        if ((buffer.buff_size = read(0, buffer.buff, BUFF_SIZE)) < 0)
        {
            perror("Read error");
            return -1;
        }

        int str_amnt, wrd_amnt, bytes_amnt;
        if (countElems(&str_amnt, &wrd_amnt, &buffer) == -1)
        {
            fprintf(stderr, "Error!\n");
            return -1;
        }

        printf("%d\t%d\t%d\n", str_amnt, wrd_amnt, buffer.buff_size);
    }



    return 0;
}