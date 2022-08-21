/*
 * This file is part of the µOS++ distribution.
 * (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "led.h"
#define CCM_RAM __attribute__(
(section(".ccmram")
)
)
#define SENDER_ONE_PRIORITY (1)
#define SENDER_TWO_PRIORITY (1)
#define RECEIVER_PRIORITY (2)
#define QUEUE_SIZE (2) //or 20
static void timer1Callback( TimerHandle_t xTimer );
static void timer2Callback( TimerHandle_t xTimer );
static void timer3Callback( TimerHandle_t xTimer );
//semaphore handles declaration
xSemaphoreHandle task1Sem = 0;
xSemaphoreHandle task2Sem = 0;
xSemaphoreHandle taskRecSem = 0;
//global queue handle declaration
xQueueHandle Global_Queue_Handle = 0;
//timer handles declaration
static TimerHandle_t xTimer1 = NULL;
static TimerHandle_t xTimer2 = NULL;
static TimerHandle_t xTimer3 = NULL;
BaseType_t xTimer1Started, xTimer2Started, xTimer3Started;
//Global variables
int Tsender;
int susuccessfullySent=0;
int blockedMessages=0;
int receivedMessage = 0;
int lowerLimit;
int upperLimit;
const int arr1 [6] = {50, 80, 110, 140, 170, 200};
const int arr2 [6]= {150, 200, 250, 300, 350, 400};
int randNum(int lower, int upper)
{
 
srand(time(0)
);
 
int num = (rand() %
 
(upper - lower + 1)
) + lower;
 
return num;
}
void printFunc()//this fun will be executed when 500 messages are received
{
 
printf("Total number of successfully sent messages : %d\n\n", susuccessfullySent);
 
printf("Total number of blocked messages : %d\n\n", blockedMessages);
 
susuccessfullySent=0;
 
blockedMessages=0;
 
receivedMessage = 0;
}
void reset()
{
 
static int iteration =0;
 
xQueueReset(Global_Queue_Handle); //Clear queue
 
if(iteration != 6) //Configure upper and lower limits
 
{
 
lowerLimit = arr1[iteration];
 
upperLimit = arr2[iteration];
 
iteration++;
 
}
 
else
 
{
 
//destroy Timer
 
xTimerDelete(xTimer1,0);
 
xTimerDelete(xTimer2,0);
 
xTimerDelete(xTimer3,0);
 
printFunc();
 
printf("Game over\n");
 
//Stop executing
 
exit(0);
 
}
}
void sender1_task(void *p) //sender
{
 
char str[20];
 
TickType_t XYZ;
 
BaseType_t status;
 
while(1)
 
{
 
XYZ = xTaskGetTickCount();
 
if(xSemaphoreTake(task1Sem,pdMS_TO_TICKS(500)
)
)
 
{
 
sprintf(str, "Time is %lu", XYZ);
 
status = xQueueSend(Global_Queue_Handle,&str,0);
 
if(status != pdPASS)
{
 
blockedMessages++;}
 
else{susuccessfullySent++;}
 
}
 
}
}
void sender2_task(void *p)
{ //sender
 
char str[20];
 
TickType_t XYZ;
 
BaseType_t status;
 
while(1)
 
{
 
XYZ = xTaskGetTickCount();
 
if(xSemaphoreTake(task2Sem,pdMS_TO_TICKS(500)
)
)
 
{
 
sprintf(str, "Time is %lu",XYZ);
 
status = xQueueSend(Global_Queue_Handle,&str,0);
 
if(status != pdPASS)
 
{blockedMessages++;}
 
else{susuccessfullySent++;}
 
}
 
}
}
void receiver_task(void *p) //Receiver
{
 
char str[20];
 
BaseType_t status;
 
while(1)
 
{
 
if(xSemaphoreTake(taskRecSem,pdMS_TO_TICKS(500)
)
)
 
{
 
status =xQueueReceive(Global_Queue_Handle,&str,0);
 
if(status==pdPASS)
 
{receivedMessage++;}
 
}
 
}
}
// ----- main() ---------------------------------------------------------------
// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
int
main(int argc, char* argv[]
)
{
 
vSemaphoreCreateBinary(task1Sem);
 
vSemaphoreCreateBinary(task2Sem);
 
vSemaphoreCreateBinary(taskRecSem);
 
Global_Queue_Handle = xQueueCreate(QUEUE_SIZE,sizeof(char)*20); //queue size var.
 
if(Global_Queue_Handle != NULL)
{
 
int Treceiver = 100;
 
reset(); // config. the boundaries
 
Tsender= randNum(lowerLimit,upperLimit);
 
xTimer1 = xTimerCreate( "TimerSender1", ( pdMS_TO_TICKS(Tsender) ), pdTRUE, ( void * ) 0, timer1Callback);
 
xTimer2 = xTimerCreate( "TimerSender2", ( pdMS_TO_TICKS(Tsender) ), pdTRUE, ( void * ) 1, timer2Callback);
 
xTimer3 = xTimerCreate( "TimerReceiver", ( pdMS_TO_TICKS(Treceiver) ), pdTRUE, ( void * ) 2, timer3Callback);
 
xTimerStart(xTimer1 ,0 );
 
xTimerStart(xTimer2 , 0 );
 
xTimerStart(xTimer3 , 0 );
 
xTaskCreate(sender1_task,
(signed char*)"Sender 1",1024,NULL,SENDER_ONE_PRIORITY,NULL);
 
xTaskCreate(sender2_task,
(signed char*)"Sender 2",1024,NULL,SENDER_TWO_PRIORITY,NULL);
 
xTaskCreate(receiver_task,
(signed char*)"Receiver",1024,NULL,RECEIVER_PRIORITY,NULL);
 
vTaskStartScheduler();
 
}
 
else{
 
puts("cannot create queue \n");
 
}
return 0;
}
#pragma GCC diagnostic pop
// ----------------------------------------------------------------------------
static void timer1Callback( TimerHandle_t xTimer )
{
 
xSemaphoreGive(task1Sem);
 
Tsender =randNum(lowerLimit,upperLimit);
 
if(
!xTimerChangePeriod(xTimer,pdMS_TO_TICKS(Tsender),0)
)
{
 
puts("failed to change the period\n");
 
}
}
static void timer2Callback( TimerHandle_t xTimer )
{
 
xSemaphoreGive(task2Sem);
 
Tsender =randNum(lowerLimit,upperLimit);
 
if(
!xTimerChangePeriod(xTimer,pdMS_TO_TICKS(Tsender),0)
)
{
 
puts("failed to change the period\n");
 
}
}
static void timer3Callback( TimerHandle_t xTimer )
{
 
xSemaphoreGive(taskRecSem);
 
if( receivedMessage == 500)
 
{reset();
 
printFunc();
 
}
}
void vApplicationMallocFailedHook( void )
{
 
/* Called if a call to pvPortMalloc() fails because there is insufficient
 
free memory available in the FreeRTOS heap. pvPortMalloc() is called
 
internally by FreeRTOS API functions that create tasks, queues, software
 
timers, and semaphores. The size of the FreeRTOS heap is set by the
 
configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
 
for( ;; );
}
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
 
( void ) pcTaskName;
 
( void ) pxTask;
 
/* Run time stack overflow checking is performed if
 
configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook
 
function is called if a stack overflow is detected. */
 
for( ;; );
}
void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;
 
/* This function is called on each cycle of the idle task. In this case it
 
does nothing useful, other than report the amout of FreeRTOS heap that
 
remains unallocated. */
 
xFreeStackSpace = xPortGetFreeHeapSize();
 
if( xFreeStackSpace > 100 )
 
{
 
/* By now, the kernel has allocated everything it is going to, so
 
if there is a lot of heap remaining unallocated then
 
the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
 
reduced accordingly. */
 
}
}
void vApplicationTickHook(void) {
}
StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
 /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
 state will be stored. */
 *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
 /* Pass out the array that will be used as the Idle task's stack. */
 *ppxIdleTaskStackBuffer = uxIdleTaskStack;
 /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
 Note that, as the array is necessarily of type StackType_t,
 configMINIMAL_STACK_SIZE is specified in words, not bytes. */
 *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;
/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *
pulTimerTaskStackSize) {
 *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
 *ppxTimerTaskStackBuffer = uxTimerTaskStack;
 *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
