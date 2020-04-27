#ifndef TMP102_H_

#define TMP102_H_


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
//#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include <math.h>
#include "utils/uartstdio.h"

#define SLAVE_ADDRESS 0x48  //Have to complete

void temp_init(void);
void temp_read(void);
void reverse(char* str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char* res, int afterpoint);
//void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count);
#endif
