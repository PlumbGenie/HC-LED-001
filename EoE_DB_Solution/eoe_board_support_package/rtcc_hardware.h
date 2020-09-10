/**
  RTCC API Header File
	
  Company:
    Microchip Technology Inc.

  File Name:
    rtcc.h

  Summary:
    Header file for the RTCC driver.

  Description:
    This header file provides APIs for the RTCC driver.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
and any derivatives exclusively with Microchip products. 
  
THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF 
NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE, OR ITS 
INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE 
IN ANY APPLICATION. 

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL 
OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED 
TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY 
OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S 
TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED 
THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS. 

*/


#ifndef RTCC_H
#define RTCC_H



#include <time.h>
#include <stdint.h>
#include "mssp_spi1_master.h"
#include "hardware.h"

typedef struct
{
int sec,min,hr;
int year,month,date,day;
}DateTime;


/***************** MCP79512 Instruction Set Summary *********************/

#define EEREAD     0x03
#define EEWRITE    0x02
#define EEWRDI     0x04
#define EEWREN     0x06
#define SRREAD     0x05
#define SRWRITE    0x01
#define RTCC_READ  0x13
#define RTCC_WRITE 0x12
#define UNLOCK     0x14
#define IDWRITE    0x32
#define IDREAD     0x33
#define CLRRAM     0x54


/*********************** RTCC Register Addresses ***************************/

#define RTCC_HUNDRETHS_OF_SECONDS   0x00  //millisecond
#define RTCC_SECONDS                0x01
#define RTCC_MINUTES                0x02
#define RTCC_HOUR                   0x03
#define RTCC_DAY                    0x04
#define RTCC_DATE                   0x05
#define RTCC_MONTH                  0x06
#define RTCC_YEAR                   0x07

#define CONTROL_REG                 0x08
#define CALIBRATION                 0x09

#define ALARM0_SECONDS              0x0C
#define ALARM0_MINUTES              0x0D
#define ALARM0_HOUR                 0x0E
#define ALARM0_DAY                  0x0F
#define ALARM0_DATE                 0x10
#define ALARM0_MONTH                0x11

#define ALARM1_HUNDRETHS_OF_SECONDS 0x12
#define ALARM1_SECONDS              0x13
#define ALARM1_MINUTES              0x14
#define ALARM1_HOUR                 0x15
#define ALARM1_DAY                  0x16
#define ALARM1_DATE                 0x17

#define PWR_DOWN_MINUTES            0x18
#define PWR_DOWN_HOUR               0x19
#define PWR_DOWN_DATE               0x1A
#define PWR_DOWN_MONTH              0x1B

#define PWR_UP_MINUTES              0x1C
#define PWR_UP_HOUR                 0x1D
#define PWR_UP_DATE                 0x1E
#define PWR_UP_MONTH                0x1F

#define MAC_ADDR_48                 0x02
#define MAC_ADDR_64                 0x00


/******************************************************************************/

#define  SQWE                       0x40            //  Square Wave Enable BIT
#define  ALM_NO                     0x00            //  no alarm activated    
#define  MFP_01H                    0x00            //  MFP = SQWAV(01 HERZ)      
#define  OSCON                      0x20            //  state of the oscillator(running or not)
#define  VBATEN                     0x08            //  enable battery for back-up
#define  VBAT_CLR                   0xEF            //  Mask to clear VBAT flag BIT
#define  EXTOSC                     0x08            //  enable external 32khz signal
#define  ST_SET                     0x80            //  start oscillator

#define EEPR_PAGE_SIZE               8

//#define UUID_MEMORY_ADDR             0x01     /*Change as required*/
//#define UUID_VALID_SIZE              1
//#define UUID_VALID_ADDR              0x00

/******************************************************************************/

#define RTCC_SPI_READ()   SPI1_ExchangeByte(0)
#define RTCC_SPI_WRITE(a) SPI1_ExchangeByte(a)
//
//#define CS_RTCC_HIGH() do{LATD2 = 1;} while(0)
//#define CS_RTCC_LOW()  do{LATD2 = 0;} while(0)

//#define RTCC_TICK PORTBBITs.RB1;

///****************************PoE Stay Alive Time*******************************/
//
//#define PoESA_ON_TIME   75
//#define PoESA_OFF_TIME  250
///******************************************************************************/


/**************************** MCP79512 APIs ***********************************/

void rtcc_init(void);
void rtcc_time_init(void);
void rtcc_handler(void);
void rtcc_set(time_t);
BIT  rtcc_isDirty(void);
time_t rtcc_get(void);
uint8_t rtcc_read(uint8_t addr);
void rtcc_read_block(uint8_t addr, void *data, uint8_t len);
void rtcc_write(uint8_t addr, uint8_t data);
void rtcc_write_block(uint8_t addr, void *data, uint8_t len);
time_t  time(time_t *t);
inline void maintain_PoESA(void);
extern volatile BIT dirtyTime;

uint8_t eepr_read_mcp79512(uint8_t addr);
void eepr_read_block_mcp79512(uint8_t addr, void *data, uint8_t len);
void eepr_write_mcp79512(uint8_t addr, uint8_t data);
void eepr_write_block_mcp79512(uint8_t addr, void *data, uint8_t len);


void e48_read_mac(uint8_t *mac);
void e64_read_mac(uint8_t *mac);
/******************************************************************************/

#endif /* RTCC_H */
