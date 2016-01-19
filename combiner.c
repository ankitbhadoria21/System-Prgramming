#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>
#define MAX_BUF 256

int main(int argc,char *argv[])
{
int fd[2];
pid_t pid1,pid2;

char file_to_read[MAX_BUF];
printf("Input absolute path of the file to read\n");
scanf("%s",file_to_read);

pipe(fd);
pid1=fork();
if(pid1>0)
{
pid2=fork();
if(pid2>0)
{
close(fd[0]);
close(fd[1]);
waitpid(pid1,NULL,0);
waitpid(pid2,NULL,0);
}
else {
char cwd[MAX_BUF+8];
getcwd(cwd,MAX_BUF);
strcat(cwd,"/reducer");
char *arg[]={cwd,"stdin",NULL};
close(fd[1]);
dup2(fd[0],0);
execvp(cwd,arg);
close(fd[0]);
}

}
else {
char cwd[MAX_BUF+7];
getcwd(cwd,MAX_BUF);
strcat(cwd,"/mapper");
char *arg[]={cwd,file_to_read,NULL};
close(fd[0]);
dup2(fd[1],1);
execvp(cwd,arg);
close(fd[1]);
}
return 0;
}
