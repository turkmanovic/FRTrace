# FRTrace #
Trace functionality for FreeRTOS

## Instructions for including the tracing library into your project ##
In order to include the library you need to do the following steps:
* Copy the FRTrace folder into your projects Source folder
* Add the following include to the FreeRTOSConfig.h file
  * #include "../Src/FRTrace/frtMapping.h"
* Add the following include to your main.c file
  * #include "FRTrace/frt.h"
* Initialize the tracing mechanism by calling xFrtInit() inside your main.c file
* In order to review the trace messages you need to register all the receiving channels with the vFrtRegisterChannel function
  * The function receives a pointer to a function which drives the trace messages to a specific interface

## Configuration ##
Depending on the number of required channels for transmitting the trace messages, the maximum number of channels can be modified by changing the following define inside FRTrace/frt.h
* MAX_NUMBER_OF_CHANNELS 

The tracing mechanism will print out status messages indicating the occupancy of the message buffer. An overflow might happen depending on the number of traced messages. The user will be notified of this occurence and can configure the buffer size, in order to avoid this issue, by changing the following define inside FRTrace/mm.h
* LOG_SIZE

This library should be used as a background mechanism used for displaying tracing messages whenever there is no task running. The priority of the FRTrace task, used for sending trace messages, should be set accordingly to the task priorities used in the current project. The FRTrace task priority can be set by changing the following define inside FRTrace/frt.h
* READ_TRACE_LOG_PRIO


