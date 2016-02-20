#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include"sem.h"
#define MAX 100

int main(int argc,char *argv[]) {
int status;
pid_t pid;
int phil=atoi(argv[1]);
int i;
remove("/tmp/semaphore");
semaphore_t *sem=semaphore_create("/tmp/semaphore",1);
if(!sem) {
printf("Semaphore not created\n");
exit(0);
}
remove("/tmp/barrier");
barrier_t *bar=barrier_create("/tmp/barrier",phil);
if(!bar) {
printf("Barrier not created\n");
exit(0);
}
semaphore_t *sem_tmp;
for(i=1;i<=phil;++i) {
char name[MAX];
sprintf(name,"%s%d","/tmp/philospher",i);
remove(name);
sem_tmp=semaphore_create(name,0);
if(!sem_tmp) {
printf("Error in creating philospher semaphores\n");
exit(0);
}
}
for(i=1;i<=phil;++i) {
char num[MAX];
sprintf(num,"%d",i);
char *argv1[]={"./phil",num,argv[2],NULL};
pid=fork();
if(pid==0) {
execvp("./phil",argv1);
}
else if(pid==-1){
printf("Error in fork()\n");
}
}
wait(&status);
return 0;
} 