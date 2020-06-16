#define _SERVER_H
#include "boot.h"
#include "i2c.h"
#include "config.h"
#include "server.h"
#include "version.h"
#include "evalkit_illb.h"
#include "is5351A_clkGEN.h"
#include "sub_server.h"            // YNG_JSKIM 200612
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char* argv[]) {


	printf(">>> check version\n");

	printf("Version %i.%i.%i\n", getMajor(), getMinor(), getPatch());
	printf(">>> boot \n");

	// 0x22 현재 default 설정됨 (boot.c 참고)
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
	printf(">>> configSubserver\n");

	configSubserver();   // YNG_JSKIM 200612


	printf(">>> startServer\n");
	startServer(deviceAddress);


	return 0;
}
