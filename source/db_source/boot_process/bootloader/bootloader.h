/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:    bootloader.h
 * Author:  C16813
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef BOOTLOADER_H
#define	BOOTLOADER_H

#include "stdint.h"

/*Full reset with register initialization.
 * The function will reset the entire system with register initialization after  
 * bootloading. The Ethernet module does not correctly start after bootloading.
 * Therefore, the entire system has to be reset once the bootloading is complete 
 * and control is transferred to the main application.
 * 
 * This is a workaround for the DHCP issue occurring after bootloading
 * 
 * @param return
 *      Nothing
 * 
 */
void resetSystemAfterBootloading(void);

/************************************************************************/
/* PROCEDURE: copy_EEPROM_to_flash										*/
/*																		*/
/* This procedure copies data from external ERPROM to PIC18F97J60 flash */
/* over the specified address range.                                    */
/*																		*/
/* Input:																*/
/*		eeprom_addr:	EEPROM Start Address							*/
/*      flash_addr :    PIC18F97J60 Flash Start Address                 */
/*																		*/
/* Returns:																*/
/*		Nothing                                                         */
/************************************************************************/
void     copyEEPROMToMCUFlash(uint32_t eeprom_addr, uint32_t flash_addr);


/************************************************************************/
/* PROCEDURE: JumpToApplication                                         */
/*																		*/
/* This procedure transfers control to  application after bootloading.  */
/*                                                                      */
/* Returns:																*/
/*		Nothing                                                         */
/************************************************************************/
void     JumpToApplication(void);
#endif  /* BOOTLOADER_H */