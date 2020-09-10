/**
 MAC EEPROM driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    mac_eeprom.h

  Summary:
    Header file for the MAC EEPROM implementation.

  Description:
    This header file provides APIs for the MAC EEPROM driver implementation

 */

/*

�  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software
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

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MAC_EEPROM_H
#define	MAC_EEPROM_H

#include <xc.h>
#include <stdint.h>
#include "spi1.h"

#define MAC_EEPROM_SPI_READ()   SPI1_Exchange8bit(0)
#define MAC_EEPROM_SPI_WRITE(a) SPI1_Exchange8bit(a)

#define MAC_ADDR_48                 0xFA
#define MAC_ADDR_64                 0xF8

#define UUID_MEMORY_ADDR            0x4A     /*Change as required*/
#define UUID_VALID_SIZE             1
#define UUID_VALID_ADDR             0x00

/***************** 25AA02E48 Instruction Set Summary *********************/
#define MAC_EE_READ     0x03
#define MAC_EE_WRITE    0x02
#define MAC_EE_WRDI     0x04
#define MAC_EE_WREN     0x06
#define MAC_EE_RDSR     0x05
#define MAC_EE_WRSR     0x01

/* Function Prototypes */
void          macEeprInit(void);
void          macEeprSendByte(uint8_t out);
uint8_t       macEeprGetByte(void);
void          macEeprCE_High(void);
void          macEeprCE_Low(void);
void          macEeprWP_Low(void);
uint8_t       macEeprReadStatusRegister(void);
void          macEeprEWSR(void);
void          macEeprWRSR(uint8_t byte);
void          macEeprWRENABLE(void);
void          macEeprWRDI(void);
void          macEeprsendByteBlockProtection(void);
void          macEeprErase(void);
uint8_t       macEeprRead(uint8_t Dst);
void          macEeprReadBlock(uint8_t Dst, void *data, uint8_t no_bytes);
void          macEeprWrite(uint8_t Dst, uint8_t byte);
void          macEeprWriteBlock(uint8_t address, void *data, uint8_t len);
void          macEeprWaitBusy(void);
void          macEeprWREN_Check(void);
void          macEeprverify(uint8_t byte, uint8_t cor_byte);
void          e48_read_mac(uint8_t *mac);

#endif	/* MAC_EEPROM_H */

