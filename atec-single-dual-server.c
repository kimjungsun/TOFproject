#define _GNU_SOURCE
#define _SERVER_ONLY
#define _SERVER_H
#define _SUBSERVER_AND_SERVER_SHARE

#include "server.h"
#include "sub_server.h"    // YNG_JSKIM 200612

char ellum_buf_own;
char ellum_buf_other;   // Çì´õ¿¡¼­ externÀ¸·Î ¼±¾ðÇÔ. Áö±ÝÀº ±Û·Î¹úº¯¼ö.
int gStartAmpDistVideo;
int dualsock;
int dualmode;

/**************************************************** º¯¼ö/±¸Á¶Ã¼   ********************************************************/
static pthread_t imageThread;
static int gStartGrayVideo    = 0;
static int gStartDCSVideo     = 0;
static int gStartDCSTOFandGrayVideo = 0;
//int gStartAmpDistVideo; Çì´õ·Î ÀÌµ¿.

static int gStartDistVideo    = 0;
static int gStartAmpVideo     = 0;
static int16_t gRunVideo      = 0;

enum threadType{
	StartGrayVideo,
	StartAmpVideo,
	StartDistVideo,
	StartAmpDistVideo,
	StartDCSVideo,
	StartDCSTOFandGrayVideo,
	AllVideo
};

typedef struct {
	int (* func)(uint16_t**);
}funPointer;

int pid ;
static unsigned int deviceAddress;
unsigned char* pData;
int gSock = 0;

int16_t temps[6];
int16_t chipInfo[8];
static int dataSizeThread = 0;
static int stopThreadFlag = 1;
/*************************************************************************************************************************/

//UDP_megan
/*
struct sockaddr_in client_addr;
socklen_t client_addr_size;
char buf[10];
	int recvlen = 0 ;
*/
/************************************************* SOCKET CONFIG   ********************************************************/
int sockfd, newsockfd;
int atecfd, newatecfd;   // atec_ver. 200612 YNG_JSKIM

socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

/*****************ATEC VER*******************/
socklen_t atec_clilen;
struct sockaddr_in atec_serv_addr, atec_cli_addr;


/************************************************imagingThread()*************************************************************/

void imagingThread(int (* funcThread)(uint16_t**)){
	uint16_t* pMemm=NULL;
	stopThreadFlag = 0;

	while(gStartGrayVideo==1 || gStartAmpVideo==1 || gStartDistVideo==1 || gStartAmpDistVideo==1 || gStartDCSVideo==1 || gStartDCSTOFandGrayVideo==1){
		dataSizeThread = 2 * funcThread(&pMemm);
		enqueue(&pMemm, dataSizeThread);
	};

	stopThreadFlag = 1;
	printf("imagingThread: stopThreadFlag = %d\n", stopThreadFlag);	//TODO remove
}
/*************************************************************************************************************************/





/************************************************startServer()*************************************************************/

int startServer(const unsigned int addressOfDevice) {

	deviceAddress = addressOfDevice;
	signal(SIGQUIT, signalHandler); // add signal to signalHandler to close socket on crash or abort
	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	signal(SIGSEGV, signalHandler);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	atecfd = socket(AF_INET, SOCK_STREAM, 0);      // TCP
	//UDP atecfd = socket(AF_INET, SOCK_DGRAM, 0);


	if (sockfd < 0) {
	return -1;
	}

	if (atecfd < 0) {
		return -1;
		}
	// set SO_REUSEADDR on a socket to true
	int optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0) {
	printf("Error setting socket option\n");
	return -1;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(TCP_PORT_NO);

	bzero((char *) &atec_serv_addr, sizeof(atec_serv_addr));
		atec_serv_addr.sin_family = AF_INET;
		atec_serv_addr.sin_addr.s_addr = INADDR_ANY;
		atec_serv_addr.sin_port = htons(9999);
		printf("ATEC PORT default is 9999\n");

/****************BIND()*******************/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	printf("Binding error: please try again in 20sec and make sure no other instance is running\n");
	return -1;
	}
	printf("Socket successfully opened\n");

	if (bind(atecfd, (struct sockaddr *) &atec_serv_addr, sizeof(atec_serv_addr)) < 0) {
	printf("Binding ATEC error: please try again in 20sec and make sure no other instance is running\n");
	return -1;
	}
/*******************************************/

/****************LISTEN()******************/
	listen(sockfd, 5);

	listen(atecfd, 1);
/*******************************************/

/****************ACCPET()******************/

clilen = sizeof(cli_addr);
atec_clilen = sizeof(atec_cli_addr);

	newatecfd = accept(atecfd, (struct sockaddr *) &atec_cli_addr, &atec_clilen);
	if (newatecfd < 0) {
		printf("Unable to accept ATEC connection. Try again after rebooting. \n");
		return -1;
		}
	printf("ATEC socket connected. Ready for communication! \n");

	printf("waiting for requests\n");

	while (1) {
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0) {
	printf("couldn't accept\n");
	return -1;
	}
/********************************************/

	handleRequest(newsockfd,newatecfd);
	close(newsockfd);
	}

	close(newatecfd);
	close(sockfd);
	close(atecfd);

	return 0;
}
/*************************************************************************************************************************/




/************************************************handleRequest()*************************************************************/

void handleRequest(int sock, int atecsock ) {
	//bbbLEDEnable(3, 1);
	char buffer[TCP_BUFFER_SIZE];
	bzero(buffer, TCP_BUFFER_SIZE);
	int size = read(sock, buffer, TCP_BUFFER_SIZE);
	gSock = sock;
	if (buffer[size - 1] == '\n'){
		buffer[size - 1] = '\0';
	}
	char stringArray[MAX_COMMAND_ARGUMENTS][MAX_COMMAND_ARGUMENT_LENGTH];
	int argumentCount = helperParseCommand(buffer, stringArray);
	unsigned char response[TCP_BUFFER_SIZE];
	bzero(response, TCP_BUFFER_SIZE);
	int16_t answer;

	//COMMANDS
	if ((strcmp(stringArray[0], "readRegister") == 0 || strcmp(stringArray[0], "read") == 0 || strcmp(stringArray[0], "r") == 0) && argumentCount >= 1 && argumentCount < 3) {
		unsigned char *values;
		int v;
		int nBytes;
		if (argumentCount == 1) {
			nBytes = 1;
		} else {
			nBytes = helperStringToHex(stringArray[2]);
		}
		int registerAddress = helperStringToHex(stringArray[1]);
		if (nBytes > 0 && registerAddress >= 0) {
			values = malloc(nBytes * sizeof(unsigned char));
			int16_t responseValues[nBytes];
			apiReadRegister(registerAddress, nBytes, values, deviceAddress);
			for (v = 0; v < nBytes; v++) {
				responseValues[v] = values[v];
			}
			send(sock, responseValues, nBytes * sizeof(int16_t), MSG_NOSIGNAL);
			free(values);
		} else {
			answer = -1;
			send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
		}
	} else if ((strcmp(stringArray[0], "writeRegister") == 0 || strcmp(stringArray[0], "write") == 0 || strcmp(stringArray[0], "w") == 0) && argumentCount > 1) {
		unsigned char *values = malloc((argumentCount - 1) * sizeof(unsigned char));
		int registerAddress = helperStringToHex(stringArray[1]);
		int i;
		int ok = 1;
		for (i = 0; i < argumentCount - 1; i++) {
			if (helperStringToHex(stringArray[i + 2]) >= 0) {
				values[i] = helperStringToHex(stringArray[i + 2]);
			} else {
				ok = 0;
			}
		}
		if (registerAddress > 0 && ok > 0) {
			answer = apiWriteRegister(registerAddress, argumentCount - 1, values, deviceAddress);
		} else {
			answer = -1;
		}
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
		free(values);
	}	else if (strcmp(stringArray[0], "enableImaging") == 0 && argumentCount == 1) {
		answer = apiSetEnableImaging(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);

		/********************************getDistanceAndAmplitudeSorted()************************************/

	}else if (strcmp(stringArray[0], "getDistanceAndAmplitudeSorted") == 0 && !argumentCount && dualmode ){

		uint16_t* pMem=NULL;
		serverStopThreads(StartAmpDistVideo);

		if(gRunVideo){
				while(1){
					if(ellum_buf_other == '0')
							break;
						}
						ellum_buf_own = '1';
				        write(dualsock,&ellum_buf_own,sizeof(ellum_buf_own));

						queueInit();	// reset queue buffer
						pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetDistanceAndAmplitudeSorted); // image queueing
						while(isQueueEmpty() == true){
							usleep(1000);
						}
						dequeue(&pMem);

						send(sock, pMem, dataSizeThread, MSG_NOSIGNAL);    // GUI SOCKET TCP/IP
						send(atecsock, pMem, dataSizeThread, MSG_NOSIGNAL);   //ATEC SOCKET TCP/IP 200612 YNG_JSKIM

						serverStopThreads(AllVideo);    // TURN OFF ELLUMINATION

						ellum_buf_own = '0';
						write(dualsock,&ellum_buf_own,sizeof(ellum_buf_own));
						while(1){
						if(ellum_buf_other == '1')
							break;
							}

					}else{
						int dataSize = 2 * apiGetDistanceAndAmplitudeSorted(&pMem);
						send(sock, pMem, dataSize, MSG_NOSIGNAL);
						send(atecsock, pMem, dataSizeThread, MSG_NOSIGNAL);  //ATEC SOCKET TCP/IP 200612 YNG_JSKIM
					}


		// *********************UDP************************ //
					/*recvfrom(atecfd, buf, BUFSIZE , MSG_NOSIGNAL,(struct sockaddr *)&client_addr, &client_addr_size);
								printf("received message :%s \n",buf);
							int ret=0;
							ret = sendto(atecsock, pMem, dataSizeThread+1, MSG_NOSIGNAL,(struct sockaddr*)&client_addr,sizeof(client_addr_size));

							if(ret== -1){
				            	printf("send error(%d)\n",ret);
				            }*/   //UDP
		/*********************************************************************************************************/
	}else if (strcmp(stringArray[0], "getDistanceAndAmplitudeSorted") == 0 && !argumentCount && !dualmode){
      printf("enter Non Dual - Atec mode \n");
				uint16_t* pMem=NULL;
				serverStopThreads(StartAmpDistVideo);

				if(gRunVideo == 1){
							if(gStartAmpDistVideo == 0 ){
								gStartAmpDistVideo = 1;
								queueInit();	// reset queue buffer
								pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetDistanceAndAmplitudeSorted);
							}

							while(isQueueEmpty() == true)	usleep(10000);
							dequeue(&pMem);
							send(sock, pMem, dataSizeThread, MSG_NOSIGNAL);
							send(atecsock, pMem, dataSizeThread, MSG_NOSIGNAL);  //ATEC SOCKET TCP/IP 200612 YNG_JSKIM

							}else{
								int dataSize = 2 * apiGetDistanceAndAmplitudeSorted(&pMem);
								send(sock, pMem, dataSize, MSG_NOSIGNAL);
								send(atecsock, pMem, dataSizeThread, MSG_NOSIGNAL);  //ATEC SOCKET TCP/IP 200612 YNG_JSKIM
							}

	}else if (strcmp(stringArray[0], "startVideo") == 0 && !argumentCount) {
		gRunVideo = 1;
		send(sock, &gRunVideo, sizeof(int16_t), MSG_NOSIGNAL);

	}else if (strcmp(stringArray[0], "stopVideo") == 0 && !argumentCount) {
		serverStopThreads(AllVideo);
		gRunVideo = 0;
		send(sock, &gRunVideo, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "enableIllumination") == 0 && argumentCount == 1) {
		if (!helperStringToInteger(stringArray[1])){
			illuminationDisable();
			answer = 0;
		} else {
			illuminationEnable();
			answer = 1;
		}
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setABS") == 0 && argumentCount == 1) {
		answer = apiEnableABS(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableVerticalBinning") == 0 && argumentCount == 1) {
		answer = apiEnableVerticalBinning(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableHorizontalBinning") == 0 && argumentCount == 1) {
		answer = apiEnableHorizontalBinning(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setRowReduction") == 0 && argumentCount == 1) {
		answer = apiSetRowReduction(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "loadConfig") == 0 && argumentCount == 1) {
		serverStopThreads(AllVideo);
		answer = apiLoadConfig(helperStringToInteger(stringArray[1]), deviceAddress);
		if (answer == 1){
			illuminationEnable();
			printf("illumination enabled\n");
		}else{
			illuminationDisable();
			printf("illumination disable\n");
		}
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "setIntegrationTime2D") == 0 && argumentCount == 1) {
		answer = apiSetIntegrationTime2D(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setIntegrationTime3D") == 0 && argumentCount == 1) {
		answer = apiSetIntegrationTime3D(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setIntegrationTime3DHDR") == 0 && argumentCount == 1) {
		answer = apiSetIntegrationTime3DHDR(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getImagingTime") == 0 && argumentCount == 0) {
		answer = apiGetImagingTime();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "filterMode") == 0 && argumentCount == 1) {
		answer=apiEnableSquareAddDcs(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "nloopFilter") == 0 && argumentCount == 1) {
		answer=apiSetNfilterLoop(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableArgCheck") == 0 && argumentCount == 1) {
		answer=apiEnableAddArgThreshold(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setAddArgThreshold") == 0 && argumentCount == 1) {
		answer = apiSetAddArgThreshold(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setAddArgMin") == 0 && argumentCount == 1) {
		answer = apiSetAddArgMin(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setAddArgMax") == 0 && argumentCount == 1) {
		answer = apiSetAddArgMax(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setMinAmplitude") == 0 && argumentCount == 1) {
		answer = apiSetMinAmplitude(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getMinAmplitude") == 0 && argumentCount == 0) {
		answer = apiGetMinAmplitude();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableDualMGX") == 0 && argumentCount == 1) {
		answer = apiEnableDualMGX(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableHDR") == 0 && argumentCount == 1) {
		answer = apiEnableHDR(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enablePiDelay") == 0 && argumentCount == 1) {
		answer = apiEnablePiDelay(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setModulationFrequency") == 0 && argumentCount == 1) {
		int16_t calibrationType = 0;
		enum threadType typeLastPaused=AllVideo;
		if(gRunVideo)
			typeLastPaused=pauseVideo();
		answer = apiSetModulationFrequency(helperStringToInteger(stringArray[1]), deviceAddress);
		calibrationType=apiGetModulationFrequencyCalibration(helperStringToInteger(stringArray[1]));
		if(calibrationType<0){
			apiCorrectDRNU(0);
		}
		if(gRunVideo)
			resumeVideo(typeLastPaused);

		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getModulationFrequencies") == 0 && argumentCount == 0) {
		int *answerp = apiGetModulationFrequencies();
		send(sock, answerp, configGetModFreqCount() * sizeof(int), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getCalibrationTypeForFreqIdx") == 0 && argumentCount == 1) {
		answer = apiGetModulationFrequencyCalibration(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setROI") == 0 && argumentCount == 4) {
		answer = apiSetROI(helperStringToInteger(stringArray[1]), helperStringToInteger(stringArray[2]), helperStringToInteger(stringArray[3]), helperStringToInteger(stringArray[4]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setOffset") == 0 && argumentCount == 1) {
		answer = apiSetOffset(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getOffset") == 0) {
		answer = apiGetOffset(deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableDefaultOffset") == 0 && argumentCount == 1) {
		enum threadType typeLastPaused=AllVideo;
		if(gRunVideo)
			typeLastPaused=pauseVideo();
		answer = apiEnableDefaultOffset(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
		if(gRunVideo)
			resumeVideo(typeLastPaused);
	}else if (strcmp(stringArray[0], "getBadPixels") == 0) {
		int16_t *badPixels = malloc(100000 * sizeof(int16_t));
		answer = apiGetBadPixels(badPixels);
		if (answer < 0) send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
		else			send(sock, badPixels, answer * sizeof(int16_t), MSG_NOSIGNAL);
		free(badPixels);
	} else if (strcmp(stringArray[0], "getTemperature") == 0) {
		int16_t size = apiGetTemperature(deviceAddress, temps);
		send(sock, &temps, size * sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getAveragedTemperature") == 0) {
		int16_t answer = apiGetAveragedTemperature(deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getChipInfo") == 0) {
		int16_t size = apiGetChipInfo(deviceAddress, chipInfo);
		send(sock, &chipInfo, size * sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setHysteresis") == 0 && argumentCount == 1) {
		answer = apiSetHysteresis(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "selectMode") == 0 && argumentCount == 1) {
		answer = apiSelectMode(helperStringToInteger(stringArray[1]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "selectPolynomial") == 0 && argumentCount == 2) {
		answer = apiSelectPolynomial(helperStringToInteger(stringArray[1]), helperStringToInteger(stringArray[2]), deviceAddress);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "enableImageCorrection") == 0 && argumentCount == 1) {
		answer = apiEnableImageCorrection(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setImageAveraging") == 0 && argumentCount == 1) {
		answer = apiSetImageAveraging(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setImageProcessing") == 0 && argumentCount == 1) {
		answer = apiSetImageProcessing(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setImageDifferenceThreshold") == 0 && argumentCount == 1) {
		answer = apiSetImageDifferenceThreshold(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "getIcVersion") == 0 && argumentCount == 0) {
		answer = apiGetIcVersion();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "getPartVersion") == 0 && argumentCount == 0) {
		answer = apiGetPartVersion();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "enableSaturation") == 0 && argumentCount == 1) {
		answer = apiEnableSaturation(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "enableAdcOverflow") == 0 && argumentCount == 1) {
		answer = apiEnableAdcOverflow(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "isFLIM") == 0 && !argumentCount) {
		answer = apiIsFLIM();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetT1") == 0 && argumentCount == 1) {
		answer = apiFLIMSetT1(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetT2") == 0 && argumentCount == 1) {
		answer = apiFLIMSetT2(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetT3") == 0 && argumentCount == 1) {
		answer = apiFLIMSetT3(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetT4") == 0 && argumentCount == 1) {
		answer = apiFLIMSetT4(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetTREP") == 0 && argumentCount == 1) {
		answer = apiFLIMSetTREP(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetRepetitions") == 0 && argumentCount == 1) {
		answer = apiFLIMSetRepetitions(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetFlashDelay") == 0 && argumentCount == 1) {
		answer = apiFLIMSetFlashDelay(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMSetFlashWidth") == 0 && argumentCount == 1) {
		answer = apiFLIMSetFlashWidth(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "FLIMGetStep") == 0 && argumentCount == 0) {
		answer = apiFLIMGetStep();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "searchBadPixels") == 0 && argumentCount == 0) {
		if (calibrationSearchBadPixelsWithMin() < 0) answer = -1;
		else answer = 0;
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	}else if (strcmp(stringArray[0], "dumpAllRegisters") == 0 && argumentCount == 0) {
		int16_t dumpData[256];
		dumpAllRegisters(deviceAddress, dumpData);
		send(sock, dumpData, sizeof(int16_t) * 256, MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "isFlimCorrectionAvailable") == 0 && argumentCount == 0) {
		answer = apiIsFlimCorrectionAvailable();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "calibrateFlim") == 0  && argumentCount == 1) {
		answer = apiCalibrateFlim(stringArray[1]);
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctFlimOffset") == 0 && argumentCount == 1) {
		answer = apiCorrectFlimOffset(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctFlimGain") == 0 && argumentCount == 1) {
		answer = apiCorrectFlimGain(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "calibrateGrayscale") == 0 && argumentCount == 1) {
		answer = apiCalibrateGrayscale(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctGrayscaleOffset") == 0 && argumentCount == 1) {
		answer = apiCorrectGrayscaleOffset(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctGrayscaleGain") == 0 && argumentCount == 1) {
		answer = apiCorrectGrayscaleGain(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "calibrateDRNU") == 0 && argumentCount == 0) {
		answer = apiCalibrateDRNU(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctDRNU") == 0 && argumentCount == 1) {
		answer = apiCorrectDRNU(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setDRNUDelay") == 0 && argumentCount == 1) {
			answer = apiSetDRNUDelay(helperStringToInteger(stringArray[1]));
			send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setDRNUDiffTemp") == 0 && argumentCount == 1) {
			answer = apiSetDRNUDiffTemp(helperStringToDouble(stringArray[1]));
			send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setDRNUAverage") == 0 && argumentCount == 1) {
			answer = apiSetDRNUAverage(helperStringToInteger(stringArray[1]));
			send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setPreheatTemp") == 0 && argumentCount == 1) {
		answer = apiSetpreHeatTemp(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctTemperature") == 0 && argumentCount == 1) {
		answer = apiCorrectTemperature(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "correctAmbientLight") == 0 && argumentCount == 1) {
			answer = apiCorrectAmbientLight(helperStringToInteger(stringArray[1]));
			send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "renewDRNU") == 0 && argumentCount == 1) {
			int16_t* data = apiRenewDRNU(helperStringToInteger(stringArray[1]));
			send(sock, data, 100 * sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "showDRNU") == 0 && argumentCount == 0) {
			int16_t* data = apiShowDRNU();
			send(sock, data, 100 * sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "loadTemperatureDRNU") == 0 && argumentCount == 0) {
			int16_t* data = apiLoadTemperatureDRNU();
			send(sock, data, 50 * sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "isGrayscaleCorrectionAvailable") == 0 && argumentCount == 0) {
		answer = apiIsEnabledGrayscaleCorrection();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);\
	} else if (strcmp(stringArray[0], "isDRNUAvailable") == 0 && argumentCount == 0) {
		answer = apiIsEnabledDRNUCorrection();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "isTemperatureCorrectionEnabled") == 0 && argumentCount == 0) {
		answer = apiIsEnabledTemperatureCorrection();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanTempK") == 0 && argumentCount == 1) {
		double value = apiSetChipTempSimpleKalmanK(helperStringToDouble(stringArray[1]));
		send(sock, &value, sizeof(double), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getKalmanTempK") == 0 && argumentCount == 0) {
		double value = apiGetChipTempSimpleKalmanK();
		send(sock, &value, sizeof(double), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "isAmbientLightCorrectionEnabled") == 0 && argumentCount == 0) {
		answer = apiIsEnabledAmbientLightCorrection();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setAmbientLightFactor") == 0 && argumentCount == 1) {
			answer = apiSetAmbientLightFactor(helperStringToInteger(stringArray[1]));
			send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "print") == 0 && argumentCount == 1) {
		answer = apiPrint(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "Kalman") == 0 && argumentCount == 1) {
		answer = apiEnableKalman(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanKdiff") == 0 && argumentCount == 1) {
		answer = apiSetKalmanKdiff(helperStringToDouble(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanK") == 0 && argumentCount == 1) {
		answer = apiSetKalmanK(helperStringToDouble(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanQ") == 0 && argumentCount == 1) {
		answer = apiSetKalmanQ(helperStringToDouble(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanThreshold") == 0 && argumentCount == 1) {
		answer = apiSetKalmanThreshold(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanNumCheck") == 0 && argumentCount == 1) {
		answer = apiSetKalmanNumCheck(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setKalmanThreshold2") == 0 && argumentCount == 1) {
		answer = apiSetKalmanThreshold2(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setTempCoef") == 0 && argumentCount == 1) {
		answer = apiSetTempCoef(helperStringToDouble(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "setSpeedOfLight") == 0 && argumentCount == 1) {
		answer = apiSetSpeedOfLight(helperStringToDouble(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "getSpeedOfLight") == 0 && argumentCount == 0) {
		double value = apiGetSpeedOfLightDev2();
		send(sock, &value, sizeof(double), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "tof") == 0 && argumentCount == 0) {
		answer = apiTOF();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "flim") == 0 && argumentCount == 0) {
		answer = apiFLIM();
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "extClkGen") == 0 && argumentCount == 1) {
		answer = apiSetExtClkGen(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "test") == 0 && argumentCount == 1) {
		answer = apiTest(helperStringToInteger(stringArray[1]));
		send(sock, &answer, sizeof(int16_t), MSG_NOSIGNAL);
	} else if (strcmp(stringArray[0], "version") == 0 && !argumentCount) {
		int16_t version = apiGetVersion();
		send(sock, &version, sizeof(int16_t), MSG_NOSIGNAL);
	}
	// unknown command
	else {
		printf("unknown command -> %s\n", stringArray[0]);
		int16_t response = -1;
		send(sock, &response, sizeof(int16_t), MSG_NOSIGNAL);
	}
	//bbbLEDEnable(3, 0);
}

/*!
 Handles signals
 @param sig signal ID
 */
void signalHandler(int sig) {
	printf("caught signal %i .... going to close application\n", sig);
	close(sockfd);
	close(newsockfd);
	//bbbLEDEnable(1, 1);
	//bbbLEDEnable(2, 1);
	//bbbLEDEnable(3, 1);
	exit(0);
}

void serverSend( void* buffer, size_t size){
	send(gSock, buffer, size, MSG_NOSIGNAL);
}

enum threadType pauseVideo(void){

	enum threadType typeLast;

	if(gStartGrayVideo)
		typeLast = StartGrayVideo;
	if(gStartAmpVideo)
		typeLast = StartAmpVideo;
	if(gStartDistVideo)
		typeLast = StartDistVideo;
	if(gStartAmpDistVideo)
		typeLast = StartAmpDistVideo;
	if(gStartDCSVideo)
		typeLast = StartDCSVideo;
	if(gStartDCSTOFandGrayVideo)
		typeLast = StartDCSTOFandGrayVideo;

	serverStopThreads(AllVideo); //reset+stop all threads egg. gStartGrayVideo
	//gRunVideo = 0; 				//no thread start possible without "startVideo"
	return typeLast;
}

void resumeVideo(enum threadType typeLast){

	switch(typeLast){
		case StartGrayVideo:
			//gRunVideo = 1;
			gStartGrayVideo=1;
			queueInit();
			pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetBWSorted);
			printf("resumedVideo StartGrayVideo\n");
			break;
		case StartAmpVideo:
			//gRunVideo = 1;
			gStartAmpVideo=1;
			queueInit();
			pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetAmplitudeSorted);
			printf("resumedVideo StartAmpVideo\n");
			break;
		case StartDistVideo:
			//gRunVideo = 1;
			gStartDistVideo=1;
			queueInit();
			pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetDistanceSorted);
			printf("resumedVideo StartDistVideo\n");
			break;
		case StartAmpDistVideo:
			//gRunVideo = 1;
			gStartAmpDistVideo=1;
			queueInit();
			pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetDistanceAndAmplitudeSorted);
			printf("resumedVideo StartAmpDistVideo\n");
			break;
		case StartDCSVideo:
			//gRunVideo = 1;
			gStartDCSVideo=1;
			queueInit();
			pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetDCSSorted);
			printf("resumedVideo StartGrayVideo\n");
			break;
		case StartDCSTOFandGrayVideo:
			//gRunVideo = 1;
			gStartDCSTOFandGrayVideo=1;
			queueInit();
			pthread_create(&imageThread, NULL, (void*)imagingThread, apiGetDCSTOFeAndGrayscaleSorted);
			printf("resumedVideo StartDCSTOFandGrayVideo\n");
			break;
		default:
			//no video mode
			break;
	}
}

void serverStopThreads(enum threadType type){
	switch(type){

	case StartGrayVideo:
		if(gStartAmpVideo==1 || gStartAmpDistVideo==1 || gStartDistVideo==1 || gStartDCSVideo==1 || gStartDCSTOFandGrayVideo==1){
			gStartAmpVideo     = 0;
			gStartAmpDistVideo = 0;
			gStartDCSVideo     = 0;
			gStartDCSTOFandGrayVideo=0;
			gStartDistVideo    = 0;
			gStartDCSTOFandGrayVideo = 0;
			while(stopThreadFlag != 1) usleep(50000);
		}
		break;

	case StartAmpVideo:
		if(gStartGrayVideo==1 || gStartAmpDistVideo==1 || gStartDistVideo==1 || gStartDCSVideo==1 || gStartDCSTOFandGrayVideo==1){
			gStartGrayVideo    = 0;
			gStartAmpDistVideo = 0;
			gStartDCSVideo     = 0;
			gStartDCSTOFandGrayVideo=0;
			gStartDistVideo    = 0;
			while(stopThreadFlag != 1) usleep(50000);
		}
		break;

	case StartDistVideo:
		if(gStartGrayVideo==1 || gStartAmpVideo==1 || gStartAmpDistVideo==1 || gStartDCSVideo==1 || gStartDCSTOFandGrayVideo==1){
			gStartGrayVideo    = 0;
			gStartAmpVideo     = 0;
			gStartAmpDistVideo = 0;
			gStartDCSVideo     = 0;
			gStartDCSTOFandGrayVideo=0;
			while(stopThreadFlag != 1) usleep(50000);
		}
		break;

	case StartAmpDistVideo:
		if(gStartGrayVideo==1 || gStartAmpVideo==1 || gStartDistVideo==1 || gStartDCSVideo==1 || gStartDCSTOFandGrayVideo==1){
			gStartGrayVideo    = 0;
			gStartAmpVideo     = 0;
			gStartDCSVideo     = 0;
			gStartDCSTOFandGrayVideo=0;
			gStartDistVideo    = 0;
			while(stopThreadFlag != 1) usleep(50000);
		}
		break;

	case StartDCSVideo:
		if(gStartGrayVideo==1 || gStartAmpVideo==1 || gStartAmpDistVideo==1 || gStartDistVideo==1 || gStartDCSTOFandGrayVideo==1){
			gStartGrayVideo    = 0;
			gStartAmpVideo     = 0;
			gStartAmpDistVideo = 0;
			gStartDistVideo    = 0;
			gStartDCSTOFandGrayVideo=0;
			while(stopThreadFlag != 1) usleep(50000);
		}
		break;

	case StartDCSTOFandGrayVideo:
		if(gStartGrayVideo==1 || gStartAmpVideo==1 || gStartAmpDistVideo==1 || gStartDistVideo==1 || gStartDCSVideo==1){
			gStartGrayVideo    = 0;
			gStartAmpVideo     = 0;
			gStartDCSVideo     = 0;
			gStartAmpDistVideo = 0;
			gStartDistVideo    = 0;
			while(stopThreadFlag != 1) usleep(50000);
		}
		break;

	default:
		if(gStartGrayVideo==1 || gStartAmpVideo==1 || gStartAmpDistVideo==1 || gStartDistVideo==1 || gStartDCSVideo==1 || gStartDCSTOFandGrayVideo==1){
			gStartGrayVideo    = 0;
			gStartAmpVideo     = 0;
			gStartAmpDistVideo = 0;
			gStartDCSVideo     = 0;
			gStartDCSTOFandGrayVideo=0;
			gStartDistVideo    = 0;
			while(stopThreadFlag != 1) usleep(50000);
		}
	}
}

/// @}
