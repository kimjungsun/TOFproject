#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char c_message[7]= "abcdef";
	char message[10];
	int str_len;
	if (argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

      for(int i = 0 ; i<5;i++){
	sock = socket(PF_INET, SOCK_STREAM, 0); //소켓을 생성하고 있다.
	if (sock == -1)
		error_handling("socket() error");
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) //connect 함수호출을 통해서 서버 프로그램에 연결 요청
		error_handling("connect() error");
	str_len = read(sock,message, sizeof(message) - 1);
        write(sock,&c_message[i],sizeof(c_message[i]));
	if (str_len == -1)
		error_handling("read() error");
	printf("Message from server:%s\n", message);
close(sock);
}	

	return 0;

}



void error_handling(char *message)

{

	fputs(message, stderr);

	fputc('\n', stderr);

	exit(1);

}
