/*****************************************************************************************
*                            ADVANCED EMBEDDED SYSTEMS DESIGN
 *                              AUTHOR:RUCHA BORWANKAR
 *                              FINAL PROJECT SPRINT 2
 *                             Ultrasonic sensor working-header file 
 *
 *    TIVA peripherals: http://www.ti.com/lit/ug/spmu296/spmu296.pdf
 *    Ultrasonic sensor datasheet:https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
 *    Delay for microsecond:https://gist.github.com/ctring/7f12d812fb594eecc493
 *    Reference code: https://github.com/YashBansod/ARM-TM4C-CCS
 **************************************************************************************/

#ifndef ULTRASONIC_SENSOR_H_
#define ULTRASONIC_SENSOR_H_
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "utils/uartstdio.h"
#include "led_task.h"
#include "switch_task.h"
#include "FreeRTOS.h"
#include <string.h>
//#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "inc/hw_timer.h"           //timer
#include "driverlib/timer.h"        //timer
#include "driverlib/interrupt.h"    //interrupt

extern xSemaphoreHandle g_pUARTSemaphore;

void Ultrasonic_sensor_IRQ();
void delayUs(uint32_t ui32Us);
void Sensor_Init();
void Motor_Init();
void calculation(uint32_t start_time,uint32_t echo_time);
void Ultrasonic_sensor_task( void * pvParameters );

#endif /* ULTRASONIC_SENSOR_H_ */
