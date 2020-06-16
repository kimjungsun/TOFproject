#ifdef _SERVER_ONLY
#include "api.h"
#include "evalkit_constants.h"
#include "i2c.h"
#include "config.h"
#include "evalkit_illb.h"
#include "boot.h"
#include "pru.h"
#include "bbb_led.h"
#include "dll.h"
#include "read_out.h"
#include "log.h"
#include "version.h"
#include "helper.h"
#include "calibration.h"
#include "calculation.h"
#include "queue_imaging.h"
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

void imagingThread(int (* funcThread)(uint16_t**));
void handleRequest(int sock,int atecsock);
void signalHandler(int sig);
void serverSend( void* buffer, size_t size);
enum threadType pauseVideo(void);
void resumeVideo(enum threadType typeLast);
void serverStopThreads(enum threadType type);

//UDP #define BUFSIZE 10
#endif


#ifdef _SERVER_H
int startServer(const unsigned int addressOfDevice);
#endif



#ifdef _SUBSERVER_AND_SERVER_SHARE
extern int dualsock;
extern int gStartAmpDistVideo;
extern char ellum_buf_own;
extern char ellum_buf_other;
extern int dual_mode;

#endif
