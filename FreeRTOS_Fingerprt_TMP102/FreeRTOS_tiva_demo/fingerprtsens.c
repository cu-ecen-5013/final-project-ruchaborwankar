/* Author : Atharv Desai
 * Fingerprint sensor C file for GT-521F32 sensor
 * References: 1) https://github.com/sparkfun/Fingerprint_Scanner-TTL/blob/master/src/FPS_GT511C3.h
 *             2) GT-521F52_Programming_guide
 *             3) GT-521FX2_datasheet_V1.1__003_.pdf
 */


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
uint8_t fmatchflag = 0;
extern xQueueHandle queue_handle;
extern SemaphoreHandle_t xSemaphore;
void Send_cmd_pkt(uint8_t parameter, uint8_t code)
{
    uint8_t x = 0;
    uint8_t command_packet[CMDPKT_SIZE] = { CMD_START_1, CMD_START_2, COMMAND_DEVICE_ID_1, COMMAND_DEVICE_ID_2 ,parameter, 0,0,0, code,0,0,0 };

    uint16_t Checksum_value = Checksum(command_packet);
    command_packet[10] = Checksum_value & 0x00FF;
    command_packet[11] = (Checksum(command_packet) & 0xFF00) >> 8;

    for(x=0; x < CMDPKT_SIZE; x++)
    {
        UARTCharPut(UART3_BASE, command_packet[x]);
    }
    UARTprintf(" Sending packet ");
    for(x=0; x < CMDPKT_SIZE; x++)
       {
           UARTprintf("%X ",command_packet[x]);
           while(UARTBusy(UART3_BASE));
       }
    UARTprintf(" \n \n");
}


uint8_t Send_rsp_pkt(void)
{
    uint8_t x = 0;
    //uint16_t temp=0;
    //uint16_t myresp =0;
    uint8_t response_pkt[12];

    for (x=0; x < 12; x++)
        {
        response_pkt[x] = (uint8_t)(UARTCharGet(UART3_BASE)&0x000000FF);
        }
    UARTprintf(" Received packet ");
    for(x=0; x < 12; x++)
       {
           UARTprintf("%X ",response_pkt[x]);
       }
    UARTprintf(" \n \n");
  return response_pkt[8];
}


void interrupt_handler(void)
{
    uint8_t checker =0;

    IntMasterDisable();  //Disable interrupts to the processor
    UARTprintf("\n Finger Pressed : In the interrupt handler\n");
    Send_cmd_pkt(1,CMD_CMOSLED);
    Send_rsp_pkt();
    UARTprintf("Checked above response\n");
    //UARTSend((uint8_t *)"Interrupt\n\r",12);
    if(GPIOIntStatus(ICPCK_PORT,true) & ICPCK_PIN)               //Returns the ICPCK raw interrupt status
    {
        //UARTSend((uint8_t *)"Port-Stat",18);
        GPIOIntClear(ICPCK_PORT, GPIOIntStatus(ICPCK_PORT,false) & ICPCK_PIN) ; //Clears the specified ICPCK interrupt source
        capture_fingerprt();
        Send_rsp_pkt();
        Identify_fingerprt();
        checker = Send_rsp_pkt();
        if(checker==48)    //GT-521F32 can store upto 200 fingerprints
                 {
                     UARTprintf("Fingerprint Matched : %d \n", checker);
                     fmatchflag = 1;
                     xSemaphoreGive(xSemaphore);

                 }
                 else
                 {
                     UARTprintf("Not Matched  %d \n", checker);
                     //UARTSend((uint8_t *)"Not Matched\n",50);
                 }
      /*   if(Identify_fingerprt()==0x30)    //GT-521F32 can store upto 200 fingerprints
         {
             UARTprintf("Fingerprint Matched\n");
             //UARTSend((uint8_t *)" Fingerprint Matched\n\r",50);

         }
         else
         {
             UARTprintf("Not Matched \n ");
             //UARTSend((uint8_t *)"Not Matched\n",50);
         }   */
        // UARTSend((uint8_t *)"Checkpoint 3: Exiting bigger if interrupt handler",80);

    }
   // UARTSend((uint8_t *)" Checkpoint 4: Outside the interrupt handler",30);
    IntMasterEnable();   // Enable the interrupt again
    UARTprintf("\n//////////////////////////////////////////////////////////////////////\n");
    Send_cmd_pkt(0,CMD_CMOSLED);
    Send_rsp_pkt();
}


uint8_t Identify_fingerprt(void)
{
   // Send_cmd_pkt(0,CMD_CMOSLED);

   // Send_cmd_pkt(1,CMD_CMOSLED);
   //// UARTSend((uint8_t *)"Idn\n\r",9);
   UARTprintf(" Identify_fingerprint\n");
    Send_cmd_pkt(0, CMD_IDENTIFY);  // to check if the acquired fingerprint is stored or not
    //return(Send_rsp_pkt());
    return 0;
}

void interrupt_config(void)      // Reference :https://www.ti.com/lit/ug/spmu298d/spmu298d.pdf Pg 280
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);  //At power-up, all peripherals are disabled which must be enabled

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))  //Wait for the GPIOC module to be ready.
    {
    }

    GPIOPinTypeGPIOInput(ICPCK_PORT, ICPCK_PIN);  // Setting PC6 as input pin
    // Reference 1 :https://www.ti.com/lit/ug/spmu298d/spmu298d.pdf Pg 260
    // Reference 2 :https://github.com/jajoosiddhant/Two-Factor-Authentication-System
    GPIOPadConfigSet(ICPCK_PORT, ICPCK_PIN, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);  // Weak pull down with 8mA

    // Register the port-level interrupt handler which is written above.
    GPIOIntRegister(GPIO_PORTC_BASE, interrupt_handler);
    // Make pin 6 rising edge triggered interrupt
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_RISING_EDGE);
    // Enable the interrupt
    GPIOIntEnable(ICPCK_PORT, ICPCK_PIN);


}


 void boot_fingerprt(void)
 {
     Send_cmd_pkt(0,CMD_CMOSLED);
     Send_cmd_pkt(1,CMD_CMOSLED);
     Send_cmd_pkt(0,CMD_CMOSLED);
     Send_cmd_pkt(1,CMD_CMOSLED);
 }

 uint8_t capture_fingerprt(void)
 {
     //Send_cmd_pkt(0,CMD_CMOSLED);
     //Send_cmd_pkt(1,CMD_CMOSLED);
     //UARTSend((uint8_t *)"Capture-finger\n\r",25);
    UARTprintf(" \n Capture Fingerprint \n");
     Send_cmd_pkt(0, CMD_CAPTUREFINGER);
     //return(Send_rsp_pkt());
     return 0;
 }

// Referred https://github.com/jajoosiddhant/Two-Factor-Authentication-System basic understanding checksum calculation
uint16_t Checksum(uint8_t cmd_packet[])
{
    uint8_t x=0;
    uint16_t checksumtotal=0;

    while (x<10)         // First 10 packets required of whole 12 size
    {
        checksumtotal = checksumtotal + cmd_packet[x];
        x++;
    }
    return checksumtotal;
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
        ROM_UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
    }
}*/


////////////////////////////// FreeRTOS Functions ///////////////////////////////////////

void fingerprint_task(void *pvParameters)
{
    uint32_t temperature=0;
    uint32_t temperature1=25;
    uint8_t i =0;
    while(1)
    {
        UARTprintf("1: In fingerprint thread ");


        if (xSemaphoreTake ( xSemaphore, 4000/portTICK_PERIOD_MS)== pdTRUE)
   // if (fmatchflag == 1)
    {

        fmatchflag = 0;
        UARTprintf("Flag Reset \n");
        UARTprintf(" \n ##################### Received = ");
        UARTCharPutNonBlocking(UART4_BASE, 'y');    // to indicate fingerprint matched to the server
        UARTCharPutNonBlocking(UART4_BASE, '\n');    // to indicate fingerprint matched to the server
       for (i=0; i<10; i++)
       {
        xQueueReceive(queue_handle, &temperature, 1000);
        UARTprintf("%d  ", temperature);
        UARTCharPutNonBlocking(UART4_BASE, (uint8_t)temperature1);  // send queue data from TMP sensor to BeagleBone
        //UARTCharPutNonBlocking(UART4_BASE, '\n');
        while(UARTBusy(UART4_BASE));
       }
        UARTprintf("\n");
    }
    else
    {
        UARTprintf("Fingerprint Not Matched \n");
    }


    vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}


uint8_t FingerprintTaskInit(void)

{
   if(xTaskCreate(fingerprint_task, (const portCHAR *)"Fprt",128, NULL,
                      1, NULL) != pdTRUE)
       {
           return(1);
       }
       /*  xTaskCreate(fingerprint_task,"Fprt",128, NULL,
                          tskIDLE_PRIORITY, NULL);       */
        //
        // Success.
        //
    UARTprintf("Fingerprint Task created \n");
    return 0;
}




