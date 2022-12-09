#include <pthread.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <exception>
#include <iostream>
#include <cassert>

#define DEBUG_MODE 1
#include "../debugLib/deb_macro.h"

const int BUFF_SIZE = 4096;

class Except : public std::exception
{
  const int line;
  const char *func;

  public:
  Except(int line, const char *func) : line(line), func(func) {}

  const char *what() const noexcept override
  {
    printf("Exception in [%s:%d]\n", func, line);

    return "Exception was triggered all info is above\n";
  }
};

class Monitor
{
  private:
    pthread_cond_t empty, full;
    pthread_mutex_t mutex;

    int buffs_amnt_, 
        head_ = 0,
        tail_ = 0,
        free_space_;
    
    bool is_done_;

  public:
    char **fifo_buffs_;

  public:
    Monitor(int buff_amnt = 16) : buffs_amnt_(buff_amnt), 
                                  free_space_(buff_amnt)
    {
      try { fifo_buffs_ = (char **) calloc(buffs_amnt_, sizeof(char *)); }
      catch (std::bad_alloc) { throw(Except(__LINE__, __func__)); }

      for (int i = 0; i < buffs_amnt_; ++i)
      {
        try {fifo_buffs_[i] = (char *) calloc(BUFF_SIZE, sizeof(char));}
        catch (std::bad_alloc){
          for (int j = 0; j < i - 1; j++)
            free(fifo_buffs_[j]);

          throw(Except(__LINE__, __func__));
        }
      }
    }

    bool setDone() { return is_done_ = true; }

    int getBuffForRead();
    int getBuffForWrite();

    void readDone();
    void writeDone();

    bool is_empty() { return free_space_ == buffs_amnt_; }
    bool is_done() { return is_done_; }

    pthread_mutex_t *getMutexPtr() { return &mutex; }

    void dump() {
      fprintf(stderr, "Monitor dump: ");
      fprintf(stderr, "head = %d, tail = %d, free_space = %d\n", 
                       head_, tail_, free_space_);
    }

    ~Monitor() 
    {
      for (int i = 0; i < buffs_amnt_; ++i) {
        free(fifo_buffs_[i]);
      }

      free(fifo_buffs_);
    }
} G_monitor;

namespace {
int safeWrite(int out_descr, const char *buff, const size_t buff_size)
{
  PRINT_LINE;

  IS_VALID(buff);

  int wrtn = 0;

  while (wrtn != buff_size)
    wrtn += write(out_descr, buff + wrtn, buff_size - wrtn);

  PRINT_LINE;

  return 0;
}

int read(char *buff, int in_descr)
{
  assert(buff);

  PRINT_LINE;
    
  ssize_t buff_size = ::read(in_descr, buff, BUFF_SIZE);

  PRINT_LINE;
  if (buff_size < 0) 
  {
    perror("Can't read from istream\n");
    throw (Except(__LINE__, __func__));
  }

  PRINT_LINE;

  return buff_size;
}

struct CMDArgs
{
  const int argc;
  const char * const *argv;
};

void *read(void *ex_args)
{
  assert(ex_args);

  PRINT_LINE;

  CMDArgs *args = (CMDArgs *)ex_args;
  int syms_read = 0;

  do {
    
    int buff_idx = G_monitor.getBuffForRead();

    if (args->argc < 2)
    {
      PRINT_LINE;
      
      try { syms_read = read(G_monitor.fifo_buffs_[buff_idx], 0); }
      catch(const Except &ex)
      {
        std::cout << ex.what() << '\n';
        exit(EXIT_FAILURE);
      }
    }
    else 
      for (int i = 1; i < args->argc; ++i)
      {
        int in_descr = open(args->argv[i], O_ASYNC);

        try { syms_read = read(G_monitor.fifo_buffs_[buff_idx], in_descr); }
        catch(const Except &ex) 
        {
          std::cout << ex.what() << '\n';
          exit(EXIT_FAILURE);
        }
      }

    if (syms_read == 0)
      G_monitor.setDone();

    G_monitor.readDone();
  
  } while (syms_read > 0);

  

  return nullptr;
}

void *write(void *ex_args)
{
  //!
  // Need to lock mutex, while writing to buff or reading from it
  // same for reader func
  assert(ex_args);

  CMDArgs *args = (CMDArgs *)(ex_args);
 
  for (;;) {
    int buff_idx = G_monitor.getBuffForWrite();
    
    if (buff_idx == -1)
    {
      G_monitor.writeDone();
      return nullptr;
    }
    PRINT_LINE;

    if (args->argc < 2)
    {
      PRINT_LINE;

      safeWrite(1, G_monitor.fifo_buffs_[buff_idx], BUFF_SIZE);
    }
    else
      for (int i = 1; i < args->argc; ++i)
        safeWrite(1, G_monitor.fifo_buffs_[buff_idx], BUFF_SIZE);
  
    G_monitor.writeDone();
  }

  return nullptr;
}
} // anonymous namespace

/// Method, that can getIndex of the first free bucket, so we will able 
/// to read from stdin to buffer
int Monitor::getBuffForRead()
{
  PRINT_LINE;
  print_debug({SAFE_PRINTF("free_space_ = %d, buffs_amnt_ = %d\n", 
                            free_space_, buffs_amnt_);})

  pthread_mutex_lock(&mutex);

  if (free_space_ == 0) 
  {
    PRINT_LINE;
    pthread_cond_wait(&full, &mutex);
  }
  
  int idx = tail_;
  tail_ = ++tail_ % buffs_amnt_;
  free_space_--;

  print_debug({SAFE_PRINTF("Idx of buff for reading = %d\n", idx);})

  return idx;
}

/// Method, that can getIndex of the first busy bucket, so we will able 
/// to write to stdout
int Monitor::getBuffForWrite()
{
  PRINT_LINE;
 
  pthread_mutex_lock(&mutex);
  
  if (is_done_)
    return -1;

  if (free_space_ == buffs_amnt_)
  {
    PRINT_LINE;
    pthread_cond_wait(&empty, &mutex);
  }

  int idx = head_;
  head_ = ++head_ % buffs_amnt_;
  free_space_++;
 
  print_debug({SAFE_PRINTF("Idx of buff for writing = %d\n", idx);})

  return idx;
}

void Monitor::readDone()
{
  print_debug({SAFE_PRINTF("Reading done\n");})

  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&empty);
}

void Monitor::writeDone()
{
  print_debug({SAFE_PRINTF("Writing done\n");})

  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&full);
}

int main(const int argc, char * const argv[])
{
  pthread_t writer,
            reader;

  CMDArgs args = {argc, argv};
  
  PRINT_LINE;
  
  if (pthread_create(&writer, nullptr, write, &args))
  {
    perror("Creating writer thread error:"); 
    return 0;
  }
  if (pthread_create(&reader, nullptr, read,  &args))
  {
    perror("Creating reader thread error:");
    pthread_cancel(writer);
    return 0;
  }

  PRINT_LINE;

  pthread_join(reader, nullptr);
  pthread_join(writer, nullptr);

  return 0;
}
