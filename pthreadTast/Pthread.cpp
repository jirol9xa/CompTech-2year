#include <pthread.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <exception>

const int BUFF_SIZE = 4096;

#define PRINT_LINE fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);

#define IS_VALID(param) {                   \
    if (!param)                             \
    {                                       \
        printf("Invalid " #param "ptr ");   \
        return -1;                          \
    }                                       \
}

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

static int safeWrite(int out_descr, const char *buff, const size_t buff_size)
{
    IS_VALID(buff);

    int wrtn = 0;

    while(wrtn != buff_size)
        wrtn += write(out_descr, buff + wrtn, buff_size - wrtn);

    return 0;
}

void read(char *buff, int in_descr)
{
  int buff_size = read(in_descr, buff, BUFF_SIZE);
  if (buff_size < 0) 
  {
    perror("Can't read from istream\n");
    throw (Except(__LINE__, __func__));
  }
}

void *read(void *)
{
  int buff_idx = G_monitor.getBuffForRead();
  read(G_monitor.fifo_buffs_[buff_idx], 0);

  return nullptr;
}

void *write(void *)
{
  int buff_idx = G_monitor.getBuffForWrite();
  safeWrite(1, G_monitor.fifo_buffs_[buff_idx], BUFF_SIZE);

  return nullptr;
}

int Monitor::getBuffForRead()
{
  pthread_mutex_lock(&mutex);

  if (first_free_ >= buffs_amnt_)
    pthread_cond_wait(&empty, &mutex);
  
  int free_idx = first_free_;
  first_free_ = (++first_free_) % buffs_amnt_;

  pthread_mutex_unlock(&mutex);

  return free_idx;
}

int Monitor::getBuffForWrite()
{
  pthread_mutex_lock(&mutex);

  if (last_busy_ <= 0)
    pthread_cond_wait(&full, &mutex);

  int busy_idx = last_busy_;
  last_busy_ = (G_monitor.first_free_ != last_busy_ + 1) ? (last_busy_ + 1) : -1;

  pthread_mutex_unlock(&mutex);

  return busy_idx;
}


int main()
{
  pthread_t writer,
            reader;
  
  pthread_create(&writer, nullptr, write, nullptr);
  pthread_create(&reader, nullptr, read,  nullptr);

  return 0;
}
