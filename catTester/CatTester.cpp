#include <sys/poll.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <iostream>
#include <cassert>
#include <poll.h>

namespace {
#define SAFE_PRINTF(...) fprintf(stderr, __VA_ARGS__)

#define PRINT_LINE {                                \
  if (DEBUG_MODE)                                   \
    SAFE_PRINTF("[%s:%d]\n", __func__, __LINE__);   \
}
#define print_debug(code)   \
{                           \
  if (DEBUG_MODE)           \
    {code;}                 \
}

#define DEBUG_MODE 1

const int BUFF_SIZE = 4096;
const int TIMEOUT   = 10;

struct Buffer
{
  char buff[BUFF_SIZE];
  int buff_size;
};

inline void dumpPollStruct(const pollfd *arg)
{
  std::cout << "PollStruct dump: " << "FileDescr = " << arg->fd << ", events = "
            << arg->events << ", revents = " << arg->revents 
            << ((arg->revents == POLLIN) ? " (POLLIN)" : " (POLLOUT)") << '\n';
}

void countFinalSize(pollfd *arr, Buffer &trash_buff, int *res_arr, int proc_amnt);
bool runTesting(pollfd *arr, int &proc_amnt, int *res_arr);
int  processRevent(pollfd *fd, Buffer &buffer);
bool verifyBytesAmnt(int *res_arr, int &proc_amnt, int &stdin_amnt);
}

int main(const int argc, char * const argv[])
{
  if (argc != 3) 
  {
    std::cout << "Wrong format, enter <cat_name> <run_numbers>\n";
    return 0;
  }
  
  int proc_amnt = atoi(argv[2]);
  int pid;
  int fds1[2],
      fds2[2];

  print_debug({std::cout << "proc_amnt = " << proc_amnt << '\n';})

  pollfd *children_arr = new pollfd[2 * proc_amnt];
  // Array for storring the amount of written bytes 
  int *bytes_amnt_arr = new int[proc_amnt];

  //
  for (int i = 0; i < proc_amnt; i++) 
    bytes_amnt_arr[i] = 0;
  //
  
  // Now connect parent proc with children
  for (int i = 0; i < 2 * proc_amnt; i += 2)
  { 
    pipe(fds1); // pipe for reading
    pipe(fds2); // pipe for writing

    children_arr[i]     = {fds1[0], POLLIN,  0};
    children_arr[i + 1] = {fds2[1], POLLOUT, 0};

    pid  = fork();
    
    if (pid == 0)
    {
      if (dup2(fds2[0], 0) == -1)
      {
        perror("dup2 failed");
        exit(EXIT_FAILURE);
      }
      if (dup2(fds1[1], 1) == -1)
      {
        perror("dup2 failed");
        exit(EXIT_FAILURE);
      }
      close(fds2[0]);
      close(fds1[1]);
  
      if (execvp(argv[1], argv + 1) == -1)
      {
        perror("execvp failed");
        exit(EXIT_FAILURE);
      }

      break;
    }
    else    // Parent process
    {
      close(fds2[0]);
      close(fds1[1]); 
    }
  }

  if (pid != 0)
  {
    if (!runTesting(children_arr, proc_amnt, bytes_amnt_arr))
      SAFE_PRINTF("Test failed\n");
    else 
      SAFE_PRINTF("Tests passed, great!\n");
  
    int status;
    for (int i = 0; i < proc_amnt; ++i)
      while (wait(&status))
        continue;
  }

  delete [] children_arr;
  delete [] bytes_amnt_arr;

  return 0;
}

namespace {
/// Function, that running all tests and compare results with standart,
/// also that function getting the input from stdin
bool runTesting(pollfd *arr, int &proc_amnt, int *res_arr)
{
  assert(arr);
  assert(proc_amnt);

  PRINT_LINE;

  Buffer buffer;
  int stdin_bytes_amnt = 0;
  while ((buffer.buff_size = read(0, buffer.buff, BUFF_SIZE)) > 0)
  {
    PRINT_LINE;

    stdin_bytes_amnt += buffer.buff_size;

    // Polling all fds
    int poll_status = poll(arr, proc_amnt * 2, TIMEOUT);
    if (poll_status == - 1)
    {
      perror("poll failed");
      exit(EXIT_FAILURE);
    }
    else if (poll_status == 0)
      SAFE_PRINTF("poll timeout ended\n");
   
    print_debug(SAFE_PRINTF("poll status = %d\n", poll_status););

    for (int i = 0; i < proc_amnt * 2; ++i)
    {
      PRINT_LINE;

      short &revents = arr[i].revents;
      int read_bytes = -1;
      if ((revents & POLLIN) || (revents & POLLOUT))
        read_bytes = processRevent(&arr[i], buffer);
      else
        continue;

      if (read_bytes > 0)
        res_arr[i / 2] += read_bytes; // i/2, because we have 2 fds on each child
                                      // process

      print_debug(
          {
            std::cout << "ResArray dump:\n";
            for (int j = 0; j < proc_amnt; ++j)
            {
              SAFE_PRINTF("Res[%d] = %d, ", j, res_arr[j]); 
            }
            std::cout << '\n';
          }
          )

      print_debug(std::cout << "read_bytes = " << read_bytes << std::endl;)

      // Need to clear state after processing
      revents = 0;
    }
  }

  countFinalSize(arr, buffer, res_arr, proc_amnt); 

  return verifyBytesAmnt(res_arr, proc_amnt, stdin_bytes_amnt);
}

int processRevent(pollfd *fd, Buffer &buffer)
{
  assert(fd);
 
  PRINT_LINE;
  dumpPollStruct(fd);

  int size_temp = 0;

  // There is data in buff to read
  if (fd->revents == POLLIN)
  {
    // Buffer for reading from pipe, does not matter, what it contains, we are
    // interested only in length of buff
    char trash_buff[BUFF_SIZE];
    int bytes_read = 0;

    do 
    {
      PRINT_LINE;
      print_debug({std::cout << "read syms to trash buff = " << size_temp << '\n';})
      
      PRINT_LINE;
      size_temp = read(fd->fd, trash_buff, BUFF_SIZE);
      PRINT_LINE;

      if (size_temp == -1)
      {
        perror("read failure");
        exit(EXIT_FAILURE);
      }

      bytes_read += size_temp;
    } while (size_temp == BUFF_SIZE);

    return bytes_read;
  }
  
  PRINT_LINE;
  // FIXME: We can not write more than 4096 symbs in row
  if ((size_temp = write(fd->fd, buffer.buff, buffer.buff_size)) == -1)
  {
    perror("write failed");
    exit(EXIT_FAILURE);
  }
  
  print_debug({std::cout << "bytes written = " << size_temp << '\n';})

  return -1;
}

void countFinalSize(pollfd *arr, Buffer &trash_buff, int *res_arr, int proc_amnt)
{
  assert(arr);
  assert(res_arr);

  int poll_status = poll(arr, proc_amnt * 2, TIMEOUT * 10);
  if (poll_status == - 1)
    {
      perror("poll failed");
      exit(EXIT_FAILURE);
    }
  else if (poll_status == 0)
    SAFE_PRINTF("poll timeout ended\n");
  
  for (int i = 0; i < proc_amnt * 2; ++i)
  {
    int read_bytes = 0;

    if (arr[i].revents & POLLIN)
    {
      read_bytes = processRevent(&arr[i], trash_buff);
      res_arr[i / 2] += read_bytes;

      SAFE_PRINTF("finally read %d bytes\n", read_bytes);
    }
  }
}

bool verifyBytesAmnt(int *res_arr, int &proc_amnt, int &stdin_amnt)
{
  assert(res_arr);

  bool is_ok = true;
  for (int i = 0; i < proc_amnt; ++i)
  {
    printf("Test #%d: Expected %d -> get %d\n", i, stdin_amnt, res_arr[i]);

    if (res_arr[i] != stdin_amnt)
      is_ok = false;
  }
  return is_ok;
}
}// anonymous namespace
