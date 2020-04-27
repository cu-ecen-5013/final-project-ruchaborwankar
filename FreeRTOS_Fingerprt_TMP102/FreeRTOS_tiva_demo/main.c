/* Author : Atharv Desai
 * FreeRTOS 10 Tiva Demo
 *
 * main.c
 *
 * Reference : Andy Kobyljanec
 *
 * This is a simple demonstration project of FreeRTOS 8.2 on the Tiva Launchpad
 * EK-TM4C1294XL.  TivaWare driverlib sourcecode is included.
 */

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "drivers/pinout.h"
#include "utils/uartstdio.h"


// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "tmp102.h"

// Demo Task declarations
void demoLEDTask(void *pvParameters);
void demoSerialTask(void *pvParameters);

void uart3_config(void);
void ConfigureUART4(void);

//****************************************************************************
//
// System clock rate in Hz.
//
//****************************************************************************
uint32_t g_ui32SysClock;
// Initialize system clock to 120 MHz
 uint32_t output_clock_rate_hz;

 //*****************************************************************************
 //
 // The mutex that protects concurrent access of UART from multiple tasks.
 //
 //*****************************************************************************
 xSemaphoreHandle g_pUARTSemaphore;
SemaphoreHandle_t xSemaphore = NULL;



 ///////////////////// Queue Handle /////////////////////////////////////
 xQueueHandle queue_handle = NULL;


// Main function
int main(void)
{

    output_clock_rate_hz = ROM_SysCtlClockFreqSet(
                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                               SYSTEM_CLOCK);
    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);

    // Set up the UART which is connected to the virtual COM port
    UARTStdioConfig(0, 9600, SYSTEM_CLOCK);

    UARTprintf(" Uart 0 working \n");
    // Initialize Uart 3 configuration
    uart3_config();
    interrupt_config();
    temp_init();
    ConfigureUART4();

/////////////////////  Semaphore Creation //////////////////////////////
xSemaphore = xSemaphoreCreateBinary();

////////////////////////////////////////////////////////////////////////

    // Initialize the GPIO pins for the Launchpad
    PinoutSet(false, false);

    // Create demo tasks
/*   xTaskCreate(demoLEDTask, (const portCHAR *)"LEDs",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);

     xTaskCreate(demoSerialTask, (const portCHAR *)"Serial",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);

*/

    if(FingerprintTaskInit() != 0)
     {
        UARTprintf("Fingerprint task not created \n");
         while(1)
         {
         }
     }

    if(TmpTaskInit() != 0)
      {
         UARTprintf("Fingerprint task not created \n");
          while(1)
          {
          }
      }


    //////////////// Queue Creation ///////////////////////////

    queue_handle = xQueueCreate(10, sizeof(int));

   vTaskStartScheduler();

    // Code should never reach this point
    return 0;
}


// Flash the LEDs on the launchpad
void demoLEDTask(void *pvParameters)
{
    UARTprintf("1:  Thread LED \n ");
    for (;;)
    {
        // Turn on LED 1
        LEDWrite(0x0F, 0x01);
        vTaskDelay(1000);

        // Turn on LED 2
        LEDWrite(0x0F, 0x02);
        vTaskDelay(1000);

        // Turn on LED 3
        LEDWrite(0x0F, 0x04);
        vTaskDelay(1000);

        // Turn on LED 4
        LEDWrite(0x0F, 0x08);
        vTaskDelay(1000);
    }
}



/*  ASSERT() Error function
 *
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}



void uart3_config(void)
{



        // Enable the peripherals used by this example.
        //
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);


        //
           // Set GPIO A0 and A1 as UART pins.
           //
           GPIOPinConfigure(GPIO_PA4_U3RX);
           GPIOPinConfigure(GPIO_PA5_U3TX);
           ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5);

           //
           // Configure the UART for 9600, 8-N-1 operation.
           //
           ROM_UARTConfigSetExpClk(UART3_BASE, output_clock_rate_hz, 9600,
                                   (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                    UART_CONFIG_PAR_NONE));


}

void ConfigureUART4(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA2_U4RX);
    ROM_GPIOPinConfigure(GPIO_PA3_U4TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    ROM_UARTConfigSetExpClk(UART4_BASE, output_clock_rate_hz, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));



    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
   /* UARTClockSourceSet(UART4_BASE, UART_CLOCK_PIOSC);
    ROM_UARTConfigSetExpClk(UART4_BASE,16000000, 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));   */

    //
    // Initialize the UART for console I/O.
    //
    //UARTStdioConfig(0, 115200, 16000000);
   // UARTEnable(UART4_BASE);
}


