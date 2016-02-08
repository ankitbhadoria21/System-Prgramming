#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<signal.h>

#define SENTINAL '%'
#define MAX_BUF 256

extern char reducer_pool[5*MAX_BUF];
extern pthread_mutex_t reducer_lock;
char summarizer_pool[5*MAX_BUF];
int letter_table[26];
pthread_mutex_t table_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t summarizer_lock=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_read2,sem_write2;
extern sem_t sem_read1,sem_write1,sem_read3,sem_read4,sem_write3,sem_write4;;
extern int doneReading;
extern pthread_mutex_t gen_read;
extern int no_of_mapper_thread,no_of_reducer_thread,no_of_summarizer_thread;
extern pthread_t mapper_thread[1000];
extern pthread_t reducer_thread[1000];
extern pthread_t summarizer_thread[1000];
extern pthread_t letter_thread;
extern pthread_t write_thread;
extern pthread_t file_reader;

typedef struct node{
char str[MAX_BUF];
int count;
struct node *next;
}node;

pthread_mutex_t node_lock=PTHREAD_MUTEX_INITIALIZER;
node node_list[256]={0};
int  doneReading2=0,doneReading3=0;
extern int doneReading1;

void* reducer(void *a)
{
char buffer[5*MAX_BUF];
char *str,prev_char=SENTINAL;
char *tmp,*word_list[MAX_BUF];
int done;
while(1) {
int ind;
sem_wait(&sem_write1);
pthread_mutex_lock(&gen_read);
done=doneReading1;
if(done){
doneReading2=1;
for(ind=0;ind<no_of_mapper_thread-no_of_reducer_thread;++ind)
sem_post(&sem_read1);
sem_post(&sem_write2);
pthread_mutex_unlock(&gen_read);
pthread_exit(0);
}
else {
pthread_mutex_unlock(&gen_read);
}
sem_wait(&sem_read2);
//prev_char=SENTINAL;
int i=0,i1=0,index=0,ttl_wrd=0,size=0;

pthread_mutex_lock(&reducer_lock);
while(reducer_pool[i1]) {
buffer[i++]=reducer_pool[i1++];
//reducer_pool[i1++]=0;
}
pthread_mutex_unlock(&reducer_lock);

tmp=strtok(buffer," ");
while(tmp) {
word_list[ttl_wrd++]=tmp;
tmp=strtok(NULL," ");
}

pthread_mutex_lock(&node_lock);
while(index<ttl_wrd){

str=strtok(word_list[index++],",");
str+=1;

if(prev_char!=SENTINAL && str[0]!=prev_char){
node *tmp=&node_list[prev_char];
node *prev_tmp=tmp;
tmp=tmp->next;
prev_tmp->next=NULL;
pthread_mutex_lock(&summarizer_lock);
while(tmp){
sprintf(summarizer_pool+size,"(%s,%d)\n",tmp->str,tmp->count);
prev_tmp=tmp;
size+=strlen(tmp->str)+5;
tmp=tmp->next;
prev_tmp->next=NULL;

//free(prev_tmp);
}
pthread_mutex_unlock(&summarizer_lock);
}

node *tmp=&node_list[str[0]];
node *prev_tmp=tmp;
tmp=tmp->next;

while(tmp){
if(!strcmp(tmp->str,str))
{
tmp->count++;
break;
}
prev_tmp=tmp;
tmp=tmp->next;
}

if(!prev_tmp->next) {
node *new_node=(node*)calloc(1,sizeof(node));
strcpy(new_node->str,str);
new_node->count=1;
prev_tmp->next=new_node;
}

prev_char=str[0];

}

if(prev_char!=SENTINAL){
node *tmp=&node_list[prev_char];
node *prev_tmp=tmp;
tmp=tmp->next;
prev_tmp->next=NULL;
pthread_mutex_lock(&summarizer_lock);
while(tmp){
sprintf(summarizer_pool+size,"(%s,%d)\n",tmp->str,tmp->count);
//puts(summarizer_pool+size);
size+=strlen(tmp->str)+5;
prev_tmp=tmp;
tmp=tmp->next;
prev_tmp->next=NULL;
//free(prev_tmp);
}
pthread_mutex_unlock(&summarizer_lock);
}

pthread_mutex_unlock(&node_lock);
sem_post(&sem_write3);
//init to 0
sem_wait(&sem_read3);
sem_post(&sem_write2);
sem_post(&sem_read1);
memset(buffer,0,5*MAX_BUF);
}
return NULL;
}

void* wordCountWriter(void *a){
int done;
while(1) {
sem_wait(&sem_write2);
pthread_mutex_lock(&gen_read);
done=doneReading2;
if(done){
pthread_mutex_unlock(&gen_read);
FILE* fp=fopen("./wordCount.txt","a+");
pthread_mutex_lock(&summarizer_lock);
int i=0,ind;
while(summarizer_pool[i]) {
fputc(summarizer_pool[i],fp);
summarizer_pool[i]='\0';
i++;
}
pthread_mutex_unlock(&summarizer_lock);
fclose(fp);
sem_post(&sem_read2);
usleep(400);
for(ind=0;ind<no_of_mapper_thread;++ind)
pthread_cancel(mapper_thread[ind]);
for(ind=0;ind<no_of_reducer_thread;++ind)
pthread_cancel(reducer_thread[ind]);
for(ind=0;ind<no_of_summarizer_thread;++ind)
pthread_cancel(summarizer_thread[ind]);
pthread_cancel(letter_thread);
pthread_exit(0);
}
else {
pthread_mutex_unlock(&gen_read);
}

FILE* fp=fopen("./wordCount.txt","a+");
pthread_mutex_lock(&summarizer_lock);
int i=0;
while(summarizer_pool[i]) {
fputc(summarizer_pool[i],fp);
summarizer_pool[i]='\0';
i++;
}
pthread_mutex_unlock(&summarizer_lock);
fclose(fp);
sem_post(&sem_read2);
}
return NULL;
}

void* summarizer(void *a) {
char *str,buffer[5*MAX_BUF];
while(1) {
int count=0,i=0,i1=0,done,ind=0;
sem_wait(&sem_write3);
pthread_mutex_lock(&gen_read);
done=doneReading2;
if(done) {
doneReading3=1;
sem_post(&sem_write4);
sem_post(&sem_read3);
pthread_mutex_unlock(&gen_read);
pthread_exit(0);
}
else {
pthread_mutex_unlock(&gen_read);
}
sem_wait(&sem_read4);
pthread_mutex_lock(&summarizer_lock);
while(summarizer_pool[i]) {
buffer[i1++]=summarizer_pool[i++];
}
buffer[i1]='\0';
pthread_mutex_unlock(&summarizer_lock);
str=strtok(buffer,"\n");
char ch;
while(str){
if(str){count+=(str[strlen(str)-2]-'0');}
str+=1;
ch=str[0];
str=strtok(NULL,"\n");
}
i=0;
pthread_mutex_lock(&table_lock);
letter_table[ch-'a']+=count;
pthread_mutex_unlock(&table_lock);
sem_post(&sem_write4);
sem_post(&sem_read3);
}

}

void* write_letter(void *a) {
int i,done;
while(1) {
sem_wait(&sem_write4);
pthread_mutex_lock(&gen_read);
done=doneReading3;
if(done) {
pthread_mutex_unlock(&gen_read);
FILE *fs=fopen("./letterCount.txt","w+");
if(!fs) {
printf("File to write lettercount could not be opened\n");
sem_post(&sem_read4);
pthread_exit(0);
}
pthread_mutex_lock(&table_lock);
for(i=0;i<26;++i) {
if(letter_table[i]) {
fprintf(fs,"(%c,%d)\n",'a'+i,letter_table[i]);
}
}
pthread_mutex_unlock(&table_lock);
fclose(fs);
sem_post(&sem_read4);
}
else {
pthread_mutex_unlock(&gen_read);
}
FILE *fs=fopen("./letterCount.txt","w+");
if(!fs) {
printf("File to write lettercount could not be opened\n");
sem_post(&sem_read4);
pthread_exit(0);
}
pthread_mutex_lock(&table_lock);
for(i=0;i<26;++i) {
if(letter_table[i]) {
fprintf(fs,"(%c,%d)\n",'a'+i,letter_table[i]);
}
}
pthread_mutex_unlock(&table_lock);
fclose(fs);
sem_post(&sem_read4);
}

}
