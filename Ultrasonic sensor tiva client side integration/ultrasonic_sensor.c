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
 *
 ***************************************************************************************/
#include "ultrasonic_sensor.h"
volatile bool flag = 1;        // to control depending on trigger condition
volatile uint32_t echotime;     // time from when echo pin is 1
volatile uint32_t start_time;     // distance of the obstacle
volatile uint32_t data_cm;        //data in centimeters
volatile uint32_t data_inch;      //data in inches
volatile uint32_t diff;           //diff=echo-start time
volatile float calcn;             //calculation depending on clock cycles

extern QueueHandle_t xSendQueue;
extern SemaphoreHandle_t sem1;
extern SemaphoreHandle_t sem2;

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

void Motor_Init(){

    //PORT A ENABLED
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4);
    //SETTING MOTOR PIN AT PORT A PIN 4 WITH 4mA STRENGTH AND WEAK PULL UP TYPE PIN
    GPIOPadConfigSet(GPIO_PORTA_BASE,GPIO_PIN_4,GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);
    UARTprintf("\n\rMotor enabled");
}


void calculation(uint32_t start_time,uint32_t echo_time){

    //according to the ultrasonic sensor datasheet,difference between the start and end time is calculated
    diff = echo_time-start_time;
    //using this difference calculating in microseconds
    calcn = (((float)(1.0/50)*diff));
    //calculating data in cm and inches according to datasheet
    data_cm = (uint32_t)calcn/58;
    data_inch=(uint32_t)calcn/148;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
       UARTprintf("Sensor data= %d cm\n",data_cm);
       xSemaphoreGive(g_pUARTSemaphore);
    if(data_cm <= 15 ){
        //set trigger high
        GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4,GPIO_PIN_4);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4,0);
    }

//


}

void Ultrasonic_sensor_task( void * pvParameters ){

    while(1)
    {
        if( xSemaphoreTake(sem1, ( TickType_t )0) == pdTRUE )
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("Sensor task\n");
            xSemaphoreGive(g_pUARTSemaphore);
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
                //calculating the distance
                calculation(start_time,echotime);
                vTaskDelay( 5 / portTICK_PERIOD_MS );
                //to eliminate distant data
                if(data_cm < 70){
                if(xQueueSend(xSendQueue, (void *)&data_cm,(TickType_t)10) == pdPASS)
                {

                        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                        UARTprintf("\n Sending data from queue:%d\n",data_cm);
                        xSemaphoreGive(g_pUARTSemaphore);
                }
                }
                else
                {
                    xQueueReset(xSendQueue);
                }
            //flag = 0;
                xSemaphoreGive(sem2);
            }

        }
       vTaskDelay( 5 / portTICK_PERIOD_MS );
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
                //calculation(start_time,echotime);
                //flag is made 1
                flag = 1;
            }

//enable the interrupts
            IntMasterEnable();
}
