#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include<pthread.h>

void error_handling(char *message);
void *thr_func(void *arg);
int clnt_sock1,clnt_sock2;
int clientsock[2] ;
char buffer;

int main(int argc, char *argv[])

{
        pthread_t tid[5];
	int status;
	int sock1,sock2;
	struct sockaddr_in serv_addr1,serv_addr2;
	struct sockaddr_in clnt_addr;
        int clnt_addr_size = sizeof(clnt_addr);
	int str_len;

	if (argc != 2)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	sock1 = socket(PF_INET, SOCK_STREAM, 0); //소켓을 생성하고 있다.
	sock2 = socket(PF_INET, SOCK_STREAM, 0); //소켓을 생성하고 있다.
	if (sock1== -1)
		error_handling("socket() error");
	if (sock2== -1)
		error_handling("socket() error");

	memset(&serv_addr1, 0, sizeof(serv_addr1));
	memset(&serv_addr2, 0, sizeof(serv_addr2));

	serv_addr1.sin_family = AF_INET;
	serv_addr1.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr1.sin_port = htons(atoi(argv[1]));
	serv_addr2.sin_family = AF_INET;
	serv_addr2.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr2.sin_port = htons(atoi(argv[1])+1);

    if(bind(sock1,(struct sockaddr*)&serv_addr1,sizeof(serv_addr1))==-1){
        printf("bind 1error\n"); 
        exit(1); }
    if(bind(sock2,(struct sockaddr*)&serv_addr2,sizeof(serv_addr2))==-1){
        printf("bind 2error\n"); 
        exit(1); }
       
        listen(sock1,5);
        listen(sock2,5);

        if((clnt_sock1 = accept(sock1,(struct sockaddr*)&clnt_addr,&clnt_addr_size))<0)
            error_handling("accept error");
        if((clnt_sock2 = accept(sock2,(struct sockaddr*)&clnt_addr,&clnt_addr_size))<0)
            error_handling("accept error");
        clientsock[0] = clnt_sock1;
	clientsock[1] = clnt_sock2;
	pthread_create(&tid[0],NULL,&thr_func,(void *)&clnt_sock1);
        pthread_create(&tid[1],NULL,&thr_func,(void *)&clnt_sock2);
	
	pthread_join(tid[0],(void **)&status);
	pthread_join(tid[1],(void **)&status);

	return 0 ; 
}
void *thr_func(void *arg){
	printf(" 쓰레드 생성됨 ~~\n");
int mysock = (int)*((int *)arg);
int hissock;
if( mysock == clientsock[0] ) 
	hissock = clientsock[1];
else 
	hissock = clientsock[0];
while(1){
if(read(mysock,&buffer,sizeof(buffer)!=0)){
	
        write(hissock,&buffer,sizeof(buffer));
	printf(" Writing  clnt_sock[%d] buffer : %c\n", mysock,buffer);
	  
}
}
}
        



void error_handling(char *message)

{

	fputs(message, stderr);

	fputc('\n', stderr);

	exit(1);

}
