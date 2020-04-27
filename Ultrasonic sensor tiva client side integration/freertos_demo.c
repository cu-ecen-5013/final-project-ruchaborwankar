/*****************************************************************************************
*                            ADVANCED EMBEDDED SYSTEMS DESIGN
 *                              AUTHOR:RUCHA BORWANKAR
 *                              FINAL PROJECT SPRINT 3
 *                              TIVA final client side
 *
 *
 *      freertos queue example code:https://www.freertos.org/a00118.html
 **************************************************************************************/
/************************************************************************************
 *                             code description
 * In this code,
 * message queue implementation,ultrasonic sensor implementation and uart are all combined to form client side on tiva.
 * Two tasks are created:ultrasonic sensor task and receive task
 * Semaphore sem1 is released
 * Ultrasonic sensor task:Sem1 semaphore is accepted by the task and Sensor gets the readings ,sends it to queue.Semaphore sem2 is released.
 * Receive task:sem2 is accepted by the task,it receives data from the queue,converts data to string and then passes it to Beaglebone by UART3
 ***************************************************************************************/
//Header files
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
#include "ultrasonic_sensor.h"      //ultrasonic sensor code linked

QueueHandle_t xSendQueue;
//QueueHandle_t xRecvQueue;
SemaphoreHandle_t sem1;                 //task 1 control
SemaphoreHandle_t sem2;                 //task 2 control

xSemaphoreHandle g_pUARTSemaphore;      //for uart

//typedef struct AMessage{
//   int qMessageID;
//   int qData;
//}xMessage;

extern uint32_t data_cm;                //global sensor data in centimeters.

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
// UARTprintf configuration
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

/***********************UART 3 TO TRANSMIT TO BBB************************************************/
void transmit_uart(){

        //UART3 PORT SETUP
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);

        //UART3 PIN SETUP
        ROM_GPIOPinConfigure(GPIO_PC6_U3RX);
        ROM_GPIOPinConfigure(GPIO_PC7_U3TX);

        //UART3 PIN CONFIGURATION
        ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);

        //CLOCK SETUP FOR UART 3
        UARTClockSourceSet(UART3_BASE, UART_CLOCK_PIOSC);

        //UART 3 SETUP AT 9600 BAUD RATE WITH A START BIT,8 BIT DATA AND 1 STOP BIT CONFIGURATION AT SYSTEM CLOCK OF 16000000
        ROM_UARTConfigSetExpClk(UART3_BASE,16000000, 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
        UARTprintf("transmit uart done\n");
}

//itoa function from Ritchie c programming book:https://en.wikibooks.org/wiki/C_Programming/stdlib.h/itoa
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert uint32_t n to characters in s */
 void itoa(uint32_t n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }


/************convert using itoa and send data to bb***************************/
void send_to_bb(uint32_t data){
    char str[5];
    itoa(data,str);
    UARTprintf("itoa String is %s\n",str);
    int len=sizeof(str);
    int i;
//    for ( i=0; i<len; i++)
//           {
//               //UARTCharPutNonBlocking(UART3_BASE,str[i]);
//               UARTprintf("%c",str[i]);
//           }
       for ( i=0; i<len; i++)
       {
           UARTCharPutNonBlocking(UART3_BASE,str[i]);
           //UARTprintf("%c",str[i]);
       }

}

/**************RECEIVE THREAD***********************/
void receive_queue( void * pvParameters ){
    uint32_t rec_data;
    while(1){
           if( xSemaphoreTake(sem2, ( TickType_t )0) == pdTRUE )
           {
               UARTprintf("Receive task\n");
               //receive queue parameters-queue handle,pointer to variable where data is to be received,max amount of time to be blocked
               if(xQueueReceive(xSendQueue, (void *)&rec_data ,(TickType_t)10) == pdPASS)
                   {
                             xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                             UARTprintf("Sending the Data received from queue data=%d\n",rec_data);
                             xSemaphoreGive(g_pUARTSemaphore);
                             //send_to_bb(rec_data);
                             UARTCharPutNonBlocking(UART3_BASE,(uint8_t)rec_data);

                   }
           }

           xQueueReset(xSendQueue);
           xSemaphoreGive(sem1);
       }

}

//void send_queue( void * pvParameters ){
//
//
//    while(1){
//        if( xSemaphoreTake(sem1, ( TickType_t )0) == pdTRUE )
//                {
//                     //send queue parameters-queue handle,pointer to data to be sent,max amount of time to be blocked
//                     if(xQueueSend(xSendQueue, (void *)&data_cm,(TickType_t)10) == pdPASS)
//                        {
//
//                            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
//                            UARTprintf("Sending data from queue:%d\n",data_cm);
//                            xSemaphoreGive(g_pUARTSemaphore);
//
//                        }
//                }
//        xSemaphoreGive(sem2);
//    }
//
//}

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
    Sensor_Init();
    transmit_uart();
    Motor_Init();

    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n****************Welcome to TIVA client 3*********\n");
    xSemaphoreGive(g_pUARTSemaphore);

       /* Create a queue with size 200 */
       xSendQueue = xQueueCreate( 200, 32);
       //xRecvQueue = xQueueCreate( 200, 32);
       if( (xSendQueue) == NULL )
       {
           xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
           UARTprintf("Error....send or receive Queue not created\n");
           xSemaphoreGive(g_pUARTSemaphore);
       }

       /*task create function arguments passed
        *  taskcreate(name of function,descriptive name of the task,stack size of task,parameters passed,priority,pass handle to task(null))*/
       if(xTaskCreate(Ultrasonic_sensor_task, (const portCHAR *)"Ultrasonic sensor",configMINIMAL_STACK_SIZE, NULL, 1, NULL) == pdPASS)
                  {
                          xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                           UARTprintf("Ultrasonic Sensor task is successfully created\n");
                           xSemaphoreGive(g_pUARTSemaphore);

                  }
//       if(xTaskCreate(send_queue, (const portCHAR *)"send queue",configMINIMAL_STACK_SIZE, NULL, 1, NULL) == pdPASS)
//           {
//                   xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
//                    UARTprintf("Send task is successfully created\n");
//                    xSemaphoreGive(g_pUARTSemaphore);
//
//           }

       if((xTaskCreate(receive_queue, (const portCHAR *)"receive queue",configMINIMAL_STACK_SIZE, NULL, 1, NULL)) == pdPASS )
                  {
                         xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                         UARTprintf("Receive task is successfully created\n");
                         xSemaphoreGive(g_pUARTSemaphore);
                  }

       //give semaphore 1 to initiate the sending of data to queue using sensor data
       xSemaphoreGive(sem1);

       vTaskStartScheduler();
    while(1)
    {
              xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
              UARTprintf("******************Error**********************\n");
              xSemaphoreGive(g_pUARTSemaphore);

    }

}
