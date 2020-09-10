/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    context.h

  Summary:
    This is the header file for the boot_storage.c

  Description:
    This header file provides the APIs for using SST26VF064B flash memory for bootloading the device.

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
 * File:        boot_storage.h
 * Author:      C16813
 *  
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef BOOT_STORAGE_H
#define	BOOT_STORAGE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include "eeprom.h"



// New Board
#define MCU_BOOTLOADER_START            0x00000000
#define MCU_APPLICATION_START           0x00001C00
#define MCU_APPLICATION_END             0x0001FFC0
#define MCU_CHECKSUM_ADDRESS            0x0001FFF6 
#define STORAGE_BOOTLOADER_START        0x00020000
#define STORAGE_APPLICATION_START       0x00021C00
#define STORAGE_APPLICATION_END         0x0003FFC0
#define ALLOW_BOOTLOADING_PROCESS       0x0003FFF5
#define MAX_STORAGE_ADDRESS             0x0003FFFF


#define RESET_AFTER_BOOTLOADING         0x0003FFF0


#define ALLOW_BOOTLOADING_PROCESS_FLAG  0xAA
#define RESET_AFTER_BOOTLOADING_FLAG    0x55

/*Writing to one address of the flash during TFTP file download process.
 * This function writes to one address of the device during TFTP download process.
 * 
 * @param address
 *      Flash destination Address 000000H - 3FFFFH
 *
 * @param byte
 *      Name of the file to be read from the TFTP server
 *
 * @param tftp_block_size
 *      Byte to be written		
 *
 * @param return
 *      1
 * 
 */
uint8_t  storage_WriteByte_TFTP(uint32_t address, uint8_t byte);

/*Writing to a block of the flash during TFTP file download process.
 * This function writes block of specified size to the flash during TFTP download process.
 * 
 * @param address
 *      Flash destination Address 000000H - 3FFFFH
 * 
 * @param data
 *      Pointer to the block of data
 *
 * @param length
 *      Number of bytes to be written
 *
 * @param return
 *      1
 *
 */
uint8_t  storage_WriteBlock_TFTP(uint32_t address, char *data, uint16_t length);

/*Reading a byte from flash.
 * This procedure programs reads a byte from flash during TFTP bootloading process.
 * 
 * @param address
 *      Flash destination Address 000000H - 3FFFFH
 * 
 * @param return
 *      1
 * 
 */
uint8_t  storage_ReadByte_TFTP(uint32_t address);

/*Reading a block of data from flash.
 * This function reads block of specified size from the flash.
 * 
 * @param address
 *      Flash destination Address 000000H - 3FFFFH
 * 
 * @param data
 *      Pointer to destination where the read data is to be stored
 *
 * @param length
 *      Number of bytes to be read
 *
 * @param return
 *      1
 *
 */
uint8_t  storage_ReadBlock_TFTP(uint32_t address, char *data, uint16_t length);

/*Checksum calculation over specified range of addresses.
 * The function calculates checksum over an address range.
 * 
 * @param start_address
 *      Start address for checksum calculation
 *
 * @param end_address
 *      End address for checksum calculation
 *
 * @param tftp_block_size
 *      Size of each packet in bytes sent by the TFTP server
 *
 * @param return
 *      Checksum
 * 
 */
uint16_t storage_Checksum_TFTP(uint32_t start_address, uint32_t end_address);


#endif	/* BOOT_STORAGE_H */

