#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define MAX_BUF 256

void mapper(FILE *fs)
{
char buffer[MAX_BUF];

while(fgets(buffer,MAX_BUF,fs)!=NULL){
char *pointToCh;
pointToCh=strtok(buffer," \n");

while(pointToCh!=NULL)
{
printf("(%s,1)\n",pointToCh);
pointToCh=strtok(NULL," \n");
}

}
}

int main(int argc, char *argv[])
{
FILE *fs=fopen(argv[1],"r");
mapper(fs);
fclose(fs);
return 0;
}
