/*
 * frt.h
 *
 *  Created on: Oct 26, 2021
 *      Author: mihailo
 */

#ifndef SRC_FRTRACE_FRT_H_
#define SRC_FRTRACE_FRT_H_

/* Read trace log task stack size */
#define READ_TRACE_LOG_THREAD_STACK_SIZE (2048)
/* Period for reading the trace message log in ms */
#define READ_TRACE_LOG_PERIOD  (1000)
/* Maximum number of channels, for transmitting log messages, which can be registered */
#define MAX_NUMBER_OF_CHANNELS (10)
/* Read trace log task priority */
#define READ_TRACE_LOG_PRIO    (16)

typedef enum {
	eFrtStateOk,
	eFrtStateError
} eFrtStateType;

/**
 * frt. h
 * <pre>void vFrtInit();</pre>
 *
 * Initalize the FreeRTOS trace library.
 *
 * Creates a task which reads the trace log messages and sends them to all the registered channels.
 * Initalize the trace log buffer used for storing the messages formed in all the defined trace functions
 *
 * @param None
 *
 * @return None
 */
eFrtStateType xFrtInit(void);
/**
 * frt. h
 * <pre>void vFrtRegisterChannel(void (*f)(char *pcMessage, uint8_t ucMessageLength));</pre>
 *
 * Register a channel for transmitting read messages from the trace log buffer.
 *
 * @param (*f)(char *pcMessage, uint8_t ucMessageLength) pointer to a function which represents a wrapper for a specific
 * interface driver.
 *
 * @return None
 */
void vFrtRegisterChannel(void (*f)(void *pcMessage, size_t ucMessageLength));

#endif /* SRC_FRTRACE_FRT_H_ */
