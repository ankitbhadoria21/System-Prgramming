#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>
#define MAX_BUF 256

void reducer(FILE *fs)
{
typedef struct node{
char str[MAX_BUF];
int count;
struct node *next;
}node;

node node_list[26]={0};
char buffer[MAX_BUF];
char *str,prev_char='1';

while(fgets(buffer,MAX_BUF,fs)!=NULL){
str=strtok(buffer,",");
str+=1;

if(prev_char!='1' && str[0]!=prev_char){
node *tmp=&node_list[prev_char-'a'];
node *prev_tmp=tmp;
tmp=tmp->next;
prev_tmp->next=NULL;
while(tmp){
printf("(%s,%d)\n",tmp->str,tmp->count);
prev_tmp=tmp;
tmp=tmp->next;
prev_tmp->next=NULL;
free(prev_tmp);
}
}

node *tmp=&node_list[str[0]-'a'];
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

if(prev_char!='1'){
node *tmp=&node_list[prev_char-'a'];
tmp=tmp->next;
while(tmp){
printf("(%s,%d)\n",tmp->str,tmp->count);
tmp=tmp->next;
}
}
}

int main(int argc,char *argv[])
{
if(!strcmp(argv[1],"stdin"))
reducer(stdin);
else printf("Wrong File Stream Passed to reducer\n");
return 0;
}
