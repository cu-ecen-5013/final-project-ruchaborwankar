/*****************************************************************************************
*                            ADVANCED EMBEDDED SYSTEMS DESIGN
 *                              AUTHOR:RUCHA BORWANKAR
 *                              FINAL PROJECT SPRINT 2
 *                              Message Queue working
 *
 *
 *      freertos queue example code:https://www.freertos.org/a00118.html
 **************************************************************************************/

/************************************************************************************
 *                             code description
 * In this code,
 * simple message queue is implemented with two tasks:send and receive.
 * A structure "AMessage" is passed from send to receive part with data:80 and messageid=1.
 * Two semaphores S1 and S2 are used wherein S1 is released after thread creation in order to ensure that
 * send occurs first and after that send thread releases the semaphore S2 for the second thread.
 * In receive thread after data is received the thread release semaphore S1 in order to ensure sending of data.
 ***************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include <string.h>
#include "led_task.h"
#include "switch_task.h"
#include "FreeRTOS.h"
//#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


QueueHandle_t xQueue;
SemaphoreHandle_t sem1;

SemaphoreHandle_t sem2;
xSemaphoreHandle g_pUARTSemaphore;
typedef struct AMessage{
   int qMessageID;
   int qData;
}xMessage;

//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
xSemaphoreHandle g_pUARTSemaphore;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************


void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    /***********************uart 0-FOR PRINTING ON CONSOLE************************************************/
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);

}


void receive_queue( void * pvParameters ){
    xMessage rec_data;
    while(1){
           if( xSemaphoreTake(sem2, ( TickType_t )0) == pdTRUE )
           {
               //receive queue parameters-queue handle,pointer to variable where data is to be received,max amount of time to be blocked
               if(xQueueReceive(xQueue, (void *)&rec_data ,(TickType_t)10) == pdPASS)
                   {
                             xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                             UARTprintf("Data received is message id=%d\t data=%d\n",rec_data.qMessageID,rec_data.qData);
                             xSemaphoreGive(g_pUARTSemaphore);
                   }
           }
           xSemaphoreGive(sem1);
       }

}

void send_queue( void * pvParameters ){
    xMessage data;

    while(1){
        if( xSemaphoreTake(sem1, ( TickType_t )0) == pdTRUE )
                {

                     data.qMessageID=1;
                     data.qData= 80;
                     //send queue parameters-queue handle,pointer to data to be sent,max amount of time to be blocked
                     if(xQueueSend(xQueue, (void *)&data ,(TickType_t)10) == pdPASS)
                        {

                            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                            UARTprintf("Sending data to queue\n");
                            xSemaphoreGive(g_pUARTSemaphore);

                        }
                }
        xSemaphoreGive(sem2);
    }

}

//*****************************************************************************//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************

int main(void)
{
    //
    // Set the clocking to run at 50 MHz from the PLL.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    // Create a semaphore since we need to signal the processing task
        //

          sem1 = xSemaphoreCreateBinary();
          sem2 = xSemaphoreCreateBinary();
    g_pUARTSemaphore = xSemaphoreCreateMutex();
    //
        // Initialize the UART0 and configure it for 115,200, 8-N-1 operation.
        //
    ConfigureUART();
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\nUART is setup \n");
    xSemaphoreGive(g_pUARTSemaphore);

       /* Create a queue with size 200 */
       xQueue = xQueueCreate( 200, sizeof( xMessage ));

       if( xQueue == NULL )
          {
           xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
           UARTprintf("Error....Queue not created\n");
           xSemaphoreGive(g_pUARTSemaphore);
          }

       /*task create function arguments passed
* taskcreate(name of function,descriptive name of the task,stack size of task,parameters passed,priority,pass handle to task(null))*/
       if(xTaskCreate(send_queue, (const portCHAR *)"send queue",configMINIMAL_STACK_SIZE, NULL, 1, NULL) == pdPASS)
           {
                   xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                    UARTprintf("Send task is successfully created\n");
                    xSemaphoreGive(g_pUARTSemaphore);

           }
       if((xTaskCreate(receive_queue, (const portCHAR *)"receive queue",configMINIMAL_STACK_SIZE, NULL, 1, NULL)) == pdPASS )
                  {
                         xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                         UARTprintf("Receive task is successfully created\n");
                         xSemaphoreGive(g_pUARTSemaphore);
                  }
       //give semaphore 1 to initiate the sending of data to queue
       xSemaphoreGive(sem1);

       vTaskStartScheduler();
    while(1)
    {

    }

}
