

#include "tmp102.h"
#include "fingerprtsens.h"
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


extern xQueueHandle queue_handle;
void temp_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    MAP_I2CMasterInitExpClk(I2C0_BASE,120000000U,false);
}


//REFERENCE: https://github.com/jajoosiddhant/Two-Factor-Authentication-System/blob/master/remote_node/src/temp.c
//Link: http://www.ti.com/lit/ds/symlink/tm4c1294ncpdt.pd   ::::Datasheet pg no. 1290, 1297

uint32_t temp_read(void)
{
    uint32_t read_value;
    //char arr[20];
    //uint8_t i=0;
    I2CMasterDataPut(I2C0_BASE,0x00);
    I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
    while(!I2CMasterBusy(I2C0_BASE));
    while(I2CMasterBusy(I2C0_BASE));
    I2CMasterSlaveAddrSet(I2C0_BASE,SLAVE_ADDRESS,true);
    I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(!I2CMasterBusy(I2C0_BASE));
    while(I2CMasterBusy(I2C0_BASE));
    read_value = I2CMasterDataGet(I2C0_BASE);
    read_value = read_value << 8;
    I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    while(!I2CMasterBusy(I2C0_BASE));
    while(I2CMasterBusy(I2C0_BASE));
    read_value |= I2CMasterDataGet(I2C0_BASE);
    read_value = read_value >> 4;
    read_value = read_value*(0.0625);
   /* ftoa(read_value,arr,3);
    arr[6] = '\n';
    arr[7] = '\r';
    UARTSend((uint8_t *)arr,20);
    for(i=0; i<255; i++);  */
    return read_value;

}

void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        MAP_UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
    }
}



///////////////////////// FreeRTOS section //////////////////////////////////

uint8_t TmpTaskInit(void)

{
   if(xTaskCreate(tmp_task, (const portCHAR *)"tmp102",128, NULL,
                      tskIDLE_PRIORITY, NULL) != pdTRUE)
       {
           return(1);
       }
       /*  xTaskCreate(fingerprint_task,"Fprt",128, NULL,
                          tskIDLE_PRIORITY, NULL);       */
        //
        // Success.
        //
   UARTprintf(" Task created \n");
    return 0;
}



void tmp_task(void *pvParameters)
{
    UARTprintf("2: In temperature thread \n ");
    uint32_t counter = 0;
    uint32_t temperature = temp_read();
    while(1)
    {
        //counter = counter + 1;
        UARTprintf("Start of while loop\n ");

       // xQueueReset( QueueHandle_t xQueue );


  /*      if (counter<9))
        {
            xQueueSend(queue_handle, &temperature, 2000);
        }
        else
            {
            xQueueReset( QueueHandle_t xQueue );
            }    */

       if ( xQueueSend(queue_handle, &temperature, 1000) == pdTRUE)
           UARTprintf("sent! %d \n",temperature);
       else
       {
           UARTprintf("################ Queue Full ############### \n");
           xQueueReset( queue_handle );
           counter = 0;
       }
       UARTprintf("Stop of while loop\n ");
    }
    UARTprintf("Stop of thread\n ");
}





