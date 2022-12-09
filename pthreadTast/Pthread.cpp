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
        first_free_,
        last_busy_;

  public:
    char **fifo_buffs_;

  public:
    Monitor(int buff_amnt = 16) : buffs_amnt_(buff_amnt), first_free_(0),
    last_busy_(-1)
  {
    try { fifo_buffs_ = new char*[buff_amnt]; }
    catch (std::bad_alloc) {
      throw(Except(__LINE__, __func__));
    }

    for (int i = 0; i < buff_amnt; ++i)
    {
      try {fifo_buffs_[i] = new char[BUFF_SIZE];}
      catch (std::bad_alloc){
        for (int j = 0; j < i - 1; j++)
          delete [] fifo_buffs_[j];

        throw(Except(__LINE__, __func__));
      }
    }
  }

    int getBuffForRead();
    int getBuffForWrite();

    pthread_mutex_t *getMutexPtr() { return &mutex; }

    void markBuffAsFree() {}

    ~Monitor() 
    {
      for (int i = 0; i < buffs_amnt_; ++i) {
        delete [] fifo_buffs_[i];
      }

      delete [] fifo_buffs_;
    }
} G_monitor;

namespace {
  int safeWrite(int out_descr, const char *buff, const size_t buff_size)
  {
    PRINT_LINE;

    IS_VALID(buff);

    int wrtn = 0;

    while(wrtn != buff_size)
      wrtn += write(out_descr, buff + wrtn, buff_size - wrtn);

    return 0;
  }

  void read(char *buff, int in_descr)
  {
    PRINT_LINE;
    
    ssize_t buff_size = ::read(in_descr, buff, BUFF_SIZE);
    while (buff_size > 0)
    {
      
    }

    PRINT_LINE;
    if (buff_size < 0) 
    {
      perror("Can't read from istream\n");
      throw (Except(__LINE__, __func__));
    }

    PRINT_LINE;
  }

  struct CMDArgs
  {
    const int argc;
    const char * const *argv;
  };

  void *read(void *ex_args)
  {
    assert(ex_args);

    CMDArgs *args = (CMDArgs *)ex_args;

    for (int i = 1; i < args->argc; ++i)
    {
      int buff_idx = G_monitor.getBuffForRead();
      int in_descr = open(args->argv[i], O_ASYNC);

      try
      {
        // FIXME: change to custom in_descr
        //read(G_monitor.fifo_buffs_[buff_idx], in_descr);
      
        
        read(G_monitor.fifo_buffs_[buff_idx], 0);
      }
      catch(Except ex) 
      {
        std::cout << ex.what() << '\n';
      }

    }

    return nullptr;
  }

  void *write(void *ex_args)
  {
    //!
    // Need to lock mutex, while writing to buff or reading from it
    // same for reader func
    
    assert(ex_args);

    CMDArgs *args = (CMDArgs *)(ex_args);

    for (int i = 1; i < args->argc; ++i)
    {
      int buff_idx = G_monitor.getBuffForWrite();
      safeWrite(1, G_monitor.fifo_buffs_[buff_idx], BUFF_SIZE);
    }

    return nullptr;
  }
} // anonymous namespace

int Monitor::getBuffForRead()
{
  pthread_mutex_lock(&mutex);

  if (first_free_ >= buffs_amnt_)
    pthread_cond_wait(&empty, &mutex);

  int free_idx = first_free_;
  first_free_  = (++first_free_) % buffs_amnt_;

  pthread_mutex_unlock(&mutex);

  return free_idx;
}

int Monitor::getBuffForWrite()
{
  pthread_mutex_lock(&mutex);

  if (last_busy_ <= 0)
    pthread_cond_wait(&full, &mutex);

  int busy_idx = last_busy_;
  last_busy_   = (G_monitor.first_free_ != last_busy_ + 1) ? (last_busy_ + 1) : -1;

  pthread_mutex_unlock(&mutex);

  return busy_idx;
}

int main(const int argc, char * const argv[])
{
  pthread_t writer,
            reader;

  CMDArgs args = {argc, argv};

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

  pthread_join(reader, nullptr);
  pthread_join(writer, nullptr);

  return 0;
}
