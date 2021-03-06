#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<semaphore.h>
#include<signal.h>
#include<stdlib.h>
#include<fcntl.h>
#define SENTINAL_CHAR '%'
#define MAX_BUF_SIZE 500
#define MAX_BUF 500

//mapper pool lock
char mapper_pool[MAX_BUF_SIZE];
char reducer_pool[5*MAX_BUF_SIZE];
pthread_mutex_t mapper_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reducer_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gen_read=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_write,sem_read,sem_read1,sem_write1;
sem_t sem_read2,sem_write2,sem_read3,sem_read4,sem_write3,sem_write4;
int doneReading=0,doneReading1=0;
int no_of_mapper_thread,no_of_reducer_thread,no_of_summarizer_thread;
pthread_t mapper_thread[1000];
pthread_t reducer_thread[1000];
pthread_t summarizer_thread[1000];
pthread_t write_thread,letter_thread;
pthread_t file_reader;

extern void* wordCountWriter(void *);
extern void* reducer(void *);
extern void* summarizer(void *);
extern void* write_letter(void *);

void* mapper_pool_updater(void *buf1)
{
char *buf=(char*)buf1;
FILE *fs=fopen(buf,"r");
if(!fs) {
printf("Wrong File Used ... Exiting\n");
exit(EXIT_FAILURE);
}

char buffer[MAX_BUF];
char ch=SENTINAL_CHAR,prev_char=SENTINAL_CHAR;
int i=0,size=0,j,i1;
while(!feof(fs)) {
j=0;i1=0;
while(ch==' '||ch=='\n') ch=fgetc(fs);
if(prev_char==SENTINAL_CHAR){
ch=fgetc(fs);
while(ch==' '||ch=='\n')ch=fgetc(fs);
prev_char=ch;
}
if(size<MAX_BUF-1) {
buffer[i++]=ch;
size++;
}
else {
buffer[i]='\0';
i=0;size=0;
sem_wait(&sem_read);
pthread_mutex_lock(&mapper_lock);
while(buffer[j]) {
mapper_pool[i1++]=buffer[j++];
}
mapper_pool[i1]='\0';
pthread_mutex_unlock(&mapper_lock);
sem_post(&sem_write);
continue;
}

ch=fgetc(fs);

if(ch==' ' || ch=='\n') {
while(ch==' '|| ch=='\n') ch=fgetc(fs);
if(ch!=prev_char) {
buffer[i]='\0';
i=0;size=0;
prev_char=ch;
sem_wait(&sem_read);
pthread_mutex_lock(&mapper_lock);
while(buffer[j]) {
mapper_pool[i1++]=buffer[j++];
}
mapper_pool[i1]='\0';
pthread_mutex_unlock(&mapper_lock);
sem_post(&sem_write);
continue;
}
else {
if(size<MAX_BUF-1) {
buffer[i++]=' ';
size++;
}
else {
buffer[MAX_BUF-1]='\0';
i=0;size=0;
sem_wait(&sem_read);
pthread_mutex_lock(&mapper_lock);
while(buffer[j]) {
mapper_pool[i1++]=buffer[j++];
}
mapper_pool[i1]='\0';
pthread_mutex_unlock(&mapper_lock);
sem_post(&sem_write);
continue;
}
}
}
}
int ind;
fclose(fs);
sem_wait(&sem_read);
pthread_mutex_lock(&gen_read);
doneReading=1;
pthread_mutex_unlock(&gen_read);
sem_post(&sem_write);

}


void* mapper(void *map)
{
char buffer[MAX_BUF];
int done;
while(1){
int i=0,i1=0;
sem_wait(&sem_write);
sem_wait(&sem_read1);
pthread_mutex_lock(&gen_read);
done=doneReading;
if(done){
int i,ind;
doneReading1+=1;
sem_post(&sem_read);
sem_post(&sem_write1);
for(ind=0;ind<no_of_reducer_thread;++ind)
sem_post(&sem_write1);
pthread_mutex_unlock(&gen_read);
pthread_exit(0);
}
else {
pthread_mutex_unlock(&gen_read);
}
pthread_mutex_lock(&mapper_lock);
while(mapper_pool[i]) {
buffer[i1++]=mapper_pool[i];
mapper_pool[i++]=0;
}
pthread_mutex_unlock(&mapper_lock);
char *pointToCh;int size=0;
pointToCh=(char*)strtok(buffer," \n");
pthread_mutex_lock(&reducer_lock);
while(pointToCh!=NULL)
{
sprintf(reducer_pool+size,"(%s,1) ",pointToCh);
size+=strlen(pointToCh)+5;
pointToCh=(char*)strtok(NULL," \n");
}
pthread_mutex_unlock(&reducer_lock);
sem_post(&sem_write1);
sem_post(&sem_read);
memset(buffer,0,MAX_BUF);
}
peintf("Mapper Thread %lld Exited\n");
}

//intialize semaphores
void init() {
sem_init(&sem_write,0,0);
sem_init(&sem_read,0,1);
sem_init(&sem_read1,0,1);
sem_init(&sem_write1,0,0);
sem_init(&sem_write2,0,0);
sem_init(&sem_read2,0,1);
sem_init(&sem_write3,0,0);
sem_init(&sem_read3,0,0);
sem_init(&sem_write4,0,0);
sem_init(&sem_read4,0,1);
}

int main(int argc,char* argv[]){
int i,j;
char file_name[MAX_BUF];
if(argc<5) {
printf("Wrong Usage\n");
exit(EXIT_FAILURE);
}
init();
strcpy(file_name,argv[1]);
no_of_mapper_thread=atoi(argv[2]);
no_of_reducer_thread=atoi(argv[3]);
no_of_summarizer_thread=atoi(argv[4]);
if(no_of_mapper_thread<=0 || no_of_mapper_thread>1000 || no_of_reducer_thread<=0 || no_of_reducer_thread>1000 ||no_of_summarizer_thread>1000|| no_of_summarizer_thread<=0) {
printf("Wrong Value for number of threads\n");
exit(EXIT_FAILURE);
}
char *file_to_read=file_name;

pthread_create(&file_reader,NULL,mapper_pool_updater,(void*)file_to_read);

for(i=0;i<no_of_mapper_thread;++i)
pthread_create(&mapper_thread[i],NULL,mapper,NULL);

for(i=0;i<no_of_reducer_thread;++i)
pthread_create(&reducer_thread[i],NULL,reducer,NULL);

for(i=0;i<no_of_summarizer_thread;++i)
pthread_create(&summarizer_thread[i],NULL,summarizer,NULL);

pthread_create(&write_thread,NULL,wordCountWriter,NULL);

pthread_create(&letter_thread,NULL,write_letter,NULL);

pthread_join(file_reader,NULL);
pthread_join(write_thread,NULL);
pthread_join(letter_thread,NULL);

return 0;
}
