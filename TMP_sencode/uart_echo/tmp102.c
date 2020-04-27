#include "tmp102.h"

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

void temp_read(void)
{
    uint32_t read_value;
    char arr[20];
    int i=0, j=0;
    char c;
    bool ret, ret1;
    //uint32_t val;
    //UARTprintf("HELLO %d",i);
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
    UARTprintf("Temperature is:: read_value: %d\n",read_value);
    ftoa(read_value,arr,1);
   // arr[6] = '\0';
   // UARTprintf("arr[0]:%c\n",arr[0]);
    //UARTprintf("arr[1]:%c\n",arr[1]);
   for(j=0; j<2; j++)
    {
       UARTCharPutNonBlocking(UART4_BASE,arr[j]);
        while(UARTBusy(UART4_BASE));
    }
   UARTCharPutNonBlocking(UART4_BASE,'\n');
   while(UARTBusy(UART4_BASE));
   // UARTCharPut(UART4_BASE,arr[6]);
   // while(UARTBusy(UART4_BASE));
   /* UARTCharPutNonBlocking(UART4_BASE,'a');
    UARTCharPutNonBlocking(UART4_BASE,'b');
    UARTCharPutNonBlocking(UART4_BASE,'c');
    UARTCharPutNonBlocking(UART4_BASE,'d');*/

   // UARTprintf("value of c is: %c\n",c);

    //arr[6] = '\n';
    //arr[7] = '\r';
   // UARTSend((uint8_t *)arr,20);
    //for(i=0; i<255; i++);
   // return 0;
   // return read_value;
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

/*void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
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
}*/


