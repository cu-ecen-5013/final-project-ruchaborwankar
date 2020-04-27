/*****************************************************************************************
*                            ADVANCED EMBEDDED SYSTEMS DESIGN
 *                              AUTHOR:RUCHA BORWANKAR
 *                              FINAL PROJECT SPRINT 1
 *                              UART DEMO FOR LOOPBACK
 *
 *      http://www.ti.com/lit/ug/spmu296/spmu296.pdf
 *
 **************************************************************************************/

/************************************************************************************
 *
 * In this code,a UART loopback is implemented wherein UART 3 is the transmitter UART and
 * UART 2 is the receiver UART.Characters "AESD" were sent on UART 3 and then received on UART 2 and printed using
 * UART 0 printf.
 * Thus,UART functionality is successful on tiva c series board.
 * here polling based uart transmission is performed.
 *
 */
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


//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>FreeRTOS Example (freertos_demo)</h1>
//!
//! This application demonstrates the use of FreeRTOS on Launchpad.
//!
//! The application blinks the user-selected LED at a user-selected frequency.
//! To select the LED press the left button and to select the frequency press
//! the right button.  The UART outputs the application status at 115,200 baud,
//! 8-n-1 mode.
//!
//! This application utilizes FreeRTOS to perform the tasks in a concurrent
//! fashion.  The following tasks are created:
//!
//! - An LED task, which blinks the user-selected on-board LED at a
//!   user-selected rate (changed via the buttons).
//!
//! - A Switch task, which monitors the buttons pressed and passes the
//!   information to LED task.
//!
//! In addition to the tasks, this application also uses the following FreeRTOS
//! resources:
//!
//! - A Queue to enable information transfer between tasks.
//!
//! - A Semaphore to guard the resource, UART, from access by multiple tasks at
//!   the same time.
//!
//! - A non-blocking FreeRTOS Delay to put the tasks in blocked state when they
//!   have nothing to do.
//!
//! For additional details on FreeRTOS, refer to the FreeRTOS web page at:
//! http://www.freertos.org/
//
//*****************************************************************************



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

        //UART 3 SETUP AT 115200 BAUD RATE WITH A START BIT,8 BIT DATA AND 1 STOP BIT CONFIGURATION AT SYSTEM CLOCK OF 16000000
        ROM_UARTConfigSetExpClk(UART3_BASE,16000000, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
        UARTprintf("transmit uart done\n");
}

void receive_uart(){
//    /******************UART 2:RECEIVE DATA ***************************************************/
           //UART2 PORT SETUP
           ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
           ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

           //UART2 PIN SETUP
           ROM_GPIOPinConfigure(GPIO_PD6_U2RX);
           ROM_GPIOPinConfigure(GPIO_PD7_U2TX);

           //UART2 PIN CONFIGURATION
           ROM_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

           //CLOCK SETUP FOR UART 2
           UARTClockSourceSet(UART2_BASE, UART_CLOCK_PIOSC);

           //UART 2 SETUP AT 115200 BAUD RATE WITH A START BIT,8 BIT DATA AND 1 STOP BIT CONFIGURATION AT SYSTEM CLOCK OF 16000000
           ROM_UARTConfigSetExpClk(UART2_BASE, 16000000, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
           UARTprintf("receive uart done\n");
}
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
    ConfigureUART();
    transmit_uart();    //CONFIGURE UART 3-TRANSMITTER
    receive_uart();     //CONFIGURE UART 2-RECEIVER

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
//        UARTprintf("Transfer data from UART 3\n");
//        UARTCharPutNonBlocking(UART3_BASE, 'A');
//        UARTCharPutNonBlocking(UART3_BASE, 'E');
//        UARTCharPutNonBlocking(UART3_BASE, 'S');
//        UARTCharPutNonBlocking(UART3_BASE, 'D');

     //STRING TRANSFER
        UARTprintf("Transfer data from UART 3\n");
        char string[]="AESD final project";
        char *ptr=string;
        int i;
        int len=sizeof(string);
        int count1=1;
        UARTprintf("length of array is %d\n",len);
        for(i=0;i<len;i++)
        {
            UARTCharPutNonBlocking(UART3_BASE,*ptr);
            UARTprintf("count;%d\n",count1);
            ptr++;
            count1++;
        }
    
        int count=1;
        UARTprintf("Receive data at UART 2\n");
        while(true){

            char c = ROM_UARTCharGet(UART2_BASE);
            UARTprintf("Received character: %c at count %d\r\n", c,count);
            count++;
        }

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
