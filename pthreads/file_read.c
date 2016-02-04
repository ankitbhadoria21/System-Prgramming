#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<semaphore.h>

#define MAX_BUF_SIZE 256
#define MAX_BUF 256

//mapper pool lock
char mapper_pool[MAX_BUF_SIZE];
char reducer_pool[5*MAX_BUF_SIZE];
pthread_mutex_t mapper_lock=PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t reducer_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gen_read=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_write,sem_read,sem_read1,sem_write1;

int doneReading=0;

void* mapper_pool_updater(void *buf)
{
int i,i1;
char buffer[MAX_BUF_SIZE];
char *buf1=(char*)buf;
FILE* fs=fopen(buf1,"r");

while(fgets(buffer,MAX_BUF_SIZE,fs)!=NULL){
i=0;i1=0;
//lock
sem_wait(&sem_read);
pthread_mutex_lock(&mapper_lock);
while(buffer[i]) {
if(buffer[i]=='\n') {
i++;continue;
}
mapper_pool[i1++]=buffer[i++];
}
mapper_pool[i1]='\0';
pthread_mutex_unlock(&mapper_lock);
//unlock
//puts(mapper_pool);
sem_post(&sem_write);
}
fclose(fs);
sem_wait(&sem_read);
doneReading=1;
sem_post(&sem_write);
}


void* mapper(void *map)
{
char buffer[MAX_BUF];
int done;
while(1){
int i=0,i1=0;
sem_wait(&sem_write);
pthread_mutex_lock(&gen_read);
done=doneReading;
pthread_mutex_unlock(&gen_read);
if(done){
sem_post(&sem_read);
break;
}

pthread_mutex_lock(&mapper_lock);
while(mapper_pool[i]) {
buffer[i1++]=mapper_pool[i++];
}

char *pointToCh;int size=0;
pointToCh=(char*)strtok(buffer," \n");
pthread_mutex_lock(&reducer_lock);
while(pointToCh!=NULL)
{
sprintf(reducer_pool+size,"(%s,1) ",pointToCh);
size+=strlen(pointToCh)+5;
pointToCh=(char*)strtok(NULL," \n");
puts(reducer_pool);
}
pthread_mutex_unlock(&reducer_lock);
pthread_mutex_unlock(&mapper_lock);
sem_post(&sem_read);
}

}




int main(){
sem_init(&sem_write,0,0);
sem_init(&sem_read,0,1);
sem_init(&sem_read1,0,1);
sem_init(&sem_write1,0,0);
pthread_t mapper_thread;
pthread_t mapper_thread1;
pthread_t file_reader;
char buf[]="./a.txt";
pthread_create(&file_reader,NULL,mapper_pool_updater,(void*)buf);
pthread_create(&mapper_thread,NULL,mapper,NULL);
pthread_create(&mapper_thread1,NULL,mapper,NULL);
pthread_join(file_reader,NULL);
pthread_join(mapper_thread,NULL);
return 0;
}
