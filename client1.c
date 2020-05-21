#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<pthread.h>
void error_handling(char *message);
void *thr_read(void *arg);
void *thr_write(void *arg);
char buffer1 = '1';
char buffer2 = '1';

int main(int argc, char* argv[])
{
        int sock;
        struct sockaddr_in serv_addr;

        int status ;
        pthread_t tid[5];
        int state ; // pthread_join parameter

        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock == -1)
               error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
        serv_addr.sin_port = htons(atoi(argv[2]));

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
               error_handling("connect() error");

        if((status = pthread_create(&tid[0],NULL,&thr_read,(void *)&sock))!=0){
			printf("READ THREAD CREATE FAIL !\n");
			exit(0);
			}
        if((status = pthread_create(&tid[1],NULL,&thr_write,(void *)&sock))!=0){
       			printf("READ THREAD CREATE FAIL !\n");
       			exit(0);
       			}
	printf("after thread : buffer2 : %c\n",buffer2);

        while(1){
	if(buffer2 == '0'){
		buffer2 = '1';
		printf("진행중 ~ \n");
	
	buffer1 = '0';
	usleep(1000);
	}
	}
        pthread_join(tid[0],(void **)&state);

        pthread_join(tid[1],(void **)&state);

        close(sock);
                return 0;
        }

void *thr_write(void *arg){
	int sock = (int)*((int *)arg);
	int c;
	while(1){
		if(buffer1 == '0'){
                  write(sock,&buffer1,sizeof(buffer1));
		  printf("0 sending ~\n");
		}}
}
void *thr_read(void *arg){
	char recv;
	int sock = (int)*((int *)arg);
	while(1){
	if(read(sock,&recv, sizeof(recv))!=0);
	{
		printf("0 receiving~\n");
	       	buffer2 = recv;
	}
}
}

void error_handling(char *message)

{
        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);

}
