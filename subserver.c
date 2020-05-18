#include "sub_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<pthread.h>
// 전역 변수 선언 ?
int gStartAmpDistVideo;
char buffer2 = '0';

void *startSubserver(void *arg)
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
        serv_addr.sin_addr.s_addr = inet_addr("192.168.1.129");
        serv_addr.sin_port = htons(7777);

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

        pthread_join(tid[1],(void **)&state);

        close(sock);
                return 0;
        }

void *thr_write(void *arg){
	int sock = (int)*((int *)arg);
    char send_meg = '0';
	while(1){
		if(gStartAmpDistVideo == 0)
		{
	       printf(" gStartAmpDistVideo is 0 now!\n");
           write(sock,&send_meg,sizeof(send_meg));
		}
	}

}
void *thr_read(void *arg){
    char recv ;
	int sock = (int)*((int *)arg);
	while(1){
	if(read(sock,&recv, sizeof(recv))!=0)
            printf("Recieve from server:%c\n" ,recv);
	buffer2 = recv ;
	}

}


void error_handling(char *message)

{
        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);

}
