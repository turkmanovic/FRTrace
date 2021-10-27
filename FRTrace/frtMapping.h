/*
 * frtMapping.h
 *
 *  Created on: Oct 26, 2021
 *      Author: mihailo
 */

#ifndef SRC_FRTRACE_FRTMAPPING_H_
#define SRC_FRTRACE_FRTMAPPING_H_

#define configUSE_TRACE_FACILITY                 1

#define traceTASK_CREATE        		 vprvFrtTaskCreate
#define traceTASK_RESUME        		 vprvFrtTaskResume
#define traceTASK_SUSPEND	     		 vprvFrtTaskSuspend
#define traceTASK_SWITCHED_IN  			 vprvFrtTaskSwitchedIn
#define traceTASK_SWITCHED_OUT  		 vprvFrtTaskSwitchedOut
#define traceMOVED_TASK_TO_READY_STATE   vprvFrtTaskMovedToReadyState
#define traceTASK_DELETE 				 vprvFrtTaskDelete
#define traceQUEUE_CREATE			     vprvFrtQueueCreate
#define traceQUEUE_RECEIVE				 vprvFrtQueueReceive
#define traceQUEUE_GIVE					 vprvFrtQueueGive
#define traceQUEUE_SEND_FROM_ISR 		 vprvFrtQueueGiveFromISR
#define traceBLOCKING_ON_QUEUE_RECEIVE   vprvFrtBlockingOnQueueReceive
#define traceBLOCKING_ON_QUEUE_SEND 	 vprvFrtBlockingOnQueueSend
#define traceMALLOC						 vprvFrtMalloc
#define traceFREE						 vprvFrtFree

#endif /* SRC_FRTRACE_FRTMAPPING_H_ */
