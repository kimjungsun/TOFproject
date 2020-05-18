#include "boot.h"
#include "i2c.h"
#include "config.h"
#include "server.h"
#include "version.h"
#include "evalkit_illb.h"
#include "is5351A_clkGEN.h"
#include "sub_Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char* argv[]) {
	pthread_t subserver;
	int status;
	printf(">>> check version\n");
	printf("Version %i.%i.%i\n", getMajor(), getMinor(), getPatch());
	printf(">>> boot \n");
	int deviceAddress = boot();
	printf("deviceAddress confirmed = 0x%x\n", deviceAddress);

	illuminationDisable();
	printf(">>> configSetDeviceAddress \n");
	configSetDeviceAddress(deviceAddress);
	printf(">>> configInit \n");
	configInit(deviceAddress);
	printf(">>> usleep \n");
	usleep(10000);
	illuminationEnable();
	if(pthread_create(&subserver,NULL,&startSubserver,(void *)NULL)!=0){
	       			printf("MAIN- SUBSERVER THREAD CREATE FAIL !\n");
	       			exit(0);
	       			}
	printf(">>>startSubserver\n");
	printf(">>> startServer\n");
	startServer(deviceAddress);
	// pthread_join(subserver,(void **)&status);
	return 0;
}
