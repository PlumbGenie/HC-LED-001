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
 * File:                boot_process.h   
 * Author:              C16813 
 * Comments:            TFTP downloading and transferring control to bootloader
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef BOOT_PROCESS_H
#define	BOOT_PROCESS_H

#include <xc.h> // include processor files - each processor file is guarded. 
#include <stdbool.h>




extern volatile bool dfd_complete_flag;



/*Downloading a boot image using DFD resource.
 * The function will download the boot image from the TFTP server.
 *  
 * @param return
 *      Nothing
 * 
 */
void dfdBootProcess(void);


/*Starting bootloading process.
 * The function will start bootloading process at specified time.
 *  
 * @param return
 *      Nothing
 * 
 */
void dfuBootProcess(void);


/*Transferrers control to the bootloader.
 * The function compares the checksum of the boot image downloaded from the TFTP server and the current image in the PIC flash.
 * If the checksums are different, the control is transferrerd to the bootloader.
 * 
 * @param Address
 *      Last address in the flash where data from the TFTP server is written to 
 *
 * @param return
 *      Nothing
 * 
 */
void TFTP_Bootload(void);

#endif	/* BOOT_PROCESS_H */

