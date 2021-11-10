/*
 * mm.c
 *
 *  Created on: Oct 26, 2021
 *      Author: mihailo
 */

/*Toolchain include*/
#include <string.h>
/*FRTrace include*/
#include "mm.h"

/* Instance of the ring buffer */
static RingBuffer_t xTraceLog = {
	.ucTailIndex = 0,
	.ucHeadIndex = 0,
	.ucDeletedMessages = 0,
	.ucOccupancy = 0
};

void vMMRingBufferInit() {
	for (int i = 0 ; i < LOG_SIZE ; i++){
		xTraceLog.xBuffer[i].eStatus = eFREE;
		//strcpy(xTraceLog.xBuffer[i].pcMessage,"EMPTY\r\n");
	}
};

void vMMWriteTraceLog(char *pcMessage){
	/* Check whether the pointed buffer is not being read currently */
	if (xTraceLog.xBuffer[xTraceLog.ucHeadIndex].eStatus == eFREE){
		/* Take semaphore of the current buffer element */
		xTraceLog.xBuffer[xTraceLog.ucHeadIndex].eStatus = eWRITING;
		if(ucMMRingBufferIsFull()) {
			/* Is going to overwrite the oldest message */
			/* Increase tail index */
			xTraceLog.ucTailIndex++;
			if (xTraceLog.ucTailIndex == LOG_SIZE){
				xTraceLog.ucTailIndex = 0;
			}
			xTraceLog.ucDeletedMessages++;
		}

		/* Place message in buffer */
		strcpy(xTraceLog.xBuffer[xTraceLog.ucHeadIndex].pcMessage, pcMessage);

		/* Release semaphore*/
		xTraceLog.xBuffer[xTraceLog.ucHeadIndex].eStatus = eFREE;

		/* Increment head index for the next input to be written */
		xTraceLog.ucHeadIndex++;
		/* If it reaches the maximum index, drop it to 0 */
		if (xTraceLog.ucHeadIndex == LOG_SIZE){
			xTraceLog.ucHeadIndex = 0;
		}
		/* Increase the occupancy */
		if (!(ucMMRingBufferIsFull())){
			xTraceLog.ucOccupancy++;
		}
	} else {
		/* Current message dropped due to the buffer element being pointed to by ucHeadIndex not being available. */
		/* Increase the number of deleted messages for the user to be aware of this behavior. */
		xTraceLog.ucDeletedMessages++;
	}
};

void vMMReadTraceLog(char* pcMessage){
	if (xTraceLog.xBuffer[xTraceLog.ucTailIndex].eStatus == eFREE){
		/* Take semaphore of the current buffer element */
		xTraceLog.xBuffer[xTraceLog.ucTailIndex].eStatus = eREADING;
		strcpy(pcMessage, xTraceLog.xBuffer[xTraceLog.ucTailIndex].pcMessage);

		/* Release semaphore*/
		xTraceLog.xBuffer[xTraceLog.ucTailIndex].eStatus = eFREE;

		/* Increase the tail index */
		xTraceLog.ucTailIndex++;
		/* If it reaches the maximum index, drop it to 0 */
		if (xTraceLog.ucTailIndex == LOG_SIZE){
			xTraceLog.ucTailIndex = 0;
		}
		/* Decrease the occupancy */
		xTraceLog.ucOccupancy--;
	}
};

inline uint8_t ucMMRingBufferIsEmpty() {
	return (xTraceLog.ucOccupancy == 0);
}

inline uint8_t ucMMRingBufferIsFull() {
	return (xTraceLog.ucOccupancy == LOG_SIZE);
}

inline uint32_t ulMMRingBufferGetOccupancy(){
	return xTraceLog.ucOccupancy;
}

uint32_t ulMMReadOverwrittenMessages() {
	uint32_t ulTemp;
	ulTemp = xTraceLog.ucDeletedMessages;
	xTraceLog.ucDeletedMessages = 0;
	return ulTemp;
}


