/**
 Using SST26VF064B for bootloading
	
  Company:
    Microchip Technology Inc.

  File Name:
    boot_storage.c

  Summary:
     This is used for implementation of a TFTP bootloader.

  Description:
    This source file provides the implementation of the APIs used during bootloading the device.

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

/**
 Section: Included Files
 */
#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
//#include "flash.h"
#include "eeprom.h"
#include "boot_storage.h"

bool storageerase = true;

static uint8_t storage_Erase_TFTP(void);


static uint8_t storage_Erase_TFTP(void)
{
    storageBootImageErase();	//Erase the chip
    printf("Previous Firmware Image Erased \r\n");
    return 1;
}

uint8_t storage_WriteByte_TFTP(uint32_t address, uint8_t byte)
{
    storageWrite(address, byte);
    storagewaitBusy();
    NOP();NOP();NOP();
    return 1;
}


uint8_t storage_WriteBlock_TFTP(uint32_t address, char *data, uint16_t length)
{
    if (storageerase == true)
    {
        storage_Erase_TFTP();
        storageerase = false;
    }
    storageWriteBlock(address, data, length);
    storagewaitBusy();
    NOP();NOP();NOP();  
    return 1;
}


uint8_t storage_ReadByte_TFTP(uint32_t address)
{    
    return storageRead(address);
}


uint8_t storage_ReadBlock_TFTP(uint32_t address, char *data, uint16_t length)
{    
    storageReadBlock(address, data, length); 
    return 1;
}


uint16_t storage_Checksum_TFTP(uint32_t start_address, uint32_t end_address)
{   
    uint16_t storage_Checksum = 0;
    storage_Checksum = storageChecksum(start_address, end_address);
    return storage_Checksum;
}
