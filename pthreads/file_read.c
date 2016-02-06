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
pthread_mutex_t reducer_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gen_read=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_write,sem_read,sem_read1,sem_write1;
sem_t sem_read2,sem_write2;
int doneReading=0,doneReading1=0;
int no_of_mapper_thread,no_of_reducer_thread,no_of_summarizer_thread;
pthread_t mapper_thread;
pthread_t mapper_thread1;
pthread_t reducer_thread;
pthread_t reducer_thread1;
pthread_t reducer_thread2;
pthread_t write_thread;
pthread_t file_reader;

extern void* wordCountWriter(void *);
extern void* reducer(void *);

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
sem_post(&sem_write);
printf("Mapper Updater Thread %lld exiting\n",pthread_self());
}


void* mapper(void *map)
{
char buffer[MAX_BUF];
int done;
while(1){
printf("Mapper Thread %lld started\n",pthread_self());
int i=0,i1=0;
sem_wait(&sem_write);
pthread_mutex_lock(&gen_read);
done=doneReading;
if(done){
doneReading1=1;
sem_post(&sem_read);
sem_post(&sem_write1);
pthread_mutex_unlock(&gen_read);
printf("Mapper Thread %lld exiting\n",pthread_self());
pthread_exit(0);
}
else {
pthread_mutex_unlock(&gen_read);
}
sem_wait(&sem_read1);
pthread_mutex_lock(&mapper_lock);
while(mapper_pool[i]) {
buffer[i1++]=mapper_pool[i];
mapper_pool[i++]=0;
}

char *pointToCh;int size=0;
pointToCh=(char*)strtok(buffer," \n");
pthread_mutex_lock(&reducer_lock);
while(pointToCh!=NULL)
{
sprintf(reducer_pool+size,"(%s,1) ",pointToCh);
size+=strlen(pointToCh)+5;
pointToCh=(char*)strtok(NULL," \n");
//puts(reducer_pool);
}
pthread_mutex_unlock(&reducer_lock);
pthread_mutex_unlock(&mapper_lock);
sem_post(&sem_write1);
sem_post(&sem_read);
memset(buffer,0,MAX_BUF);
}
peintf("Mapper Thread %lld Exited\n");
}

void init() {
sem_init(&sem_write,0,0);
sem_init(&sem_read,0,1);
sem_init(&sem_read1,0,1);
sem_init(&sem_write1,0,0);
sem_init(&sem_write2,0,0);
sem_init(&sem_read2,0,1);
}


int main(int argc,char* argv[]){
init();
//no_of_mapper_thread=itoa(argv[1]);
//no_of_reducer_thread=itoa(argv[2]);
/*
pthread_t mapper_thread;
pthread_t mapper_thread1;
pthread_t reducer_thread;
pthread_t reducer_thread1;
pthread_t reducer_thread2;
pthread_t write_thread;
pthread_t file_reader;
*/
char buf[]="./a.txt";
pthread_create(&file_reader,NULL,mapper_pool_updater,(void*)buf);
pthread_create(&mapper_thread,NULL,mapper,NULL);
pthread_create(&mapper_thread1,NULL,mapper,NULL);
pthread_create(&reducer_thread,NULL,reducer,NULL);
pthread_create(&write_thread,NULL,wordCountWriter,NULL);

pthread_join(file_reader,NULL);
pthread_join(mapper_thread,NULL);
pthread_join(mapper_thread1,NULL);
pthread_join(reducer_thread,NULL);
pthread_join(write_thread,NULL);
return 0;
}
