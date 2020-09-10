/*
 * File:   bootloader.c
 * Author: C16813
 *
 * Created on November 12, 2015, 11:24 AM
 */


#include <xc.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "eeprom.h"
#include "memory.h"
#include "bootloader.h"
#include "boot_storage.h"

void resetSystemAfterBootloading(void)
{
    uint8_t testFlag = 0;

    testFlag = storage_ReadByte_TFTP(RESET_AFTER_BOOTLOADING);
    printf("Test Flag = 0x%.2X \r\n", testFlag);
    
    // This is a workaround for the DHCP issue occurring after bootloading
    if(storage_ReadByte_TFTP(RESET_AFTER_BOOTLOADING) == RESET_AFTER_BOOTLOADING_FLAG)
    {
//        logMessage("Sending a System RESET", LOG_TFTP, LOG_ALERTS, LOG_DEST_ETHERNET);
        printf("Sending a System RESET  \r\n");
        
        // Clear the TEST_BOOTLOADER_FLAG first so as not to enter in this loop again
        storageWrite(RESET_AFTER_BOOTLOADING, 0xFF);
        
        // This indicates that the control was transferred here for the first time after bootloading
        RESET(); 
//        ETH_SendSystemReset();
    }
}


void copyEEPROMToMCUFlash(uint32_t eeprom_addr, uint32_t flash_addr)
{
    uint8_t transfer_buff[64];  
    
    while (flash_addr <= MCU_APPLICATION_END)
    {
        memset(transfer_buff, 0, sizeof(transfer_buff));
        storage_ReadBlock_TFTP(eeprom_addr, transfer_buff, sizeof(transfer_buff));
        MEMORY_WriteBlock((uint32_t)flash_addr, (uint8_t *)transfer_buff);
        eeprom_addr += sizeof(transfer_buff);
        flash_addr  += sizeof(transfer_buff);       
    } 
    
    printf("FINAL EEPROM ADDRESS = %lu  \r\n", eeprom_addr);
    printf("FINAL MCU FLASH ADDRESS = %lu  \r\n", flash_addr);
}


void JumpToApplication(void)
{
    #ifdef __XC8
    asm("GOTO " ___mkstr(MCU_APPLICATION_START));
    #endif
}