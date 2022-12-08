#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

const int BUFF_SIZE = 4096;

#define PRINT_LINE fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);

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
        fprintf(stderr, "wrtn to buff in cat = %d\n", wrtn);
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

    while (buff_size == BUFF_SIZE)
    {
        PRINT_LINE;
        buff_size = read(in_descr, buff, BUFF_SIZE);
        fprintf(stderr, "read to buff in myCat = %d\n", buff_size);

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
    
    close(in_descr);
    return 0;
}


int main(const int argc, const char *argv[])
{
    int in_descr = 0;

    if (argc < 2)
    {
        myCat(in_descr, 1);
        return 0;
    }

    //for (int i = 1; i < argc; ++i)
    //{   
        //in_descr = open(argv[i], O_ASYNC);

        if (myCat(in_descr, 1))
        {
            printf("Error in myCat\n");
     //       break;
        }
    //}
    
    return 0;
}
