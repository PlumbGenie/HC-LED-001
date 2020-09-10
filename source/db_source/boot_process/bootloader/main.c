/**
  Main file for implementing the Demo functionality
	
  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

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

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include "hardware.h"
#include "eeprom.h"
#include "memory.h"
#include "bootloader.h"
#include "boot_storage.h"
#include "configuration.h"

asm("PSECT intcode");
asm("GOTO " ___mkstr(MCU_APPLICATION_START+0x8));
asm("PSECT intcodelo");
asm("GOTO " ___mkstr(MCU_APPLICATION_START+0x18));

//#asm
//PSECT intcode
//    GOTO    PROG_START+0x8
//PSECT intcodelo 
//    GOTO    PROG_START+0x18
//#endasm

/**************************************************************************************************/

void main(void)
{
    uint8_t eepromRead;
    uint32_t eepr_start_address = STORAGE_APPLICATION_START, flash_start_address = MCU_APPLICATION_START; // Address where application starts 
    
    SYSTEM_Initialize();     
        
    eepromRead = storage_ReadByte_TFTP(ALLOW_BOOTLOADING_PROCESS);   
    
    if(eepromRead == ALLOW_BOOTLOADING_PROCESS_FLAG)
    {
        printf("Flashing Bootloader Image");
        // disable the high priority interrupts
        IPEN = 0;
        GIEH = 0;
        GIEL = 0;
        
        POE_STAY_ALIVE_LAT = 1;
        
        // Start bootloading
        copyEEPROMToMCUFlash(eepr_start_address, flash_start_address);
        
        POE_STAY_ALIVE_LAT = 1;
        
        
        eepromRead = 0xFF;
        storage_WriteByte_TFTP(ALLOW_BOOTLOADING_PROCESS, eepromRead);
        
        NOP();NOP();NOP();
        
        resetSystemAfterBootloading();
        
        POE_STAY_ALIVE_LAT = 1;
        
        NOP();NOP();NOP();
        
        JumpToApplication();
    }
    else
    {
        
        JumpToApplication();
    }
    return;
}
