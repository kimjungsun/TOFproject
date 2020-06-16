#define _SUB_SERVER_ONLY
#define _SUBSERVER_AND_SERVER_SHARE

#include "sub_server.h"
#include "server.h"

int dualsock;
char ellum_buf_own;
char ellum_buf_other;
int tcp_port;
pthread_t subserver;
int dualmode;

void configSubserver(){

	char mode ='S';
	char ellum_order;
	//printf(" DUAL SENSOR MODE[D]| SINGLE SENSOR MODE[S]  :");

    switch(mode){

    case 'S' :
    	printf("Initializing Single Mode\n");
        dualmode = 0;
        break;
    case 'D' :
    	printf("Initializing Dual Mode\n");
        dualmode = 1;

        printf("Set the port address : ");
        while(1){
        	  tcp_port = getchar();
       	        if(tcp_port == '\n') continue;
       	        else    getchar();
       	    }


        printf(">>> ELLUMINATION SETTING PRIMARY[P] | SECONDARY[S] :");
    	 while(1){

    	        	  ellum_order = getchar();
    	       	        if(ellum_order == '\n') continue;
    	       	        else    getchar();
    	       	    }
        if(ellum_order == 'P' || ellum_order == 'p'){
            printf("Set as Primary\n ");
        	gStartAmpDistVideo = 1;
        	ellum_buf_own = '1';
			ellum_buf_other = '0';
        }
        else if(ellum_order == 'S' || ellum_order == 's'){
            printf("Set as Secondary\n ");
        	gStartAmpDistVideo = 0;
        	ellum_buf_own = '0';
			ellum_buf_other = '1';

        }
        else{
            printf("Unable to set as the choice. Set as the default setting(Primary)\n ");
               	gStartAmpDistVideo = 1;
               	ellum_buf_own = '1';
       			ellum_buf_other = '0';
               }

    	if(pthread_create(&subserver,NULL,&startSubserver,(void *)NULL)!=0){
    		       			printf("SUBSERVER THREAD CREATE FAIL. \n");
    		       			exit(0);
    		       			}
    	printf(">>>startSubserver\n");
        break;
    default :
    	printf("Invalid Mode, initializing Single mode as a default mode. \n");
    	break;
    }

}
void *startSubserver(void *arg)
{

        dualsock = socket(PF_INET, SOCK_STREAM, 0);   // TCP MODE : SOCK_STREAM / UDP MODE : SOCK_DGRAM
               if (dualsock == -1)
                      error_handling("socket() error");


        int status ;
        pthread_t tid[2];
        int state ; // pthread_join parameter

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("192.168.7.33");
        serv_addr.sin_port = htons(tcp_port);

        printf("IP Address set as 192.168.7.33  & Port Address set as %d\n ", tcp_port);

        if (connect(dualsock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
               error_handling("connect() error");

        if((status = pthread_create(&tid[0],NULL,&thr_read,(void *)&dualsock))!=0){
			printf("READ THREAD CREATE FAIL !\n");
			exit(0);
			}

        pthread_join(tid[0],(void **)&state);

        close(dualsock);
        return 0;
}

void *thr_read(void *arg){
    char recv ;
	int sock = (int)*((int *)arg);
	printf("thread_read initialized (SUBSERVER) \n");

	while(1){
	if(read(sock,&recv,sizeof(recv))!=0)
		ellum_buf_other = recv ;
	}

}

void error_handling(char *message)

{
        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);
}
