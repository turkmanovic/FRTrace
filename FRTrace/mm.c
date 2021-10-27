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

/* Instance of the pointer to the ring buffer */
static RingBuffer_t *pxTraceLog;

void vMMRingBufferInit() {
	pxTraceLog->ucTailIndex = 0;
	pxTraceLog->ucHeadIndex = 0;
	for (int i = 0 ; i < LOG_SIZE ; i++){
		pxTraceLog->xBuffer[i].eStatus = eFREE;
		strcpy(pxTraceLog->xBuffer[i].pcMessage,"EMPTY");
	}
	pxTraceLog->ucDeletedMessages = 0;
	pxTraceLog->ucOccupancy = 0;
};

void vMMWriteTraceLog(char *pcMessage){
	/* Check whether the pointed buffer is not being read currently */
	if (pxTraceLog->xBuffer[pxTraceLog->ucHeadIndex].eStatus == eFREE){
		/* Take semaphore of the current buffer element */
		pxTraceLog->xBuffer[pxTraceLog->ucHeadIndex].eStatus = eWRITING;
		if(ucMMRingBufferIsFull()) {
			/* Is going to overwrite the oldest message */
			/* Increase tail index */
			pxTraceLog->ucTailIndex++;
			if (pxTraceLog->ucTailIndex == LOG_SIZE){
				pxTraceLog->ucTailIndex = 0;
			}
			pxTraceLog->ucDeletedMessages++;
			pxTraceLog->ucOccupancy--;
		}

		/* Place message in buffer */
		strcpy(pxTraceLog->xBuffer[pxTraceLog->ucHeadIndex].pcMessage, pcMessage);

		/* Increment head index for the next input to be written */
		pxTraceLog->ucHeadIndex++;
		/* If it reaches the maximum index, drop it to 0 */
		if (pxTraceLog->ucHeadIndex == LOG_SIZE){
			pxTraceLog->ucHeadIndex = 0;
		}
		/* Increase the occupancy */
		pxTraceLog->ucOccupancy++;
		/* Release semaphore*/
		pxTraceLog->xBuffer[pxTraceLog->ucHeadIndex].eStatus = eFREE;
	} else {
		/* Current message dropped due to the buffer element being pointed to by ucHeadIndex not being available. */
		/* Increase the number of deleted messages for the user to be aware of this behavior. */
		pxTraceLog->ucDeletedMessages++;
	}
};

void vMMReadTraceLog(char* pcMessage){
	if (!(ucMMRingBufferIsEmpty())){
		/* Take semaphore of the current buffer element */
		pxTraceLog->xBuffer[pxTraceLog->ucTailIndex].eStatus = eREADING;
		strcpy(pcMessage, pxTraceLog->xBuffer[pxTraceLog->ucTailIndex].pcMessage);

		/* Increase the tail index */
		pxTraceLog->ucTailIndex++;
		/* If it reaches the maximum index, drop it to 0 */
		if (pxTraceLog->ucTailIndex == LOG_SIZE){
			pxTraceLog->ucTailIndex = 0;
		}
		/* Decrease the occupancy */
		pxTraceLog->ucOccupancy--;
		/* Release semaphore*/
		pxTraceLog->xBuffer[pxTraceLog->ucTailIndex].eStatus = eFREE;
	}
};

inline uint8_t ucMMRingBufferIsEmpty() {
	return (pxTraceLog->ucOccupancy == 0);
}

inline uint8_t ucMMRingBufferIsFull() {
	return (pxTraceLog->ucOccupancy == LOG_SIZE);
}

inline uint8_t ucMMRingBufferGetOccupancy(){
	return pxTraceLog->ucOccupancy;
}

uint8_t ucMMReadOverwrittenMessages() {
	uint8_t ucTemp;
	ucTemp = pxTraceLog->ucDeletedMessages;
	pxTraceLog->ucDeletedMessages = 0;
	return ucTemp;
}


