/**
  DFD Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    dfd.c

  Summary:
     This is the implementation of DFD Resource.

  Description:
    This source file provides the implementation of the API for the DFD Resource.

 */

/*

?  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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


#include "dfd.h"
#include "udpv4.h"
#include "tcpip_types.h"
#include "object_heap.h"
//#include "coap_contentFormat.h"
#include "tftp.h"
#include "ip_database.h"



#define DFD_ELEMENTS 6
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

obj_index_t dfd_index_object_index;

static uint8_t readFromEthDfdData(obj_index_t objIdx, menu_items_t mid, bootDFD_t *p);

const dfd_lst dfd_items_list[5] = {
    {"tsrv", TFTP_SRV_ADD},
    {"tnam", TFTP_FILE_NAME},
    {"frev",TFTP_FIRM_REV},
    {"tbeg",TIME_BEGIN},
    {"tbsz",TFTP_BLOCK_SIZE}
};




int16_t initDfdResource(void)
{   
    bootDFD_t ae;  
    memset(&ae,0,sizeof(ae));

    dfd_index_object_index = obj_malloc(sizeof(bootDFD_t),"RSRC");    
    
    ae.tftpServerAddress = string_insertWord("192.168.0.12",ae.tftpServerAddress);
    ae.fileName = string_insertWord("/srv/tftp/dotNet.bin",ae.fileName);
    ae.fmwrRevNumber = string_insertWord("Version-1",ae.fmwrRevNumber);
    ae.tftpBlockSize = string_insertWord("512",ae.tftpBlockSize);
    ae.timeToBegin = 1; 
    ae.coapToken = string_insertWord("1",ae.coapToken);
    
    obj_setObjectAtIndex(dfd_index_object_index, (const void *)&ae);
        
    return dfd_index_object_index;
}

error_msg dfdInit(void *rdata, obj_index_t objIdx)
{
    return SUCCESS;
}

error_msg dfdGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    bootDFD_t p;   
    time_t now_time;
    static uint8_t dfdElements;
    char buf[8];  //jira: CAE_MCU8-5647
    
    dfdElements = DFD_ELEMENTS;
  
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
     /**** Use Encode to write data to the buffer ****/  
    
    time(&now_time);    
//    printf("\r\nDFD Time %lu \r\n",now_time);
    
    writeAuxData(&dfdElements,MAP);  
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    Get_ToEthernet((char *)"tsrv",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.tftpServerAddress),TEXT_STRING);
    
    Get_ToEthernet((char *)"tnam",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.fileName),TEXT_STRING);
    
    Get_ToEthernet((char *)"frev",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.fmwrRevNumber),TEXT_STRING);
    
    Get_ToEthernet((char *)"tbeg",TEXT_STRING);
    ltoa(buf,p.timeToBegin,10);  //jira: CAE_MCU8-5647
    Get_ToEthernet(buf,UNSIGNED_INTEGER32);  //jira: CAE_MCU8-5647
    
    Get_ToEthernet((char *)"tbsz",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.tftpBlockSize),TEXT_STRING);
    
    return SUCCESS;
}

error_msg dfdPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret=SUCCESS;
    char read_ptr[MAX_WORD_SIZE];  
    menu_items_t mid;
    uint8_t len =0, i;
    const dfd_lst *choice = NULL;
    choice = dfd_items_list;
    uint8_t payload_length;
    bootDFD_t p;   

    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    p.coapToken = string_insertWord(COAP_getReceivedClientToken(),p.coapToken);
    p.coapDestAddr = string_insertWord(makeIpv4AddresstoStr(UDP_GetSrcIP()),p.coapDestAddr);
    
    payload_length = PayloadLength;
    while(payload_length)
    {
        len =0;
        memset(read_ptr,0,sizeof(read_ptr));
        if(CfElements.array_map_elements > 0)
        {
            len += parseData(read_ptr,sizeof(read_ptr));
            for(i = 0, choice = NULL; i < 5; i++)
            {
                if (strncmp(read_ptr, dfd_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0)
                {
                    choice = dfd_items_list + i;
                    break;
                }
            }          
            memset(read_ptr,0,sizeof(read_ptr));
            mid = choice ? choice->id : UNKNOWN;           
            len += readFromEthDfdData(objIdx,mid,&p);   
            payload_length -= len;           
        }
    }
    obj_setObjectAtIndex(objIdx, (const void *)&p);    
    
    return ret;
}


static uint8_t readFromEthDfdData(obj_index_t objIdx, menu_items_t mid, bootDFD_t *p)
{
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;

    
    memset(read_ptr,0,sizeof(read_ptr));    
    switch(mid)
        {            
            case TFTP_SRV_ADD:
                len += parseData(read_ptr,sizeof(read_ptr));
                p->tftpServerAddress = string_insertWord(read_ptr,p->tftpServerAddress);
                CfElements.array_map_elements--;
                break;
            case TFTP_FILE_NAME:
                len += parseData(read_ptr,sizeof(read_ptr));
                p->fileName = string_insertWord(read_ptr,p->fileName);
                CfElements.array_map_elements--;
                break;
            case TFTP_FIRM_REV:
                 len += parseData(read_ptr,sizeof(read_ptr));
                  p->fmwrRevNumber= string_insertWord(read_ptr,p->fmwrRevNumber);            
                  CfElements.array_map_elements--;
                break;
            case TIME_BEGIN:                
                len += parseData(read_ptr,sizeof(read_ptr));
                Put_ToResource(read_ptr,&p->timeToBegin,UNSIGNED_INTEGER32);                
                 CfElements.array_map_elements--;
                break;
            case TFTP_BLOCK_SIZE:
                len += parseData(read_ptr,sizeof(read_ptr));
                p->tftpBlockSize = string_insertWord(read_ptr,p->tftpBlockSize);    
                CfElements.array_map_elements--;
                break;            
        default:
            break;
    }
    
    return len;
}


error_msg dfdPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    return ERROR;
}

error_msg dfdPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    return ERROR;
}

error_msg dfdDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    bootDFD_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.coapDestAddr;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.coapToken;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.fileName;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.fmwrRevNumber;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.tftpBlockSize;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.tftpServerAddress;
    ret = string_removeWord(stringIdxToBeRemoved);

    return ret;
}



restfulMethodStructure_t dfdMethods = {dfdInit,dfdGetter,dfdPutter,dfdPatcher,dfdDeleter,dfdPoster,initDfdResource};
