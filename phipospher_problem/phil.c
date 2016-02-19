#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#define MAX 10
#define LEFT(i,n) (i+n-1)%n
#define RIGHT(i,n) (i+1)%n

enum state_v{thinking,hungry,eating};
sem_t read_sem,phil_sem[MAX];
int state[MAX]={0};//put this in file
int n;
pthread_cond_t cond_var;
pthread_mutex_t cond_lock;
pthread_mutexattr_t mutex_attr;
pthread_condattr_t cond_attr;
int count=0; //put this in file
pthread_t thread[MAX];

void test(int i);

void eat(int i) {
printf("Philospher %d Eating\n",i);
sleep(1);
}

void think(int i) {
printf("Philospher %d Thinking\n",i);
sleep(1);
}

void take_fork(int i) {
sem_wait(&read_sem);
state[i]=hungry;
test(i);
sem_post(&read_sem);
sem_wait(&phil_sem[i]);
}

void put_fork(int i) {
sem_wait(&read_sem);
state[i]=thinking;
test(LEFT(i,n));
test(RIGHT(i,n));
sem_post(&read_sem);
}

void test(int i) {
if(state[i]==hungry && state[LEFT(i,n)]!=eating && state[RIGHT(i,n)]!=eating) {
state[i]=eating;
sem_post(&phil_sem[i]);
}
}

void* philospher(void* in) {
int i=(int)in;
pthread_mutex_lock(&cond_lock);
count++;
printf("c %d\n",count);
if(count==n) {
pthread_cond_broadcast(&cond_var);
}
pthread_cond_wait(&cond_var,&cond_lock);
pthread_mutex_unlock(&cond_lock);
printf("after %d\n",i);
while(1) {
think(i);
take_fork(i);
eat(i);
put_fork(i);
}

}

int main()
{
int i;
pthread_mutexattr_init(&mutex_attr);
pthread_condattr_init(&cond_attr);
pthread_mutexattr_setpshared(&mutex_attr,PTHREAD_PROCESS_SHARED);
pthread_condattr_setpshared(&cond_attr,PTHREAD_PROCESS_SHARED);
pthread_mutex_init(&cond_lock,&mutex_attr);
pthread_cond_init(&cond_var,&cond_attr);

sem_init(&read_sem,0,1);
for(i=0;i<MAX;++i)
sem_init(&phil_sem[i],0,0);
n=10;
for(i=1;i<=n;++i)
pthread_create(&thread[i],NULL,philospher,(void*)i);

for(i=0;i<=n;++i)
pthread_join(thread[i],NULL);
return 0;
}
