#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define SENTINAL '%'
#define MAX_BUF 256

char reducer_pool[5*MAX_BUF]="(fsjka,1) (fsjka,1) (fsjka,1) (fsjka,1) (fsjka,1) (fhsjdkhda,1) (jkdka,1) (jkdka,1) (hjfhsjf,1) (hjfhsjf,1) (hjfhsjf,1) (hajfhaj,1) (hajfhaj,1) (hajfhaj,1) (hajfhaj,1) (fsjka,1) (faj,1)";
pthread_mutex_t reducer_lock=PTHREAD_MUTEX_INITIALIZER;
char summarizer_pool[5*MAX_BUF];
pthread_mutex_t summarizer_lock=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_read2,sem_write2;

typedef struct node{
char str[MAX_BUF];
int count;
struct node *next;
}node;

pthread_mutex_t node_lock=PTHREAD_MUTEX_INITIALIZER;
node node_list[256]={0};

void reducer()
{
char buffer[5*MAX_BUF];
char *str,prev_char=SENTINAL;
char *tmp,*word_list[MAX_BUF];

sem_wait(&sem_read2);
prev_char=SENTINAL;
int i=0,i1=0,index=0,ttl_wrd=0,size=0;

pthread_mutex_lock(&reducer_lock);
while(reducer_pool[i1]) {
buffer[i++]=reducer_pool[i1++];
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
/*
int i;
for(i=0;summarizer_pool[i];++i)
printf("%c",summarizer_pool[i]);
printf("\n");
*/
size+=strlen(tmp->str)+5;
tmp=tmp->next;
prev_tmp->next=NULL;
free(prev_tmp);
}
pthread_mutex_unlock(&summarizer_lock);
}

node *tmp=&node_list[str[0]];
node *prev_tmp;

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
//size=0;
node *tmp=&node_list[prev_char];
tmp=tmp->next;
node *prev_tmp;
pthread_mutex_lock(&summarizer_lock);
while(tmp){
sprintf(summarizer_pool+size,"(%s,%d)\n",tmp->str,tmp->count);
//puts(summarizer_pool+size);
size+=strlen(tmp->str)+5;
prev_tmp=tmp;
tmp=tmp->next;
free(prev_tmp);
}
pthread_mutex_unlock(&summarizer_lock);
}
pthread_mutex_unlock(&node_lock);
sem_post(&sem_write2);
}


void wordCountWriter(){
sem_wait(&sem_write2);
FILE* fp=fopen("./wordCount.txt","a+");
pthread_mutex_lock(&summarizer_lock);
int i=0;
while(summarizer_pool[i]) {
fputc(summarizer_pool[i++],fp);
}
pthread_mutex_unlock(&summarizer_lock);
fclose(fp);
sem_post(&sem_read2);
}

int main()
{
sem_init(&sem_write2,0,0);
sem_init(&sem_read2,0,1);
reducer();
wordCountWriter();
}
