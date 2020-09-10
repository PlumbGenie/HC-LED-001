/**
  MEMORY Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    memory.c

  @Summary
    This is the generated driver implementation file for the MEMORY driver using MPLAB® Code Configurator

  @Description
    This file provides implementations of driver APIs for MEMORY.
    
 */

/*
Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 */

/**
  Section: Included Files
 */

#include <xc.h>
#include <stdio.h>
#include "memory.h"

/**
  Section: Flash Module APIs
 */

uint8_t MEMORY_ReadByte(uint32_t flashAddr) {
    TBLPTRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (flashAddr & 0x000000FF);

    asm("TBLRD");

    return (TABLAT);
}

uint16_t MEMORY_ReadWord(uint32_t flashAddr) {
    return ((((uint16_t) MEMORY_ReadByte(flashAddr + 1)) << 8) | (MEMORY_ReadByte(flashAddr)));
}

void MEMORY_WriteByte(uint32_t flashAddr, uint8_t *flashRdBufPtr, uint8_t byte) {
    uint32_t blockStartAddr = (uint32_t) (flashAddr & ((END_FLASH - 1) ^ (ERASE_MEMORY_BLOCKSIZE - 1)));
//    uint32_t offset = (uint32_t) (flashAddr & (uint32_t)(ERASE_MEMORY_BLOCKSIZE - 1));
    volatile uint32_t offset = 0; //(uint32_t) (flashAddr & (uint32_t)(ERASE_MEMORY_BLOCKSIZE - 1));
    uint16_t i;

    offset = (ERASE_MEMORY_BLOCKSIZE - 1);
    offset = offset & flashAddr;
    // Entire row will be erased, read and save the existing data
    for (i = 0; i < ERASE_MEMORY_BLOCKSIZE; i++) {
        flashRdBufPtr[i] = MEMORY_ReadByte((blockStartAddr + i));
    }
    NOP();
    // Load byte at offset
    flashRdBufPtr[offset] = byte;

    // Writes buffer contents to current block
    MEMORY_WriteBlock(blockStartAddr, flashRdBufPtr);
}

int8_t MEMORY_WriteBlock(uint32_t writeAddr, uint8_t *flashWrBufPtr) {
    uint32_t blockStartAddr = (uint32_t) (writeAddr & ((END_FLASH - 1) ^ (ERASE_MEMORY_BLOCKSIZE - 1)));
    uint8_t GIEBitValue = INTCONbits.GIE; // Save interrupt enable
    uint8_t i;

//    printf("block Start Addr = 0x%X ", blockStartAddr);
//    printf("write Addr = 0x%X \r\n", writeAddr);
    // Flash write must start at the beginning of a row
//    if (writeAddr != blockStartAddr) {
//        return -1;
//    }

    // Block erase sequence
    if (writeAddr == blockStartAddr)
    {    
    MEMORY_EraseBlock(writeAddr);
    }

//    if (writeAddr < blockStartAddr +1024)
//    {
    // Block write sequence
    TBLPTRU = (uint8_t) ((writeAddr & 0x00FF0000) >> 16); // Load Table point register
    TBLPTRH = (uint8_t) ((writeAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (writeAddr & 0x000000FF);

    // Write block of data
    for (i = 0; i < WRITE_MEMORY_BLOCKSIZE; i++) {
        TABLAT = flashWrBufPtr[i]; // Load data byte

        if (i == (WRITE_MEMORY_BLOCKSIZE - 1)) {
            asm("TBLWT");
        } else {
            asm("TBLWTPOSTINC");
        }
    }

//    EECON1bits.EEPGD = 1;   // CPT EDIT: For EEPROM memory select.. You do not have this
//    EECON1bits.CFGS = 0;     // CPT EDIT: For Configuration Memory select.. You do not have this
    EECON1bits.WREN = 1;
    INTCONbits.GIE = 0; // Disable interrupts
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1; // Start program

    EECON1bits.WREN = 0; // Disable writes to memory
    INTCONbits.GIE = GIEBitValue; // Restore interrupt enable
//    }
    return 0;
}

void MEMORY_EraseBlock(uint32_t baseAddr) {
    uint8_t GIEBitValue = INTCONbits.GIE; // Save interrupt enable

    TBLPTRU = (uint8_t) ((baseAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t) ((baseAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (baseAddr & 0x000000FF);

//    EECON1bits.EEPGD = 1;   // CPT EDIT: For EEPROM memory select.. You do not have this
//    EECON1bits.CFGS = 0;    // CPT EDIT: For Configuration Memory select.. You do not have this
    EECON1bits.WREN = 1;
    EECON1bits.FREE = 1;
    INTCONbits.GIE = 0; // Disable interrupts
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    INTCONbits.GIE = GIEBitValue; // Restore interrupt enable
}

uint32_t MEMORY_Checksum(uint32_t startAddr,uint32_t endAddr)         
{
    uint8_t flashByte = 0,i = 0;
    uint32_t flashChecksum = 0;
    
    while(startAddr <= endAddr)
    {
        flashByte = MEMORY_ReadByte(startAddr);
        i++;
//        printf("flash Address = 0x%X\r\n", startAddr);
//        printf("flash byte number = %d  flash Byte = 0x%X\r\n", i, flashByte);
        flashChecksum += flashByte;
        startAddr += sizeof(flashByte);
    }    
    return flashChecksum;
}
