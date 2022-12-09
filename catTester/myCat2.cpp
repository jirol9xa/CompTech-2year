#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

const int BUFF_SIZE = 4096;
#define DEBUG_MODE 0


#define PRINT_LINE                                      \
{                                                       \
  if (DEBUG_MODE)                                       \
    fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);   \
}

#define IS_VALID(param) {                   \
    if (!param)                             \
    {                                       \
        printf("Invalid " #param "ptr ");   \
        return -1;                          \
    }                                       \
}

static int safeWrite(int out_descr, const char *buff, const size_t buff_size)
{
    IS_VALID(buff);

    PRINT_LINE;
    /*for (int i = 0; i < buff_size; ++i)
    {
      fprintf(stderr, "%c", buff[i]);
    }
    fprintf(stderr, "\n");*/

    int wrtn = 0;


    PRINT_LINE;
    while(wrtn != buff_size)
    {
        wrtn += write(out_descr, buff + wrtn, buff_size - wrtn);
        PRINT_LINE;
    }

    PRINT_LINE;
    return 0;
}

static int myCat(const int in_descr, const int out_descr)
{
    if (in_descr < 0)
    {
        perror("Can't open file");
        return -1;
    }

    char buff[BUFF_SIZE];
    int  buff_size = BUFF_SIZE;

    while (buff_size > 0)
    {
        PRINT_LINE;
        buff_size = read(in_descr, buff, BUFF_SIZE);

        if (buff_size < 0)
        {
            perror("Can't read from istream\n");
            return -1;
        }
        
        if (safeWrite(out_descr, buff, buff_size))
        {
           printf("Can't write whole buff\n");
           return -1;
        }

        PRINT_LINE;
    }
   
    PRINT_LINE;

    return 0;
}


int main(const int argc, const char *argv[])
{
    int in_descr  = 0,
        out_descr = 1;

    if (argc < 2)
    {
        myCat(in_descr, out_descr);
        return 0;
    }
    if (myCat(in_descr, out_descr))
    {
      printf("Error in myCat\n");
    }
    
    close(in_descr);
    close(out_descr);

    return 0;
}
