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

#define DEBUG_MODE 1
#include "../debugLib/deb_macro.h"

namespace {
struct SyncData {
  int shm;
  char *buff;
  int buff_len;
  sem_t *sem;
  sem_t *is_full;
  sem_t *is_empty;
};

void parentProc(SyncData sync_data);
void childProc(SyncData sync_data);
} // anonymous namespace

int main(const int argc, char *const argv[]) {
  if (argc < 3) {
    std::cout << "Wrong format, enter <any-key-word> <buff-len>\n";
    exit(EXIT_FAILURE);
  }

  off_t buff_len = atoi(argv[2]);
  bool need_allocate = false;

  SyncData data;

  data.shm = shm_open(argv[1], 0, 0777);
  if (data.shm == -1) {
    if ((data.shm = shm_open(argv[1], O_CREAT, 0777)) == -1) {
      perror("shm_open error");
      exit(EXIT_FAILURE);
    }

    need_allocate = true;
  }

  if (need_allocate) {
    if (ftruncate(data.shm, buff_len) == -1) {
      perror("ftruncate error");
      exit(EXIT_FAILURE);
    }
  }

  data.buff = (char *)mmap(nullptr, buff_len, PROT_WRITE | PROT_READ,
                           MAP_SHARED, data.shm, 0);
  if (data.buff == (void *)-1) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

  if ((data.sem = sem_open("/sem", O_CREAT)) == SEM_FAILED) {
    perror("sem_open failed");
  }
  if ((data.is_full = sem_open("/is_full", O_CREAT)) == SEM_FAILED) {
    perror("sem_open failed");
  }
  if ((data.is_empty = sem_open("/is_empty", O_CREAT)) == SEM_FAILED) {
    perror("sem_open failed");
  }

  int pid;
  for (int i = 0; i < buff_len; ++i) {
    pid = fork();

    if (pid == 0)
      break;
  }

  // Now we need to run all processes
  if (pid == 0)
    childProc(data);
  else
    parentProc(data);

  if (pid != 0) {
    munmap(data.buff, buff_len);
    close(data.shm);
    shm_unlink(argv[1]);
  }

  return 0;
}

void parentProc(SyncData data) {}

void childProc(SyncData data) {}
