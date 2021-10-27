/*
 * mm.h
 *
 *  Created on: Oct 26, 2021
 *      Author: mihailo
 */

#ifndef SRC_FRTRACE_MM_H_
#define SRC_FRTRACE_MM_H_

#include <stdint.h>

/* Size of the ring buffer in terms of how many message can be stored in it without overwriting previous messages.*/
#define LOG_SIZE 128
/* Size of messages used by the tracing mechanism (ring buffer messages, place holders while forming messages)*/
#define MESSAGE_SIZE 70

/**
 * Enum type indicating the status of each trace messages inside the ring buffer.
 * Replaces the FreeRTOS semaphore mechanism in order to perform resource protection.
 */
typedef enum {
		eFREE = 0,       /**< Enum value indicating buffer element can be accessed either for reading or writing. */
		eWRITING = 1,    /**< Enum value indicating that the buffer element is being written to. */
		eREADING = 2     /**< Enum value indicating that the buffer element is being read from. */
} eAccessRights;

/**
 * Structure which holds the trace message alongside the status of the message inside the buffer.
 */
typedef struct xLogMessage {
	eAccessRights eStatus;   /**< Field holding the status of the buffer element. */
    char pcMessage[MESSAGE_SIZE]; /**< Field holding the actual message. */
} xLogMessage;

/**
 * Structure which implements the ring buffer.
 * The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */
typedef struct RingBuffer_t {
  /* Buffer memory. */
  xLogMessage xBuffer[LOG_SIZE];
  /* Index of tail. */
  uint8_t ucTailIndex;
  /* Index of head. */
  uint8_t ucHeadIndex;
  /* Number of deleted message due to overwriting **/
  uint8_t ucDeletedMessages;
  /* Number of unread messages in the buffer */
  uint8_t ucOccupancy;
} RingBuffer_t;

/**
 * mm. h
 * <PRE>void vMMRingBufferInit();</PRE>
 *
 * Initializes all the buffer elements and its fields to default values.
 *
 * @return None
 */
void vMMRingBufferInit();

/**
 * mm. h
 * <PRE>void vMMWriteTraceLog(char *pxMessage);</PRE>
 *
 * @param char* Pointer to a message which is intended to be written to the ring buffer.
 *
 * @return None
 *
 */
void vMMWriteTraceLog(char *pcMessage);
/**
 * mm. h
 * <PRE>void vMMReadTraceLog(char *pcMessage)</PRE>
 *
 * @return Returns the oldest message from the ring buffer by storing the pointer to the message into pcMessage.
 *
 * \defgroup pcTaskGetName pcTaskGetName
 * \ingroup TaskUtils
 */
void vMMReadTraceLog(char* pcMessage);
/**
 * mm. h
 * <PRE>uint8_t ucMMRingBufferIsEmpty();</PRE>
 *
 * @return Returns 1 in case the ring buffer is empty. Returns 0 otherwise.
 *
 */
uint8_t ucMMRingBufferIsEmpty();

/**
 * mm. h
 * <PRE>uint8_t ucMMRingBufferIsFull();</PRE>
 *
 * @return 1 in case the ring buffer is full. Returns 0 otherwise.
 *
 */
uint8_t ucMMRingBufferIsFull();

/**
 * mm. h
 * <PRE>uint8_t ucMMRingBufferGetOccupancy();</PRE>
 *
 * @return number of unread messages inside the ring buffer.
 *
 */
uint8_t ucMMRingBufferGetOccupancy();

/**
 * mm. h
 * <PRE>uint8_t ucMMReadOverwrittenMessages();</PRE>
 *
 * @return number of messages overwritten from the ring buffer due to overflow.
 *
 */
uint8_t ucMMReadOverwrittenMessages();

#endif /* SRC_FRTRACE_MM_H_ */
