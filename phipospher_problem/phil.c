#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#include<memory.h>
#include"sem.h"

#define MAX 100
#define LEFT(i,n) (i+n-1)%n
#define RIGHT(i,n) (i+1)%n

enum state_v{thinking,hungry,eating};
int n;

void test(int i);

void set_state(int i,int state) {
printf("in set\n");
char file_name[MAX];
sprintf(file_name,"%s%d","/tmp/philospher",i);
semaphore_t *sem_tmp=semaphore_open(file_name);
sem_tmp->state=state;
semaphore_close(sem_tmp);
}

int get_state(int i) {
printf("in get\n");
char file_name[MAX];
sprintf(file_name,"%s%d","/tmp/philospher",i);
semaphore_t *sem_tmp=semaphore_open(file_name);
int state=sem_tmp->state;
printf("state-%d\n",state);
semaphore_close(sem_tmp);
return state;
}

void eat(int i) {
printf("Philospher %d Eating\n",i);
sleep(1);
}

void think(int i) {
printf("Philospher %d Thinking\n",i);
sleep(1);
}

void take_fork(int i) {
char phil_name[MAX];
sprintf(phil_name,"%s%d","/tmp/philoshpher",i);
semaphore_t *read_sem=semaphore_open("/tmp/semaphore");
semaphore_wait(read_sem);
set_state(i,hungry);
printf("Philospher %d Hungry\n",i);
test(i);
semaphore_post(read_sem);
printf("after post\n");
//semaphore_close(read_sem);//check if works uncommented
semaphore_t *phil_sem=semaphore_open(phil_name);
semaphore_wait(phil_sem);
printf("after take\n");
}

void put_fork(int i) {
semaphore_t *read_sem=semaphore_open("/tmp/semaphore");
semaphore_wait(read_sem);
set_state(i,thinking);
test(LEFT(i,n));
test(RIGHT(i,n));
semaphore_post(read_sem);
}

void test(int i) {
char phil_name[MAX];
sprintf(phil_name,"%s%d","/tmp/philospher",i);
semaphore_t *phil_sem=semaphore_open(phil_name);
if(get_state(i)==hungry && get_state(LEFT(i,n))!=eating && printf("in test\n") && get_state(RIGHT(i,n))!=eating) {
set_state(i,eating);
semaphore_post(phil_sem);
}
}

void philospher(int i,int nooftime) {
while(nooftime--) {
printf("i-%d , t-%d",i,nooftime);
think(i);
take_fork(i);
eat(i);
put_fork(i);
}

}

int main(int argc, char *argv[])
{
printf("in phi\n");
int phi_no=atoi(argv[1]);
int ttl_loop=atoi(argv[2]);
printf("%d %d\n",phi_no,ttl_loop);
barrier_t *bar=barrier_open("/tmp/barrier");
barrier(bar);
philospher(phi_no,ttl_loop);
return 0;
}
