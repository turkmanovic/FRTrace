/*
 * frt.c
 *
 *  Created on: Oct 26, 2021
 *      Author: mihailo
 */

/*Toolchain include*/
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
#include "string.h"

/*FRTrace include*/
#include "frt.h"
#include "mm.h"

/* Driver include */
#include <drv_usart.h>

typedef enum {
	eFrtDataStateUndef,
	eFrtDataStateInit,
	eFrtDataStateServiceTask,
	eFrtDataStateError
} eFrtDataStateType;

typedef struct {

	eFrtDataStateType eState;

	/* Trace log buffer occupancy */
	uint32_t ulOccupancy;

} eFrtDataType;

static eFrtDataType xFrtData = {
		.eState = eFrtDataStateUndef
};

static uint8_t ucQueueType;

/* Strings representing object of interest in forming trace messages */
static char pcObjectStringArr[5][20] = {"Queue",
		"Mutex",
		"Counting semaphore",
		"Binary semaphore",
		"Recursive semaphore"};

/* Task states of interest in forming trace messages */
static char pcTaskStatusArr[8][25] = {" created",
		" resumed",
		" suspended",
		" switched in",
		" switched out",
		" moved to READY state",
		" blocked",
		" deleted"};

/* Queue states of interest in forming trace messages*/
static char pcQueueStatusArr[4][20] = {"created","given", "taken", "given from ISR"};

/* Placeholder used for forming trace messages which are to be sent to the trace log buffer */
static char pcMessageHolder[MESSAGE_SIZE];

/* Task control block definition included in order to be able to obtain the name of the current task. */
typedef struct tskTaskControlBlock 			/* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
	volatile StackType_t	*pxTopOfStack;	/*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

#if ( portUSING_MPU_WRAPPERS == 1 )
	xMPU_SETTINGS	xMPUSettings;		/*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT. */
#endif

	ListItem_t			xStateListItem;	/*< The list that the state list item of a task is reference from denotes the state of that task (Ready, Blocked, Suspended ). */
	ListItem_t			xEventListItem;		/*< Used to reference a task from an event list. */
	UBaseType_t			uxPriority;			/*< The priority of the task.  0 is the lowest priority. */
	StackType_t			*pxStack;			/*< Points to the start of the stack. */
	char				pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

#if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
	StackType_t		*pxEndOfStack;		/*< Points to the highest valid address for the stack. */
#endif

#if ( portCRITICAL_NESTING_IN_TCB == 1 )
	UBaseType_t		uxCriticalNesting;	/*< Holds the critical section nesting depth for ports that do not maintain their own count in the port layer. */
#endif

#if ( configUSE_TRACE_FACILITY == 1 )
	UBaseType_t		uxTCBNumber;		/*< Stores a number that increments each time a TCB is created.  It allows debuggers to determine when a task has been deleted and then recreated. */
	UBaseType_t		uxTaskNumber;		/*< Stores a number specifically for use by third party trace code. */
#endif

#if ( configUSE_MUTEXES == 1 )
	UBaseType_t		uxBasePriority;		/*< The priority last assigned to the task - used by the priority inheritance mechanism. */
	UBaseType_t		uxMutexesHeld;
#endif

#if ( configUSE_APPLICATION_TASK_TAG == 1 )
	TaskHookFunction_t pxTaskTag;
#endif

#if( configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 )
	void			*pvThreadLocalStoragePointers[ configNUM_THREAD_LOCAL_STORAGE_POINTERS ];
#endif

#if( configGENERATE_RUN_TIME_STATS == 1 )
	uint32_t		ulRunTimeCounter;	/*< Stores the amount of time the task has spent in the Running state. */
#endif

#if ( configUSE_NEWLIB_REENTRANT == 1 )
	/* Allocate a Newlib reent structure that is specific to this task.
		Note Newlib support has been included by popular demand, but is not
		used by the FreeRTOS maintainers themselves.  FreeRTOS is not
		responsible for resulting newlib operation.  User must be familiar with
		newlib and must provide system-wide implementations of the necessary
		stubs. Be warned that (at the time of writing) the current newlib design
		implements a system-wide malloc() that must be provided with locks.

		See the third party link http://www.nadler.com/embedded/newlibAndFreeRTOS.html
		for additional information. */
	struct	_reent xNewLib_reent;
#endif

#if( configUSE_TASK_NOTIFICATIONS == 1 )
	volatile uint32_t ulNotifiedValue;
	volatile uint8_t ucNotifyState;
#endif

	/* See the comments in FreeRTOS.h with the definition of
	tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
#if( tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0 ) /*lint !e731 !e9029 Macro has been consolidated for readability reasons. */
	uint8_t	ucStaticallyAllocated; 		/*< Set to pdTRUE if the task is a statically allocated to ensure no attempt is made to free the memory. */
#endif

#if( INCLUDE_xTaskAbortDelay == 1 )
	uint8_t ucDelayAborted;
#endif

#if( configUSE_POSIX_ERRNO == 1 )
	int iTaskErrno;
#endif

} tskTCB;

/* Pointer to the current task control block. Used for obtaining the name of the current task. */
extern struct tskTaskControlBlock* volatile pxCurrentTCB;

/* Queue for storing drive functions of all the registered interfaces. */
typedef void (*channel_functions_t)(void *pcMessage, size_t ucMessageLength);
static channel_functions_t pvChannelFunctions[MAX_NUMBER_OF_CHANNELS];
//static void (*pvChannelFunctions[MAX_NUMBER_OF_CHANNELS]) (void *pcMessage, size_t ucMessageLength);
/* Variable holding the number of registered channels. */
static uint8_t ucChannelNum = 0;
/* Place holder for messages */
static char pcMessage[MESSAGE_SIZE];

/* Number of deleted messages in the trace log buffer due to overflow */
static uint32_t ulOverwrittenMessages;

void vprvFrtTaskCreate(TaskHandle_t xTask){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", pcTaskGetName(xTask), pcTaskStatusArr[0]);
	vMMWriteTraceLog(pcMessageHolder);
};

void vprvFrtTaskResume(TaskHandle_t xTask){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", pcTaskGetName(xTask), pcTaskStatusArr[1]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtTaskSuspend(TaskHandle_t xTask){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", pcTaskGetName(xTask), pcTaskStatusArr[2]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtTaskSwitchedIn(){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", ((struct tskTaskControlBlock*) pxCurrentTCB)->pcTaskName, pcTaskStatusArr[3]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtTaskSwitchedOut(){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", ((struct tskTaskControlBlock*) pxCurrentTCB)->pcTaskName, pcTaskStatusArr[4]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtTaskMovedToReadyState(TaskHandle_t xTask){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", pcTaskGetName(xTask), pcTaskStatusArr[5]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtTaskDelete(TaskHandle_t xTask){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", pcTaskGetName(xTask), pcTaskStatusArr[7]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtQueueCreate(QueueHandle_t xQueue){
	ucQueueType = ucQueueGetQueueType(xQueue);
	sprintf(pcMessageHolder, "%u %s 0x%x %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], (unsigned int)xQueue, pcQueueStatusArr[0]);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtQueueReceive(QueueHandle_t xQueue){
	const char *ucQueueName;
	ucQueueType = ucQueueGetQueueType(xQueue);
	if (!(ucQueueName = pcQueueGetName(xQueue))){
		sprintf(pcMessageHolder, "%u %s 0x%x %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], (unsigned int)xQueue, pcQueueStatusArr[2]);
	} else {
		sprintf(pcMessageHolder, "%u %s %s %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], ucQueueName, pcQueueStatusArr[2]);
	}
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtQueueSend(QueueHandle_t xQueue){
	const char *ucQueueName;
	ucQueueType = ucQueueGetQueueType(xQueue);
	if (!(ucQueueName = pcQueueGetName(xQueue))){
		sprintf(pcMessageHolder, "%u %s 0x%x %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], (unsigned int)xQueue, pcQueueStatusArr[1]);
	} else {
		sprintf(pcMessageHolder, "%u %s %s %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], ucQueueName, pcQueueStatusArr[1]);
	}
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtQueueGiveFromISR(QueueHandle_t xQueue){
	const char *ucQueueName;
	ucQueueType = ucQueueGetQueueType(xQueue);
	if (!(ucQueueName = pcQueueGetName(xQueue))){
		sprintf(pcMessageHolder, "%u %s 0x%x %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], (unsigned int)xQueue, pcQueueStatusArr[3]);
	} else {
		sprintf(pcMessageHolder, "%u %s %s %s\n\r", (unsigned int)xTaskGetTickCount(), pcObjectStringArr[ucQueueType], ucQueueName, pcQueueStatusArr[3]);
	}
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtBlockingOnQueueReceive(QueueHandle_t xQueue){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", ((struct tskTaskControlBlock*) pxCurrentTCB)->pcTaskName, " blocked on queue receive");
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtBlockingOnQueueSend(QueueHandle_t xQueue){
	sprintf(pcMessageHolder, "%u %s%s%s\n\r", (unsigned int)xTaskGetTickCount(), "Task ", ((struct tskTaskControlBlock*) pxCurrentTCB)->pcTaskName, "blocked on queue send");
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtMalloc(void* pvAddress, uint8_t uiSize){
	unsigned int pvCastAddress = (unsigned int)pvAddress;
	sprintf(pcMessageHolder, "%u %s%u%s%X\n\r", (unsigned int)xTaskGetTickCount(), "Allocated ", uiSize," bytes from address 0x", pvCastAddress);
	vMMWriteTraceLog(pcMessageHolder);
}

void vprvFrtFree(void* pvAddress, uint8_t uiSize){
	unsigned int pvCastAddress = (unsigned int)pvAddress;
	sprintf(pcMessageHolder, "%u %s%u%s%X\n\r", (unsigned int)xTaskGetTickCount(), "Freed ", uiSize," bytes from address 0x", pvCastAddress);
	vMMWriteTraceLog(pcMessageHolder);
}

void vFrtRegisterChannel(void (*f)(void *pcMessage, size_t ucMessageLength)){
	pvChannelFunctions[ucChannelNum++] = f;
}

void vFrtCallback(void *pcMessage, size_t ucMessageLength) {
	DRV_USART_Write(DRV_UART_7, (uint8_t*)pcMessage, (uint16_t)ucMessageLength);
}

void vprvFrtReadTraceLog(){
	for (;;) {
		switch (xFrtData.eState){
		case eFrtDataStateUndef:
			vTaskDelay(1);
			break;
		case eFrtDataStateInit:
			vFrtRegisterChannel(vFrtCallback);
			xFrtData.eState = eFrtDataStateServiceTask;
			break;
		case eFrtDataStateServiceTask:
			/* Read how many events have been missed due to buffer overflow, if any */
			ulOverwrittenMessages = ulMMReadOverwrittenMessages();

			/* If any message have been overwritten, send a corresponding message to the user. */
			if (ulOverwrittenMessages){
				sprintf(pcMessage, "Overwritten %lu messages\n\r", ulOverwrittenMessages);
				for (int i = 0 ; i < ucChannelNum ; i++) (*pvChannelFunctions[i]) (pcMessage, strlen(pcMessage));
			}

			/* Acquire the current occupancy of the trace log buffer and send it to the user. */
			xFrtData.ulOccupancy = ulMMRingBufferGetOccupancy();
			/* If the buffer is empty, nothing will be sent */
			if (xFrtData.ulOccupancy){
				sprintf(pcMessage, "Occupancy: %lu/%i\n\r", xFrtData.ulOccupancy, LOG_SIZE);

				for (int i = 0 ; i < ucChannelNum ; i++) (*pvChannelFunctions[i]) (pcMessage, strlen(pcMessage));

				/* Read the whole trace log buffer and send all the read messages to all the registered channels. */
				while(!(ucMMRingBufferIsEmpty()))
				//for (uint16_t j = 0; j < xFrtData.ulOccupancy; j++)
				{
					vMMReadTraceLog(pcMessage);
					for (int i = 0 ; i < ucChannelNum ; i++) (*pvChannelFunctions[i]) (pcMessage, strlen(pcMessage));
				}
			}

			/* Delay this task for a predefined time */
			vTaskDelay(READ_TRACE_LOG_PERIOD);
			break;
		case eFrtDataStateError:
			vTaskDelay(1);
			break;
		default:
			break;
		}
	}
}

eFrtStateType xFrtInit(void)
{
	if (xFrtData.eState != eFrtDataStateUndef) return eFrtStateError;

	/* Initialize the trace log buffer */
	vMMRingBufferInit();

	/* Create a task used for reading the trace log. */
	if (xTaskCreate(vprvFrtReadTraceLog,
			"ReadTraceLogThread",
			READ_TRACE_LOG_THREAD_STACK_SIZE,
			NULL,
			READ_TRACE_LOG_PRIO,
			NULL) != pdPASS) return eFrtStateError;

	xFrtData.eState = eFrtDataStateInit;

	return eFrtStateOk;
}
