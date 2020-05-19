
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
	printf("thr_read initialized\n");

	while(1){
	if(read(sock,&recv, sizeof(recv))!=0)
            printf("Recieve from server:%c\n" ,recv);
	buffer2 = recv ;
	printf("BUFFER : %c\n",buffer2);
	}
	printf(" 쓰레드 파괴됨...\n");

}


void error_handling(char *message)

{
        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);

}
