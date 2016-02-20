#include<stdio.h>
#include<semaphore.h>
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
