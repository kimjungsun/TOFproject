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
char buffer1 = '0';   // 상대는 각각 1과 0으로 
char buffer2 ='1';
void *startSubserver(void *arg){
        int status ;
        pthread_t tid[5];
        int state ; // pthread_join parameter

        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock == -1)
               error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("192.168.7.33");
        serv_addr.sin_port = htons(7811);

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

        pthread_join(tid[0],(void **)&state);
        printf("쓰레드 write 종료\n");

        pthread_join(tid[1],(void **)&state);
        printf("쓰레드 read 종료\n");

        close(sock);
                return 0;
        }

void *thr_write(void *arg){
	printf("thr_write initialized\n");
	int sock = (int)*((int *)arg);
	while(1){
		if(buffer1 == '0')
		{
           write(sock,&buffer1,sizeof(buffer1));
		}
	}

}
void *thr_read(void *arg){
    char recv ;
	int sock = (int)*((int *)arg);
	printf("thr_read initialized\n");

	while(1){
	if(read(sock,&recv, sizeof(recv))!=0)
	buffer2 = recv ;
	}
	printf(" 쓰레드 파괴됨...\n");

}


void error_handling(char *message)

{
        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);

}
