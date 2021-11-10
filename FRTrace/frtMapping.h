/*
 * frtMapping.h
 *
 *  Created on: Oct 26, 2021
 *      Author: mihailo
 */
#ifndef SRC_FRTRACE_FRTMAPPING_H_
#define SRC_FRTRACE_FRTMAPPING_H_

#define configUSE_TRACE_FACILITY                 1

#undef traceTASK_CREATE
#define traceTASK_CREATE        		 vprvFrtTaskCreate

#undef traceTASK_RESUME
#define traceTASK_RESUME        		 vprvFrtTaskResume

#undef traceTASK_SUSPEND
#define traceTASK_SUSPEND	     		 vprvFrtTaskSuspend

#undef traceTASK_SWITCHED_IN
#define traceTASK_SWITCHED_IN  			 vprvFrtTaskSwitchedIn

#undef traceTASK_SWITCHED_OUT
#define traceTASK_SWITCHED_OUT  		 vprvFrtTaskSwitchedOut

#undef traceMOVED_TASK_TO_READY_STATE
#define traceMOVED_TASK_TO_READY_STATE   vprvFrtTaskMovedToReadyState

#undef traceTASK_DELETE
#define traceTASK_DELETE 				 vprvFrtTaskDelete

#undef traceQUEUE_CREATE
#define traceQUEUE_CREATE			     vprvFrtQueueCreate

#undef traceQUEUE_RECEIVE
#define traceQUEUE_RECEIVE				 vprvFrtQueueReceive

#undef traceQUEUE_SEND
#define traceQUEUE_SEND					 vprvFrtQueueSend

#undef traceQUEUE_SEND_FROM_ISR
#define traceQUEUE_SEND_FROM_ISR 		 vprvFrtQueueGiveFromISR

#undef traceBLOCKING_ON_QUEUE_RECEIVE
#define traceBLOCKING_ON_QUEUE_RECEIVE   vprvFrtBlockingOnQueueReceive

#undef traceBLOCKING_ON_QUEUE_SEND
#define traceBLOCKING_ON_QUEUE_SEND 	 vprvFrtBlockingOnQueueSend

#undef traceMALLOC
#define traceMALLOC						 vprvFrtMalloc

#undef traceFREE
#define traceFREE						 vprvFrtFree

#endif /* SRC_FRTRACE_FRTMAPPING_H_ */
