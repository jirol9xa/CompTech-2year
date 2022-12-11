#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h> /* For O_* constants */
#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/types.h>
#include <unistd.h>

#define DEBUG_MODE 0
#include "../debugLib/deb_macro.h"

namespace {
struct SyncData {
  int shm;
  int *food_amnt;
  sem_t *sem;
  sem_t *is_full;
  sem_t *is_empty;
};

#define WAIT_SEM(param)         \
{                               \
  if (sem_wait(param) == -1)    \
  {                             \
    perror("sem_wait failed");  \
    exit(EXIT_FAILURE);         \
  }                             \
}

#define POST_SEM(param)         \
{                               \
  if (sem_post(param) == -1)    \
  {                             \
    perror("sem_post failed");  \
    exit(EXIT_FAILURE);         \
  }                             \
}

void parentProc(SyncData sync_data, int food_each_time);
void childProc(SyncData sync_data);
} // anonymous namespace

int main(const int argc, char *const argv[]) {
  if (argc < 6) {
    std::cout << "Wrong format, enter <any-key-word> "
              << "<amnt of food by mother each time> "
              << "<sem_name>" << "<is_full_sem_name> "
              << "is_empty_sem_name" << '\n';
    exit(EXIT_FAILURE);
  }

  off_t child_amnt = atoi(argv[2]);
  bool need_allocate = false;

  SyncData data;

  data.shm = shm_open(argv[1], O_RDWR, 0777);
  if (data.shm == -1) {
    if ((data.shm = shm_open(argv[1], O_CREAT | O_RDWR | O_EXCL, 0777)) == -1) {
      perror("shm_open error");
      exit(EXIT_FAILURE);
    }

    need_allocate = true;
  }

  if (need_allocate) {
    if (ftruncate(data.shm, sizeof(int)) == -1) {
      perror("ftruncate error");
      exit(EXIT_FAILURE);
    }
  }

  data.food_amnt = (int *)mmap(nullptr, sizeof(int), PROT_WRITE | PROT_READ,
                           MAP_SHARED, data.shm, 0);
  if (data.food_amnt == (void *)-1) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

  if ((data.sem = sem_open(argv[3], O_CREAT | O_EXCL, 0777, 0)) == SEM_FAILED) {
    perror("sem_open failed");
    exit(EXIT_FAILURE);
  }
  if ((data.is_full = sem_open(argv[4], O_CREAT | O_EXCL, 0777, 0)) == SEM_FAILED) {
    perror("sem_open failed");
    exit(EXIT_FAILURE);
  }
  if ((data.is_empty = sem_open(argv[5], O_CREAT | O_EXCL, 0777, 0)) == SEM_FAILED) {
    perror("sem_open failed");
    exit(EXIT_FAILURE); 
  }

  int pid;
  for (int i = 0; i < child_amnt; ++i) {
    pid = fork();

    if (pid == 0)
      break;
  }

  // Now we need to run all processes
  if (pid == 0)
  {
    data.shm = shm_open(argv[1], O_RDWR, 0777);
    data.food_amnt = (int *)mmap(nullptr, sizeof(int), PROT_WRITE | PROT_READ,
                           MAP_SHARED, data.shm, 0);
    if (data.food_amnt == (void *)-1) 
    {
      perror("mmap failed");
      exit(EXIT_FAILURE);
    }
    
    childProc(data);
  }
  else
    parentProc(data, atoi(argv[2]));

  if (pid != 0) {
    munmap(data.food_amnt, child_amnt);
    close(data.shm);
    shm_unlink(argv[1]);
  }

  return 0;
}

namespace {
void parentProc(SyncData data, int food_each_time) 
{
  *data.food_amnt = food_each_time; 
  for (int i = 0; i < food_each_time; ++i) 
  {
    POST_SEM(data.is_full);
  }
  POST_SEM(data.sem);

  for (;;)
  {
    WAIT_SEM(data.is_empty); 
    WAIT_SEM(data.sem);

    *(data.food_amnt) = food_each_time; 
    std::cout << "Mother bring a food\n";

    sleep(2);

    POST_SEM(data.sem);
 
    for (int i = 0; i < food_each_time; ++i)
      POST_SEM(data.is_full);
  }
}

void childProc(SyncData data) 
{
  for (;;)
  {
    WAIT_SEM(data.is_full);
    WAIT_SEM(data.sem);
  
    int food_amnt = --*data.food_amnt;
    std::cout << "Child ate a food, food remained " << food_amnt << '\n';
 
    sleep(2);

    POST_SEM(data.sem);

    if (food_amnt <= 0)
      POST_SEM(data.is_empty)
  }
}
} // anonymous namespace
