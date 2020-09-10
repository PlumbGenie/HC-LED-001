/**
 Using DFD and DFU resources for bootloading
	
  Company:
    Microchip Technology Inc.

  File Name:
    boot_process.c

  Summary:
    This is used for using the DFD and DFU resources for bootloading.

  Description:
    This source file provides the implementation of the APIs using the DFD and DFU resources.

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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware.h"
#include "memory.h"
#include "dfd.h"
#include "dfu.h"
#include "coap.h"
#include "coap_options.h"
#include "tcpip_types.h"
#include "udpv4.h"
#include "object_heap.h"
#include "data_model.h"
#include "graph.h"
#include "cbor.h"
#include "coap_options.h"
#include "contentFormat.h"
#include "act_sen.h"
#include "coap_resources.h"
#include "tftp.h"
#include "ip_database.h"
#include "inventory.h"
#include "string_heap.h"
#include "boot_storage.h"
#include "log.h"


#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_TFTP, msgSeverity, msgLogDest) 
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif




volatile bool dfd_complete_flag = false;


void TFTP_Bootload(void)
{
    uint8_t storageWriteComplete = 0;
    uint8_t syslogBuffer[22];
    uint16_t cksm = 0, mcuFlashChecksum = 0;
    bootDFU_t p; //jira: M8TS-688
    vertex_index_t dfuVertexIdx; //jira: M8TS-688
    vertex_t v; //jira: M8TS-688
    
    memset(syslogBuffer, 0, sizeof(syslogBuffer));
    
    mcuFlashChecksum = MEMORY_ReadWord(MCU_CHECKSUM_ADDRESS);
    sprintf(syslogBuffer, "PIC18 Checksum=0x%.2X",  mcuFlashChecksum);
    logMsg(syslogBuffer, LOG_ALERTS, LOG_DEST_ETHERNET);
    printf("PIC18 Flash Checksum = 0x%.4X \r\n", mcuFlashChecksum);
    
    memset(syslogBuffer, 0, sizeof(syslogBuffer));
    
    cksm = storage_Checksum_TFTP(STORAGE_APPLICATION_START, STORAGE_APPLICATION_END);
    sprintf(syslogBuffer, "EEPROM Checksum=0x%.2X",  cksm);
    logMsg(syslogBuffer, LOG_ALERTS, LOG_DEST_ETHERNET);
    printf("EEPROM Storage Checksum = 0x%.4X \r\n", cksm);   //jira: CAE_MCU8-5647
    
    memset(syslogBuffer, 0, sizeof(syslogBuffer));
    
    storage_WriteByte_TFTP(ALLOW_BOOTLOADING_PROCESS, ALLOW_BOOTLOADING_PROCESS_FLAG);  
    storageWriteComplete = storage_ReadByte_TFTP(ALLOW_BOOTLOADING_PROCESS);
    sprintf(syslogBuffer, "writeFlag=0x%.2X",  storageWriteComplete);
//    printf("storageWriteComplete = 0x%.2X \r\n", storageRead(ALLOW_BOOTLOADING_PROCESS));
    logMsg(syslogBuffer, LOG_ALERTS, LOG_DEST_ETHERNET);
    
    memset(syslogBuffer, 0, sizeof(syslogBuffer));
    
    storage_WriteByte_TFTP(RESET_AFTER_BOOTLOADING, RESET_AFTER_BOOTLOADING_FLAG); 
//    printf("Bootloader flag = 0x%.2X \r\n", storageRead(RESET_AFTER_BOOTLOADING));
    sprintf(syslogBuffer, "bootFlag=0x%.2X",  storageRead(RESET_AFTER_BOOTLOADING));
    logMsg(syslogBuffer, LOG_ALERTS, LOG_DEST_ETHERNET);
    
    if((storageWriteComplete == ALLOW_BOOTLOADING_PROCESS_FLAG) && (mcuFlashChecksum != cksm))
    {
//        printf("Turning off the Ethernet module \r\n");
        logMsg("Turning off the Ethernet module", LOG_ALERTS, LOG_DEST_ETHERNET);
//        printf("Transferring control to bootloader \r\n");
        logMsg("Transferring control to bootloader", LOG_ALERTS, LOG_DEST_ETHERNET);
        ECON2bits.ETHEN = 0;
//        printf("Ethernet module turned off \r\n");    
        
        POE_STAY_ALIVE_LAT = 1;
        
        dfuVertexIdx = graph_get_vertex_idx_byName(DFU); //jira: M8TS-688
        graph_getVertexAtIndex(&v,dfuVertexIdx); //jira: M8TS-688
        
        p.updateTime = 0; //jira: M8TS-688
        obj_setObjectAtIndex(v.objIdx, (const void *)&p); //jira: M8TS-688
        
        __delay_ms(10); __delay_ms(10); __delay_ms(10);
        asm("GOTO " ___mkstr(MCU_BOOTLOADER_START));
    }
    else
    {
        storage_WriteByte_TFTP(ALLOW_BOOTLOADING_PROCESS, 0xFF);
        asm("GOTO " ___mkstr(MCU_APPLICATION_START));
    }
}


void dfdBootProcess()
{
    uint32_t tftpAddress;
    obj_index_t obj_idx, obj_idx_fw; 
    bootDFD_t p; 
    inventory_eeprom_t q;
    time_t now_time;    
    error_msg ret = ERROR;
    char tftp_block_size[5];
    char new_frmw_version[MAX_WORD_SIZE];
    char curr_frmw_version[MAX_WORD_SIZE];
    coapTransmit_t ptr ;
    uint8_t coapTokenLength;
    int16_t dfdVertexIdx;
    vertex_t v;
    
    now_time = 0;
    
    dfdVertexIdx= graph_get_vertex_idx_byName(DFD);
    graph_getVertexAtIndex(&v, dfdVertexIdx);
    obj_idx = v.objIdx;
    obj_getObjectAtIndex(v.objIdx, (const void *)&p);
    
    for(uint16_t i; i < MAXVERTEXCOUNT; i++ ) // currently it only checks 1 and assumes all have the same fw_rev
    {
        obj_idx_fw = graph_get_vertex_idx_verified_byType(INVENTORY_TYPE, i);
        if(obj_idx_fw != -1)break;
    }
    obj_getObjectAtIndex(obj_idx_fw, (const void *)&q); 
    memset(&ptr,0,sizeof(ptr));
    time(&now_time);
    
//    printf("%ld \r\n", p.timeToBegin);
    
    if(p.timeToBegin == now_time)
    {   
        logMsg("Downloading Bootloader Image", LOG_ALERTS, (LOG_DEST_CONSOLE | LOG_DEST_ETHERNET));
        strcpy(new_frmw_version,string_getWordAtIndex(p.fmwrRevNumber));
        strcpy(curr_frmw_version,string_getWordAtIndex(q.firmwareRevision));
        
        if(strncmp(new_frmw_version, curr_frmw_version, MAX_WORD_SIZE) != 0)
        {
            tftpAddress = makeStrToIpv4Address(string_getWordAtIndex(p.tftpServerAddress));     
            strcpy(tftp_block_size, string_getWordAtIndex(p.tftpBlockSize));        
            TFTP_Configure(tftpAddress, string_getWordAtIndex(p.fileName),tftp_block_size); 
//            printf("Sending TFTP Read Request \r\n");   
            logMsg("Sending TFTP Read Request", LOG_ALERTS, (LOG_DEST_CONSOLE | LOG_DEST_ETHERNET));
            ret = TFTP_Read_Write_Request(tftp_read);  
            if(ret == SUCCESS)
            {
                p.timeToBegin = 0;
                obj_setObjectAtIndex(obj_idx, (const void *)&p);  
            }                
        }
    }
    if(last_block == true)
    {
        last_block = false;
        dfd_complete_flag = true;    
        coapTokenLength = strlen(string_getWordAtIndex(p.coapToken));
        memcpy(ptr.clientToken, string_getWordAtIndex(p.coapToken), coapTokenLength);
        ptr.destIP = makeStrToIpv4Address(string_getWordAtIndex(p.coapDestAddr)); 
        COAP_Transmit(&ptr, NON_TYPE, PUT, false);                
        COAP_AddURIPathOption(ROOT_DIRECTORY);
        COAP_AddURIPathOption("dfd");
        COAP_AddContentFormatOption(CBOR);
        UDP_Write8(0xFF);
        CBOR_Encode_Text_String("DFDSuccess");
        CBOR_Encode_Unsigned_Int(dfd_complete_flag);
        UDP_Send(); 
//        printf("Bootloader Image download complete \r\n");    
        logMsg("Bootloader Image download complete", LOG_ALERTS, (LOG_DEST_CONSOLE | LOG_DEST_ETHERNET));
    }
}



void dfuBootProcess(void)
{
    bootDFU_t p; 
    time_t now_time; 
    vertex_index_t dfuVertexIdx;
    vertex_t v;
    
    now_time = 0;
    
    dfuVertexIdx = graph_get_vertex_idx_byName(DFU);
    graph_getVertexAtIndex(&v,dfuVertexIdx);
//    printf("Boot Process DFU Obj Index = %d \r\n", obj_idx);
    obj_getObjectAtIndex(v.objIdx, (const void *)&p);
    
    time(&now_time);
    
//    printf("now time = %lu time to begin %lu \r\n",now_time,  p.updateTime);
    if(p.updateTime == (uint32_t)now_time)    //jira: CAE_MCU8-5647
    {
        if(dfd_complete_flag == true)
        {
            printf("Flashing \r\n");
            logMsg("Flashing Bootloader Image", LOG_ALERTS, LOG_DEST_ETHERNET);
            TFTP_Bootload();
        }
    }    
}


