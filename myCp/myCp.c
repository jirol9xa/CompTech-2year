#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

const int BUFF_SIZE = 4096;

typedef struct
{
  unsigned v : 1;
  unsigned f : 1;
  unsigned i : 1;
} Options;

#define PRINT_LINE fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);

#define IS_VALID(param) {                 \
  if (!param)                             \
  {                                       \
    printf("Invalid " #param "ptr ");     \
    return -1;                            \
  }                                       \
}

static int safeWrite(int out_descr, const char *buff, const size_t buff_size)
{
  IS_VALID(buff);

  PRINT_LINE;

  int wrtn = write(out_descr, buff, buff_size);
  if (wrtn < 0)
  {
    perror("Can't write to file");
    return -1;
  }

  while(wrtn != buff_size)
  {
    wrtn += write(out_descr, buff + wrtn, buff_size - wrtn);
    PRINT_LINE;
    printf("wrtn = %d\n", wrtn);
  }
  return 0;
}

static int myCat(const int in_descr, const int out_descr)
{
  if (in_descr < 0)
  {
    perror("Can't open input file");
    return -1;
  }
  if (out_descr < 0)
  {
    perror("Can't open output file");
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
  }

  return 0;
}

void eat_line() { while (getchar() != '\n'); }


int main(const int argc, char * const argv[])
{
  if (argc < 3)
  {
    printf("Not enough cml line args. (at least 3)\n");
    return 0;
  }

  Options opts;
  int opt;
  int amnt = 0;

  while ((opt = getopt(argc, argv, "ifv")) != -1)
  {
    printf("opt = %d\n", opt);

    switch (opt)
    {
      case 'v':
        opts.v = 1;
        amnt++;
        break;
      case 'f':
        opts.f = 1;
        amnt++;
        break;
      case 'i':
        opts.i = 1;
        amnt++;
        break;
      // default:
    }
  }
  
  if (argc - amnt == 3)
  {
    int in_descr  = open(argv[1 + amnt], O_ASYNC);

    int out_descr = open(argv[2 + amnt], O_TRUNC | O_WRONLY);
    if (out_descr)
    {
      if (opts.i)
      {
        printf("Are you sure you want to overwrite the contents of the file %s?\n"
               "Press y/n\n", argv[2 + amnt]);
        if (getchar() == 'n')
          return 0;
      }
    }
    else
      out_descr = open(argv[2 + amnt], O_CREAT | O_TRUNC | O_WRONLY, 0777);

    if (opts.v)
      printf("%s is copying\n", argv[1 + amnt]);

    myCat(in_descr, out_descr);
  
    close(in_descr);
    close(out_descr);

    return 0;
  }

  int  dir_name_len = strlen(argv[argc - 1]);   // making template for new files name
  char out_name[128];                           // like "dir/...."
  strcpy(out_name, argv[argc - 1]);
  out_name[dir_name_len] = '/';

  int i;
  for (i = 1 + amnt; i < argc-1; ++i)
  {
    int in_descr  = open(argv[i], O_ASYNC);

    strcat(out_name, argv[i]);
    int out_descr = open(out_name, O_TRUNC | O_WRONLY);
    if (out_descr)
    {
      if (opts.i)
      {
        printf("Are you sure you want to overwrite the contents of the file %s?\n"
               "Press y/n\n", out_name);
        
        if (getchar() == 'n')
        {
          out_name[dir_name_len + 1] = '\0';    // recovering template "dir/..."
          eat_line();
          continue;
        }
        eat_line();
      }
    }
    else
      out_descr = open(out_name, O_CREAT | O_TRUNC | O_WRONLY, 0777);

    if (opts.v)
      printf("%s is copying\n", argv[i]);

    myCat(in_descr, out_descr);
  
    close(in_descr);
    close(out_descr);
    out_name[dir_name_len + 1] = '\0';    // recovering template "dir/..."
  }

  return 0;
}