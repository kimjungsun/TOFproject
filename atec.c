#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 153600
void error_handling(char *message);

int main(int argc, char* argv[])
{

	// FILE I/O 
	FILE *fp1;
	FILE *fp2;
	int index;
	int data;
	fp1 = fopen("/home/jskim/distance.txt","w");
	fp2 = fopen("/home/jskim/amplitude.txt","w");

        uint16_t stream[BUF_SIZE];
	memset(stream, 0, sizeof(stream));
        int sock;
        struct sockaddr_in serv_addr;
	socklen_t  serv_addr_size = sizeof(serv_addr);
        sock = socket(PF_INET, SOCK_STREAM, 0);//TCP
       // sock = socket(PF_INET, SOCK_DGRAM, 0);//UDP
        if (sock == -1)
               error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
        serv_addr.sin_port = htons(atoi(argv[2]));

	//TCP 
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
               error_handling("connect() error");
        //UDP
	/*
	char buf[10] = "HELLO";	 
       	int val=0;
  */
int i = 0;
while(1){
	/*val = sendto(sock,buf,strlen(buf),0,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
        if(val == -1 ) printf("send to error!\n");
	else printf("Well transported\n");
        val = recvfrom(sock,stream,sizeof(stream),0,(struct sockaddr*)&serv_addr,&serv_addr_size);//udp
          if(val == -1) printf("recv from error\n");     
              else printf("Well Got!\n");
*/
	read(sock,stream,sizeof(stream));
	if(i==10000){

	    fwrite(stream,2,76800*2,fp1);
	    //fwrite(stream+76800,2,76800,fp2);
		printf("fwrite completed\n");
	    fclose(fp1);
	    fclose(fp2);
	}
	i++;
	      }

        close(sock);

return 0;
        }

void error_handling(char *message)

{
        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);

}
