/**
  STORAGE API Header File
	
  Company:
    Microchip Technology Inc.

  File Name:
    eeprom.h

  Summary:
    Header file for the STORAGE driver.

  Description:
    This header file provides APIs for the STORAGE driver.

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

#ifndef EEPROM_H
#define	EEPROM_H



#include <xc.h>
#include <stdint.h>
#include <stdlib.h>


//typedef int storagePtr;
typedef int32_t storagePtr;
//#define VERSIONID                       131070
//#define STORAGE_FREE_SIZE                131070     // None of the blocks are write protected
//#define SPACE_WRITE_READY               0xFFFFFFFF   //all 1s mean the memory location is empty for keywords
//#define STORAGE_POST_ERASE_VAL           0xFF //an erased cell will have all 1s
//#define STORAGE_ERASE_SIZE               131071 // Memory available for the datamodel, rest is occupied by the bootloader

#define VERSIONID                       129022
#define STORAGE_FREE_SIZE               129022     // None of the blocks are write protected
#define SPACE_WRITE_READY               0xFFFFFFFF   //all 1s mean the memory location is empty for keywords
#define STORAGE_POST_ERASE_VAL          0xFF //an erased cell will have all 1s
#define STORAGE_ERASE_SIZE              129023 // Memory available for the datamodel, rest is occupied by the bootloader

/* DTLS Handshake message Storage*/
#define DTLS_START_ADDRESS              129024
#define DTLS_END_ADDRESS                131071
#define DTLS_ERASE_SIZE                 (DTLS_END_ADDRESS - DTLS_START_ADDRESS)

    
/* Function Prototypes */
void          storageInit(void);
void          storagesendByte(uint8_t out);
uint8_t       storagegetByte(void);
void          storagesendByteBlockProtection(void);
void          storageDatabaseErase(void);
void          storageBootImageErase();
uint8_t       storageRead(uint32_t Dst);
void          storageReadBlock(uint32_t Dst, void *data, uint16_t no_bytes);
void          storageWrite(uint32_t Dst, uint8_t byte);
void          storageWriteBlock(uint32_t address, void *data, uint16_t len);
void          storageverify(uint8_t byte, uint8_t cor_byte);
uint16_t      storageChecksum (uint32_t Dst_Start,uint32_t Dst_End);
void          storageDtlsHandshakeErase();
void          storagewaitBusy(void);

#endif	/* EEPROM_H */

