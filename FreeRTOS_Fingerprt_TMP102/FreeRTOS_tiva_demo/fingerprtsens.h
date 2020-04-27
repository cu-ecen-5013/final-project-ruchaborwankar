/* Author : Atharv Desai
 * Fingerprint sensor header file for GT-521F32 sensor
 * References: 1) https://github.com/sparkfun/Fingerprint_Scanner-TTL/blob/master/src/FPS_GT511C3.h
 *             2) GT-521F52_Programming_guide
 *             3) GT-521FX2_datasheet_V1.1__003_.pdf
 */


#ifndef FINGERPRTSENS_H_
#define FINGERPRTSENS_H_
#include "main.h"

#include <stdint.h>
#include <stdbool.h>
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
#include "utils/uartstdio.h"
//#include "uart_echo.h"


/************************** Command Packet structure and Size ********************************/

#define CMD_START_1             (0x55)  // Static byte - beginning of a command packet
#define CMD_START_2             (0xAA)  // Static byte - beginning of a command packet
#define COMMAND_DEVICE_ID_1     (0x01)   // ID Byte 1 (L byte)
#define COMMAND_DEVICE_ID_2     (0x00)   // ID Byte 2 (H byte)

#define CMDPKT_SIZE         (12)            // Command Packet has size of 12 bytes

/************************************************************************************/



/************************** Command Packet Command Options **********************************/

#define CMD_OPEN             (0x01)
#define CMD_CLOSE            (0x02)
#define CMD_ACK              (0x30)
#define CMD_NACK             (0x31)
#define CMD_CMOSLED          (0x12)
#define CMD_VERIFY           (0x50)
#define CMD_IDENTIFY         (0X51)
#define CMD_ISPRESSFINGER    (0x26)
#define CMD_VERIFYTEMPLATE   (0x52)
#define CMD_IDENTIFYTEMPLATE (0x53)
#define CMD_CAPTUREFINGER    (0x60)

/*******************************************************************************************/


/*************************** Parameter values in Command packet for different conditions **********************/

#define PARAM_PRESSFINGER    (0)         // GT-521F52_Programming_guide_V10_20161001.pdf Page 21 - finger pressed parameter value
//#define PARAM_LEDTURNON      (1)         // GT-521F52_Programming_guide_V10_20161001.pdf Page 15 - Cmos led on parameter value
//#define PARAM_LEDTURNOFF     (0)         // GT-521F52_Programming_guide_V10_20161001.pdf Page 15 - Cmos led off parameter value
#define PARAM_NOPARAMETER    (0)

/*************************************************************************************************************/

/**************************** ICPCK pin as an Interrupt *******************************************************/
// ICPCK pin works as interrupt with 4 statuses as described in  GT-521FX2_datasheet_V1.1__003_.pdf Page  10

#define ICPCK_PORT            (GPIO_PORTC_BASE)
#define ICPCK_PIN             (GPIO_PIN_6)



/************************************************* Error Codes ***********************************************/

// For NACK response packet, Parameter is error code. Referred from GT-521F52_Programming_guide_V10_20161001.pdf Page 10

#define NACK_VERIFY_FAILED   (0x1007)
#define NACK_IDENTIFY_FAILED (0x1008)
#define NACK_BAD_FINGER      (0x100D)
#define NACK_FINGER_NOT_PRESSED (0x1012)

/************************************************************************************************************/


/******************************************** Function Declarations *****************************************/

////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : Send_cmd_pkt                                                          //
//  Description   : Sends the Command Packet of size 12 to GT-521F32 through UART         //
//  Parameters    : parameter and command are used to send in the command packet          //
//  Return Type   : void                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////

void Send_cmd_pkt (uint8_t parameter, uint8_t code);

////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : Send_rsp_pkt                                                          //
//  Description   : Sends the Response Packet of size 12 from GT-521F32 through UART      //
//  Parameters    : parameter  used to send in the response packet                        //
//  Return Type   : uint8_t                                                               //
////////////////////////////////////////////////////////////////////////////////////////////


uint8_t Send_rsp_pkt(void);

////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : Identify_fingerprt                                                    //
//  Description   : Identifies the fingerprint whether its store in GT-521F32 dtatbase    //
//  Parameters    : void                                                                  //
//  Return Type   : uint8_t                                                               //
////////////////////////////////////////////////////////////////////////////////////////////


uint8_t Identify_fingerprt(void);


////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : interrupt_config                                                      //
//  Description   : Sets up the interrupt for ICPCK pin on GPIO pin PC6 of TIVA           //
//  Parameters    : void                                                                  //
//  Return Type   : void                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////


void interrupt_config(void);

////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : boot_fingerprt                                                        //
//  Description   : Sets up the CMOS LEd sequence to indicate led booting up              //
//  Parameters    : void                                                                  //
//  Return Type   : void                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////


void boot_fingerprt(void);

////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : capture_fingerprt                                                     //
//  Description   : captures the fingerprint image with low resolution                    //
//  Parameters    : void                                                                  //
//  Return Type   : uint8_t                                                               //
////////////////////////////////////////////////////////////////////////////////////////////


uint8_t capture_fingerprt(void);

////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : Checksum                                                              //
//  Description   : Return the 16 bit checksum calculated on receiving cmd_packet         //
//  Parameters    : 8 bit packet each time from cmd_packet                                //
//  Return Type   : uint16_t                                                              //
////////////////////////////////////////////////////////////////////////////////////////////

uint16_t Checksum(uint8_t cmd_packet[]);

//void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count);



////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : fingerprint_task                                                      //
//  Description   : Executes a freertos fingerprint task                                  //
//  Parameters    : void                                                                  //
//  Return Type   : uint8_t                                                               //
////////////////////////////////////////////////////////////////////////////////////////////

void fingerprint_task(void *pvParameters);


////////////////////////////////////////////////////////////////////////////////////////////
//  Function Name : FingerprintTaskInit                                                   //
//  Description   : creates a freertos fingerprint task                                   //
//  Parameters    : void                                                                  //
//  Return Type   : uint8_t                                                               //
////////////////////////////////////////////////////////////////////////////////////////////

uint8_t FingerprintTaskInit(void);



#endif
