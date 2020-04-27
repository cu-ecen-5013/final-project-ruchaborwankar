/*****************************************************************************************
*                            ADVANCED EMBEDDED SYSTEMS DESIGN
 *                              AUTHOR:RUCHA BORWANKAR
 *                              FINAL PROJECT SPRINT 2
 *                             Ultrasonic sensor working
 *
 *    TIVA peripherals: http://www.ti.com/lit/ug/spmu296/spmu296.pdf
 *    Ultrasonic sensor datasheet:https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
 *    Delay for microsecond:https://gist.github.com/ctring/7f12d812fb594eecc493
 *    Reference code: https://github.com/YashBansod/ARM-TM4C-CCS
 **************************************************************************************/

/************************************************************************************
 *                             code description
 * In this code,
 * ultrasonic sensor is interfaced with vcc at 5v(from other board since TIVA C series 123G board has 3.3V on board)
 * gnd,trigger at PA5 and echo at PA6.
 * Ground pins of sensor,tiva c series board and other board which is connected for power supply are connected together.
 * Since,Echo pin gives 5 v output in order to make it compatible for Tiva 123G board,a voltage divider circuit is implemented to
 * bring down the voltage to 3.3V and then applied to the PA6 pin of the TIVA board.
 * 3 1k ohm resistor is used to form the voltage divider circuit.
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
#include "inc/hw_timer.h"           //timer
#include "driverlib/timer.h"        //timer
#include "driverlib/interrupt.h"    //interrupt

volatile bool flag = 1;        // to control depending on trigger condition
volatile uint32_t echotime;     // time from when echo pin is 1
volatile uint32_t start_time;     // distance of the obstacle
volatile uint32_t data_cm;        //data in centimeters
volatile uint32_t data_inch;      //data in inches
volatile uint32_t diff;           //diff=echo-start time
volatile float calcn;             //calculation depending on clock cycles

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

void Ultrasonic_sensor_IRQ();

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

void delayUs(uint32_t ui32Us) {

/***************************************************
 * // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 1 microsecond = 1 us = SysCtlClockGet() / 3 / 1000000
 * *********************************************/
    SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}

void Sensor_Init(){
    //enabling timer 1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    //timer config
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC_UP);
    UARTprintf("\n\rtimer enabled");

    //Trigger pin enabling at pin PA 5 //OUTPUT TYPE OF PIN

    //PORT A ENABLED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);
    //SETTING TRIGGER PIN AT PORT A PIN 5 WITH 2mA STRENGTH AND WEAK PULL UP TYPE PIN
    GPIOPadConfigSet(GPIO_PORTA_BASE,GPIO_PIN_5,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    UARTprintf("\n\rTrigger enabled");

    //ECHO ENABLED AT PIN PA6 //INPUT TYPE OF PIN
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_INT_PIN_6);
    //SET INTERRUPT AT PIN WITH BOTH EDGE DETECTION
    GPIOIntTypeSet(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_BOTH_EDGES );
    //SETS THE INTERRUPT HANDLER
    GPIOIntRegister(GPIO_PORTA_BASE,Ultrasonic_sensor_IRQ);
    // CLEAR INTERRUPTS
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_INT_PIN_6);
    UARTprintf("\n\rEcho enabled");

}


//void transmit_uart(){
//    /***********************uart 3:TRANSMIT DATA************************************************/
//        //UART3 PORT SETUP
//        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
//        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);
//
//        //UART3 PIN SETUP
//        ROM_GPIOPinConfigure(GPIO_PC6_U3RX);
//        ROM_GPIOPinConfigure(GPIO_PC7_U3TX);
//
//        //UART3 PIN CONFIGURATION
//        ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
//
//        //CLOCK SETUP FOR UART 3
//        UARTClockSourceSet(UART3_BASE, UART_CLOCK_PIOSC);
//
//        //UART 3 SETUP AT 9600 BAUD RATE WITH A START BIT,8 BIT DATA AND 1 STOP BIT CONFIGURATION AT SYSTEM CLOCK OF 16000000
//        ROM_UARTConfigSetExpClk(UART3_BASE,16000000, 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
//        UARTprintf("transmit uart done\n");
//}


void calculation(uint32_t start_time,uint32_t echo_time){

    //according to the ultrasonic sensor datasheet,difference between the start and end time is calculated
    diff = echo_time-start_time;
    //using this difference calculating in microseconds
    calcn = (((float)(1.0/50)*diff));
    //calculating data in cm and inches according to datasheet
    data_cm = (uint32_t)calcn/58;
    data_inch=(uint32_t)calcn/148;
//
    UARTprintf("Sensor data obtained is %d cm and %d inches\n",data_cm,data_inch);

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
    ConfigureUART();    //UART 0 configuration to print data
    UARTprintf("\nUART is setup \n");
    Sensor_Init();      //initialize all the pins for ultrasonic sensor
    UARTprintf("\nSensor is ready and setup \n");
    // Create a semaphore since we need to signal the processing task
    //
//    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);

    while(1)
    {

        while(flag != 0)
        {
            /**********************************
            * according to the hc-sr04 datasheet,
            * 10usec short pulse is required to start
            * ranging and raise its echo
            *************************************/
           //set trigger high
            GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5,GPIO_PIN_5);
           //set 10usec delay
            delayUs(10);
           // set trigger low
            GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5,0);
            flag = 0;
        }
    }

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

void Ultrasonic_sensor_IRQ(){
//disable all interrupts
        IntMasterDisable();
//clear the interrupt on echo pin
            GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_6);
            // if echo pin is high
            if (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6) == GPIO_PIN_6)
            {
                //set Timer1 with value 0 //hwreg=hardware register,32 bit
                HWREG(TIMER1_BASE + TIMER_O_TAV) = 0;
                // Timer 1 enabled to measure time
                TimerEnable(TIMER1_BASE, TIMER_A);
                //start time recorded
                start_time=TimerValueGet(TIMER1_BASE,TIMER_A);
                flag=0;
            }
            else{
                //time from echo calculated
                echotime = TimerValueGet(TIMER1_BASE, TIMER_A);
                // Disable Timer1 to stop calculating time for echo pin=1
                TimerDisable(TIMER1_BASE, TIMER_A);
                //calculating the distance
                calculation(start_time,echotime);
                //flag is made 1
                flag = 1;
            }

//enable the interrupts
            IntMasterEnable();
}
