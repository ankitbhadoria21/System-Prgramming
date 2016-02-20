#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#include<memory.h>
#include<stdlib.h>
#include"sem.h"

#define MAX 100
#define LEFT(i,n) (i+n-1)%n
#define RIGHT(i,n) (i+1)%n

int n;
enum state_v{thinking,hungry,eating};
void test(semaphore_t *sem,int i);

void set_state(semaphore_t *sem,int i,int state) {
printf("\nin set\n");
sem->state[i]=state;
}

int get_state(semaphore_t* sem,int i) {
printf("in get\n");
return sem->state[i];
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
sprintf(phil_name,"%s%d","/tmp/philospher",i);
semaphore_t *read_sem=semaphore_open("/tmp/semaphore");
semaphore_wait(read_sem);
set_state(read_sem,i,hungry);
printf("Philospher %d Hungry\n",i);
test(read_sem,i);
semaphore_post(read_sem);
printf("after post\n");
//semaphore_close(read_sem);//check if works uncommented
semaphore_t *phil_sem=semaphore_open(phil_name);
if(!phil_sem) {
printf("cant open phil_sem\n");
exit(0);
}
semaphore_wait(phil_sem);
printf("after take\n");
}

void put_fork(int i) {
semaphore_t *read_sem=semaphore_open("/tmp/semaphore");
semaphore_wait(read_sem);
set_state(read_sem,i,thinking);
test(read_sem,LEFT(i,n));
test(read_sem,RIGHT(i,n));
semaphore_post(read_sem);
}

void test(semaphore_t *sem,int i) {
char phil_name[MAX];
sprintf(phil_name,"%s%d","/tmp/philospher",i);
semaphore_t *phil_sem=semaphore_open(phil_name);
if(get_state(sem,i)==hungry && get_state(sem,LEFT(i,n))!=eating && get_state(sem,RIGHT(i,n))!=eating) {
set_state(sem,i,eating);
semaphore_post(phil_sem);
printf("exit test\n");
}
}

void philospher(int i,int nooftime) {
while(nooftime--) {
think(i);
take_fork(i);
eat(i);
put_fork(i);
printf("put fork\n");
}

}

int main(int argc, char *argv[])
{
int phi_no=atoi(argv[1]);
int ttl_loop=atoi(argv[2]);
n=atoi(argv[3]);
barrier_t *bar=barrier_open("/tmp/barrier");
printf("philospher process %d waiting at barrier\n",phi_no);
barrier(bar);
philospher(phi_no,ttl_loop);
return 0;
}
