//original code from https://computing.llnl.gov/tutorials/pthreads/man/pthread_mutexattr_init.txt
//Modified for this Assignment
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include "sem.h"


semaphore_t * semaphore_create(char *semaphore_name, int sem_val)
{
int fd,i;
semaphore_t *semap;
pthread_mutexattr_t psharedm;
pthread_condattr_t psharedc;

fd = open(semaphore_name, O_RDWR | O_CREAT | O_EXCL, 0666);
if (fd < 0)
return (NULL);
(void) ftruncate(fd, sizeof(semaphore_t));
(void) pthread_mutexattr_init(&psharedm);
(void) pthread_mutexattr_setpshared(&psharedm,PTHREAD_PROCESS_SHARED);
(void) pthread_condattr_init(&psharedc);
(void) pthread_condattr_setpshared(&psharedc,PTHREAD_PROCESS_SHARED);
semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0);
close (fd);
(void) pthread_mutex_init(&semap->lock, &psharedm);
(void) pthread_cond_init(&semap->nonzero, &psharedc);
semap->count = sem_val;
for(i=1;i<=MAX_SIZE;++i)
semap->state[i]=0;
return (semap);
}


semaphore_t* semaphore_open(char *semaphore_name)
{
int fd;
semaphore_t *semap;

fd = open(semaphore_name, O_RDWR, 0666);
if (fd < 0)
return (NULL);
semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),PROT_READ | PROT_WRITE, MAP_SHARED,
fd, 0);
close (fd);
return (semap);
}


void semaphore_post(semaphore_t *semap)
{
pthread_mutex_lock(&semap->lock);
if (semap->count == 0)
pthread_cond_signal(&semap->nonzero);
semap->count++;
pthread_mutex_unlock(&semap->lock);
}


void semaphore_wait(semaphore_t *semap)
{
pthread_mutex_lock(&semap->lock);
while (semap->count == 0)
pthread_cond_wait(&semap->nonzero, &semap->lock);
semap->count--;
pthread_mutex_unlock(&semap->lock);
}


void semaphore_close(semaphore_t *semap)
{
munmap((void *) semap, sizeof(semaphore_t));
}


barrier_t* barrier_create(char* name, int max)
{
   int fd;
   barrier_t * bar_t;
   pthread_mutexattr_t lock_attr;
   pthread_condattr_t cond_attr;
   remove(name);
   fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
   if(fd<0) return NULL;
   ftruncate(fd, sizeof(barrier_t));
   pthread_mutexattr_init(&lock_attr);
   pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);

   pthread_condattr_init(&cond_attr);
   pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);

   bar_t = (barrier_t *)mmap(NULL,sizeof(barrier_t),PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
   close(fd);
   pthread_mutex_init(&bar_t->lock,&lock_attr);
   pthread_cond_init(&bar_t->done,&cond_attr);
   bar_t->curr_num = 0;
   bar_t->allowed =  max;
   return bar_t;

}
barrier_t* barrier_open(char* name)
{
   int fd;
   barrier_t *bar_map;
   fd = open(name, O_RDWR, 0666);
   if (fd < 0)
       return (NULL);
   bar_map = (barrier_t *) mmap(NULL, sizeof(barrier_t), PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0);
   close (fd);
   return bar_map;
}

void barrier_wait( barrier_t* bar_map)
{
   pthread_mutex_lock(&bar_map->lock);
   bar_map->curr_num++;
   if(bar_map->curr_num == bar_map->allowed)
       pthread_cond_broadcast(&bar_map->done);
  else
      pthread_cond_wait(&bar_map->done, &bar_map->lock);
    pthread_mutex_unlock(&bar_map->lock);
}

void barrier_close(barrier_t *barrier)
{
munmap((void *) barrier, sizeof(barrier_t));
}
