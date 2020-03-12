#include "calc_def_pi_delay_sorted.h"
#include "pru.h"
#include "api.h"
#include "fp_atan2.h"
#include "calibration.h"
#include "config.h"
#include "hysteresis.h"
#include "adc_overflow.h"
#include "saturation.h"
#include "calculator.h"
#include "iterator_default.h"
#include "evalkit_constants.h"
#include "calculation.h"
#include "config.h"
#include "temperature.h"
#include <math.h>

#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


// YNG_CYLEE_190307A
#include <sys/mman.h>
#include <sys/types.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

/// \addtogroup calculate
/// @{

static uint16_t pixelData[MAX_NUM_PIX * 2];
static uint16_t pMem[4 * MAX_NUM_PIX];

// YNG_CYLEE_190307A
//==============================================================================
// << DDR MEM >>
static int fd_ddrmem;
#define _DDR_MAP_SIZE          0x96000   // 76800 x 4 frame x 2 byte = 614,400(decimal)
#define DDR_MEM_BASE_ADDR   0x30000000
//volatile static int* DDR_MEM_addr;
//static uint16_t* DDR_MEM_addr; // volatile ?
volatile static uint16_t* DDR_MEM_addr; // volatile ?
//==============================================================================
// << DMA >>
static int fd_mem_dma;
#define _DMA_MAP_SIZE       0x10000
#define DMA_REG_BASE_ADDR   0x41220000
volatile static int* DMA_REG_addr;
//==============================================================================
static int init_flag = 0;
//==============================================================================

// 190404B
//==============================================================================
static clock_t start_t;
static int COUNT = 0;
//==============================================================================
// 190412A
static struct timeval start_point, end_point;
//==============================================================================


/*!
Does sin modulation Pi delay ( DCS0, DCS1, DCS2, DCS3)  image acquisition and returns sorted distance/amplitude data array
@param type calculation type: 0 - distance, 1-amplitude, 2 - distance and amplitude
@param data pointer to data pointer array. This arrays contains image data
@returns image size (number of pixels)
 */
int calcDefPiDelayGetDataSorted(enum calculationType type, uint16_t **data){
	int numPix = 4 * pruGetNCols() * pruGetNRowsPerHalf() * pruGetNumberOfHalves();
	int dcsAdd0=0;
	int dcsAdd1=0;
	int nPixelPerDCS = 0;
	int size=0;

	uint16_t pixelDCS0=0;
	uint16_t pixelDCS1=0;
	uint16_t pixelDCS2=0;
	uint16_t pixelDCS3=0;

	uint16_t ampGrayscale=0;

	struct TofResult tofResult;

	uint16_t* pMem1;

	// YNG_190325A
	uint16_t* data2;

	/*double elapsedTime;
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);*/


	// YNG_CYLEE_190307A
        if (init_flag==0) {

        	printf("DDR, DMA INIT !!!\n");

	//==========================================================================
	        /* Open the file for the memory device: */
	        fd_mem_dma = open("/dev/mem", O_RDWR);
	        fd_ddrmem = open("/dev/mem", O_RDWR);

	        if (fd_mem_dma < 0) {
	                perror("\x1b[31m" "Failed to open /dev/mem (%s)\n" "\x1b[0m");
	                return -1;
	        }
	        if (fd_ddrmem < 0) {
	                perror("\x1b[31m" "Failed to open /dev/mem (%s)\n" "\x1b[0m");
	                return -1;
	        }
	//==========================================================================================================
	// << DDR MEM >>
	       DDR_MEM_addr = mmap(NULL, _DDR_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_ddrmem, DDR_MEM_BASE_ADDR);

	       if(DDR_MEM_addr == MAP_FAILED){
	         perror("### !!! Memory mapping failed !!! ###");
	         exit(EXIT_FAILURE);
	       }
	//==========================================================================================================
	//==========================================================================================================
	// << DMA >>
	       DMA_REG_addr = mmap(NULL, _DMA_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem_dma, DMA_REG_BASE_ADDR);

	       if(DMA_REG_addr == MAP_FAILED){
	         perror("### !!! Memory mapping failed !!! ###");
	         exit(EXIT_FAILURE);
	       }
	//==========================================================================================================
        }

        init_flag = 1;

	//==========================================================================================================

	if (configStatusEnableAddArg()!=0){
		size = pruGetImage(data);
		pMem1 = *data;

		nPixelPerDCS = size / 4;

                //### YNG_190328A ###
                // Just Monitoring TEST
                printf("\n ################################################################################### \n"); 
                printf("\n ########### configStatusEnableAddArg ##############################################\n"); 
                printf("\n ################################################################################### \n"); 

			for(int i=0; i<(nPixelPerDCS); i++){
				dcsAdd0=pMem1[i]+pMem1[i+2*nPixelPerDCS];	//check dcs0+2
				dcsAdd1=pMem1[i+nPixelPerDCS]+pMem1[i+3*nPixelPerDCS];	//check dcs0+2
				if (configGetPartType() == EPC635 || configGetPartType() == EPC503) {
					dcsAdd0=dcsAdd0 >> 4;
					dcsAdd1=dcsAdd1 >> 4;
				}

				if((fabs(dcsAdd0-4096) < (float)configGetDcsAdd_threshold())&&(fabs(dcsAdd1-4096) < (float)configGetDcsAdd_threshold())){
/*
					pMem[i] = pMem1[i];
					pMem[i+nPixelPerDCS] = pMem1[i+nPixelPerDCS];
					pMem[i+2*nPixelPerDCS] = pMem1[i+2*nPixelPerDCS];
					pMem[i+3*nPixelPerDCS] = pMem1[i+3*nPixelPerDCS];
*/
					*(DDR_MEM_addr+i) = pMem1[i];
					*(DDR_MEM_addr+i+nPixelPerDCS) = pMem1[i+nPixelPerDCS];
					*(DDR_MEM_addr+i+2*nPixelPerDCS) = pMem1[i+2*nPixelPerDCS];
					*(DDR_MEM_addr+i+3*nPixelPerDCS) = pMem1[i+3*nPixelPerDCS];

				}else{
/*
					pMem[i] = 0x0000;
					pMem[i+nPixelPerDCS] = 0x0000;
					pMem[i+2*nPixelPerDCS] = 0x0000;
					pMem[i+3*nPixelPerDCS] = 0x0000;
*/
					*(DDR_MEM_addr+i) = 0x0000;
					*(DDR_MEM_addr+i+nPixelPerDCS) = 0x0000;
					*(DDR_MEM_addr+i+2*nPixelPerDCS) = 0x0000;
					*(DDR_MEM_addr+i+3*nPixelPerDCS) = 0x0000;

				}

				if(i == 4500 && configPrint()>1){
					printf("print 4500 dcs0+2: %2.4f dcs0+2: %2.4f threshold %d \n", fabs(dcsAdd0-4096),fabs(dcsAdd1-4096),configGetDcsAdd_threshold());

				}
			}
		}else{
			size = pruGetImage(data);
			//pMem = *data;
			nPixelPerDCS = size / 4;

			/*
			for(int i=0; i<numPix; i++){
				pMem[i] = pMem1[i];
			}
                        */

			//### YNG_190412A ###
                      //usleep( 50000); // FINAL

			// YNG_CYLEE_190307A
			//==========================================================================
/*
			int addr;
			uint16_t* pMem2;
			uint16_t* data2;

			data2 = *data;

			// CYLEE DMA TEST
			pMem2 = pMem;
*/

// TEMPORAL Skip ! YNG_190325A 
/*
			*DMA_REG_addr = 0x00010006; // Reset !
			*(DMA_REG_addr+6) = 0x60000000; // Video Capture Addr
			*(DMA_REG_addr+8) = 0x30000000; // DDR MEM Addr
*/
		      //*(DMA_REG_addr+10) = 153600*4; // Transfer Bytes
/*
			*(DMA_REG_addr+10) = numPix*2; // Transfer Bytes
*/

//=============================================
/*
            for (addr = 0; addr < numPix; addr++) {
            *pMem2 = *data2;
             pMem2++;
             data2++;
             }
*/
//=============================================

//==========================================================================
// CYLEE
//			memcpy( (void*)pMem, (void*)(*data), numPix * sizeof(uint16_t) );	//sho 25.10.17
//==========================================================================

/*
  int addr;
  uint16_t* pMem2;
  uint16_t* data2;
  pMem2 = pMem;
  data2 = *data;
  for (addr = 0; addr < numPix; addr++) {
  *pMem2 = *data2;
  pMem2++;
  data2++;
  }
*/
//==========================================================================

	// YNG_CYLEE_190325A
	data2 = *data;

   /*gettimeofday(&tv2, NULL);
	elapsedTime = (double)(tv2.tv_sec - tv1.tv_sec) + (double)(tv2.tv_usec - tv1.tv_usec)/1000000.0;
	printf("calculationDistanceAndAmplitudeSorted 1 in seconds: = %fs\n", elapsedTime);*/
		}

	if( (calculationGetEnableAmbientLight() !=0) || calculationGetEnableTemperature() != 0 ){
		int tempDiff = temperatureGetTemperature(configGetDeviceAddress()) - calibrateGetTempCalib();
		calculationSetDiffTemp(tempDiff);	// set temperature difference
	}

	calculatorInit(2);
	iteratorDefaultInit(nPixelPerDCS, pruGetNCols(), pruGetNRowsPerHalf() * pruGetNumberOfHalves());

// YNG_CYLEE_190418A
//	while(iteratorDefaultHasNext()){
//	struct Position p = iteratorDefaultNext();

/*
	pixelDCS0 = pMem[p.indexMemory];
	pixelDCS1 = pMem[p.indexMemory + nPixelPerDCS];
	pixelDCS2 = pMem[p.indexMemory + 2 * nPixelPerDCS];
	pixelDCS3 = pMem[p.indexMemory + 3 * nPixelPerDCS];
*/

// TEMPORAL Skip ! YNG_190325A 
/*
	pixelDCS0 = *(DDR_MEM_addr+p.indexMemory);
	pixelDCS1 = *(DDR_MEM_addr+p.indexMemory + nPixelPerDCS);
	pixelDCS2 = *(DDR_MEM_addr+p.indexMemory + 2 * nPixelPerDCS);
	pixelDCS3 = *(DDR_MEM_addr+p.indexMemory + 3 * nPixelPerDCS);
*/

	//check symmetry, if not -> pruGetImage()

//### YNG_190325A ###
// TEMPORAL Skip ! YNG_190325A 
/*
		if(calculationGetEnableAmbientLight() != 0){
			ampGrayscale = temperatureGetGrayscalePixel(p.indexMemory);
			pixelDCS0 = calculationAmbientDCSCorrectionPixel(pixelDCS0, ampGrayscale);
			pixelDCS1 = calculationAmbientDCSCorrectionPixel(pixelDCS1, ampGrayscale);
		}

		calculatorSetArgs4DCS(pixelDCS0, pixelDCS1, pixelDCS2, pixelDCS3, p.indexMemory, p.indexMemory);
*/

//### YNG_190327H ###
/*
		switch(type){
		case DIST:
                //### YNG_190325A ###
		//	pixelData[p.indexSorted] = calculatorGetDistAndAmpSine().dist;
			pixelData[p.indexSorted] = *(data2+(p.indexMemory*2));
			break;
		case AMP:
			pixelData[p.indexSorted] = calculatorGetAmpSine();
			break;
		case INFO:
			tofResult = calculatorGetDistAndAmpSine();
			pixelData[p.indexSorted] = tofResult.dist;
			pixelData[p.indexSorted + nPixelPerDCS] = tofResult.amp;
			break;
		}
*/
                //### YNG_190412A ###
/*
                //### YNG_190411A ###
		pixelData[p.indexMemory] = *(data2 + p.indexMemory);
		pixelData[p.indexMemory + (320*240)] = *(data2 + (320*240) + p.indexMemory);
*/

//	}

	//### YNG_190412A ###
	  //usleep( 38500); // FINAL
	  //usleep( 30000); // ERROR
	  //usleep( 35000); // O.K (26 fps)
	  //usleep( 34000); // O.K (27 fps)
	  //usleep( 32000); // ERROR (@Integ 1700us)
	  //usleep( 32500); // (Integ 1700us --> ERROR, Integ 500us --> O.K (27.8 fps))
	  //usleep( 32000); // (Integ  500us --> O.K (28.2 fps))
	  //usleep( 31500); // (Integ 1300us --> O.K (28.6 fps))
	  //usleep( 31000); // (Integ 1200us --> O.K (29.3 fps))
	  //usleep( 30500); // (Integ 1000us --> O.K (29.6 fps))
	  //usleep( 30000); // (Integ  950us --> O.K (### 30 fps ###))
	    usleep( 33000); // ### O.K ### (# 27.5 fps # @Integ_1700us)

        //### DMA TRANSFER YNG_190412A ###
        *DMA_REG_addr = 0x00010006;       // Reset !
        *(DMA_REG_addr+6) = 0x60000000;   // Video Capture Addr
        *(DMA_REG_addr+8) = 0x30000000;   // DDR MEM Addr
        *(DMA_REG_addr+10) = 320*240*2*2; // Transfer Bytes (320*240 pixel(1 frame) X 2 bytes (16bit data) X 2 (Dist & Amp))

//### YNG_190412A ###
//	*data = pixelData;
	*data = DDR_MEM_addr;

// 190404B
//-------------------------------------------------------------------------
/*
	clock_t end_t;
	float fps = 0.0000;


	if (COUNT < 100) {
	  COUNT += 1;
        }
        else {     // COUNT == 100
	  end_t = clock();
	  fps = ( 100.0 / (float)(end_t - start_t) ) * CLOCKS_PER_SEC;
	  COUNT = 0;
	  start_t = clock();
	  printf("#---------------------------\n");
	  printf("# FPS = (%f) \n", fps);
	  printf("#---------------------------\n");
	}
*/
        double operating_time;
        double FrameRate;

        // Counting 1 ~ 100
	if (COUNT < 100) {
	  COUNT += 1;
        }
        else {     // COUNT == 100
          gettimeofday(&end_point, NULL); 
          operating_time = (double)(end_point.tv_sec)+(double)(end_point.tv_usec)/1000000.0-(double)(start_point.tv_sec)-(double)(start_point.tv_usec)/1000000.0;
          FrameRate = 100.0 / operating_time;

//	  printf("############################\n");
//	  printf("#  100 FRAME COUNTER       #\n");
//        printf("#  (%f          )    #\n",operating_time);
          printf("##### << %f >> #####\n",FrameRate);
//	  printf("############################\n");

	  COUNT = 1;
          gettimeofday(&start_point, NULL);
	}
//-------------------------------------------------------------------------

	if (type == INFO){
		return nPixelPerDCS * 2;
	}

	//gettimeofday(&tv2, NULL);
	//elapsedTime = (double)(tv2.tv_sec - tv1.tv_sec) + (double)(tv2.tv_usec - tv1.tv_usec)/1000000.0;
	//printf("calculationDistanceAndAmplitudeSorted 1 in seconds: = %fs\n", elapsedTime);

	return nPixelPerDCS;
}
