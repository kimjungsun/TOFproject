

void *startSubserver(void *arg);
void configSubserver();


#ifdef _SUB_SERVER_ONLY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<pthread.h>
#include <stdio_ext.h>

void error_handling(char *message);
void *thr_read(void *arg);
void *thr_write(void *arg);


#endif
