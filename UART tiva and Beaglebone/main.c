/*****************************************************************************************
*                            ADVANCED EMBEDDED SYSTEMS DESIGN
 *                              AUTHOR:RUCHA BORWANKAR
 *                              FINAL PROJECT SPRINT 1
 *                       UART DEMO for transmitting and receiving data: Beaglebone black
 *
 *      http://www.ti.com/lit/ug/spmu296/spmu296.pdf
 *
 **************************************************************************************/

/************************************************************************************
 *                             code description
 * In this code,depending on receiving or transmitting data 2 definitions are created.
 * If defined receivedat,poll the receive char check till we receive data.
 * If defined transmitdata,send data to beaglebone black
 * here polling based uart transmission is performed.
 * 9600 baud rate is used on both beaglebone and tiva boards for initial setup.
 *
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
//#include "timers.h"

//to receive from beaglebone black use "receivedata" and to transmit data to beaglebone black use "transmitdata"
#define receivedata
//#define transmitdata


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
    UARTStdioConfig(0, 9600 , 16000000);


}

void transmit_uart(){
    /***********************uart 3:TRANSMIT DATA************************************************/
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

//void receive_uart(){
////    /******************UART 2:RECEIVE DATA ***************************************************/
//           //UART2 PORT SETUP
//           ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//           ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
//
//           //UART2 PIN SETUP
//           ROM_GPIOPinConfigure(GPIO_PD6_U2RX);
//           ROM_GPIOPinConfigure(GPIO_PD7_U2TX);
//
//           //UART2 PIN CONFIGURATION
//           ROM_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
//
//           //CLOCK SETUP FOR UART 2
//           UARTClockSourceSet(UART2_BASE, UART_CLOCK_PIOSC);
//
//           //UART 2 SETUP AT 115200 BAUD RATE WITH A START BIT,8 BIT DATA AND 1 STOP BIT CONFIGURATION AT SYSTEM CLOCK OF 16000000
//           ROM_UARTConfigSetExpClk(UART2_BASE, 16000000, 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
//           UARTprintf("receive uart done\n");
//}
//*****************************************************************************
//
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

    //
    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    //
    ConfigureUART();    //UART 0 configuration to print data
    transmit_uart();    //CONFIGURE UART 3-TRANSMITTER and Receiver
 // receive_uart();

    // Create a semaphore since we need to signal the processing task
    //
//    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\nUART is setup \n");
  //  xSemaphoreGive(g_pUARTSemaphore);
//    g_pUARTSemaphore = xSemaphoreCreateMutex();

//    BaseType_t xtask1;
//   // BaseType_t xtask2;
//
//    xtask1=xTaskCreate(Transmit,"Transmit",configMINIMAL_STACK_SIZE, NULL,tskIDLE_PRIORITY, NULL);
//    if(xtask1 != pdTRUE)
//       {
//               UARTprintf("***********Task1 not created*******************\n");
//               return (1);
//       }

    //CHARACTER TRANSFER
//        UARTCharPutNonBlocking(UART3_BASE, 'A');
//        UARTCharPutNonBlocking(UART3_BASE, 'E');
//        UARTCharPutNonBlocking(UART3_BASE, 'S');
//        UARTCharPutNonBlocking(UART3_BASE, 'D');


#ifdef receivedata
while(1){


        int count=1;
        //transmit();
        while(!UARTCharsAvail(UART3_BASE)){
            UARTprintf("waiting to receive data from BeagleBone\n");
        }

        while(UARTCharGetNonBlocking(UART3_BASE)){
            //UARTprintf("arrived\n");
            char c = ROM_UARTCharGet(UART3_BASE);
            UARTprintf("%c", c,count);
            count++;
        }

}
#endif

#ifdef transmitdata
while(1){
    UARTprintf("Sending data now to Beaglebone \n");
 //   CHARACTER TRANSFER
        UARTCharPutNonBlocking(UART3_BASE, 'A');
        UARTCharPutNonBlocking(UART3_BASE, 'E');
        UARTCharPutNonBlocking(UART3_BASE, 'S');
        UARTCharPutNonBlocking(UART3_BASE, 'D');

    //STRING transfer
//            char string[]="AESD";
//            char *ptr=string;
//            int i;
//            int len=sizeof(string);
//            int count1=1;
//            UARTprintf("length of array is %d\n",len);
//            for(i=0;i<len;i++)
//            {
//                  UARTCharPutNonBlocking(UART3_BASE,*ptr);
//                  UARTprintf("letter count sent:%d\n",count1);
//                  ptr++;
//                  count1++;
//            }
}
#endif


    //
    // Start the scheduler.  This should not return.
    //
    vTaskStartScheduler();

    g_pUARTSemaphore = xSemaphoreCreateMutex();
    //
    // In case the scheduler returns for some reason, print an error and loop
    // forever.
    //

    while(1)
    {

    }
}
