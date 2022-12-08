#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <poll.h>

#define PRINT_LINE fprintf(stderr, "[%s:%d]\n", __func__, __LINE__)

const int BUFF_SIZE = 4096;
const int TIMEOUT = 1000;

struct Pollfd
{
  int   fd;
  short events;
  short revents;
};

void runTesting(pollfd *arr, int proc_amnt);

int main(const int argc, char * const argv[])
{
  if (argc < 3) 
  {
    std::cout << "Wrong format, enter ./<> <cat_name> <run_numbers>\n";
    return 0;
  }
  
  int proc_amnt = atoi(argv[2]);
  int pid = 228322;
  int fds[2];

  // Now we need creat array with pipe's fd
  // Creating uint64_t, because we need create argc amnt arrays int[2];
  pollfd *children_arr = new pollfd[2 * proc_amnt];

  // Now connect parent proc with children
  for (size_t i = 0, j = 1; i < 2 * proc_amnt; i += 2, j += 2)
  { 
    int fd = children_arr[i].fd;
    
    pipe(fds);  // pipe for reading
    children_arr[i] = {fds[0], POLL_IN};
    children_arr[j] = {fds[1], POLL_OUT};

    pid  = fork();
    
    if (pid == 0)
    {
      if (dup2(fds[0], 0) == -1)
      {
        perror("dup2 failed");
        exit(EXIT_FAILURE);
      }
      if (dup2(fds[1], 1) == -1)
      {
        perror("dup2 failure");
        exit(EXIT_FAILURE);
      }
      close(fds[1]);
      close(fds[0]);
    
      break;
    }
  }

  if (pid != 0)
  {
    int status;
    for (int i = 0; i < proc_amnt; ++i)
      while (wait(&status))
        continue;
  }

  delete [] children_arr;
  return 0;
}

void runTesting(pollfd *arr, int proc_amnt)
{
  assert(arr);
  assert(proc_amnt);

  char buff[BUFF_SIZE];
  int buff_size;
  while (buff_size = read(0, buff, BUFF_SIZE))
  {
    // Polling all fds
    int poll_status = poll(arr, proc_amnt * 2, TIMEOUT);
    if (poll_status == - 1)
    {
      perror("poll failed");
      exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < proc_amnt * 2; ++i)
    {
      if (arr[i].revents == POLL_IN)
      {
       if (write(arr[i].fd, buff, buff_size) == -1)
       {
        perror("write failed");
        exit(EXIT_FAILURE);
       }
        // After use we need make our fd "clear"
        arr[i].revents = 0;
      }
    }
  }
}
