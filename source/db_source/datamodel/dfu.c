/**
  DFU Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    dfu.c

  Summary:
     This is the implementation of DFU Resource.

  Description:
    This source file provides the implementation of the API for the DFU Resource.

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


#include "dfu.h"
#include "coap.h"
#include "tcpip_types.h"
#include "object_heap.h"
//#include "coap_contentFormat.h"
#include "tftp.h"
//#include "flash_storage.h"




#define DFU_ELEMENTS 2
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

obj_index_t dfu_index_object_index;

static uint8_t readFromEthDfuData(obj_index_t objIdx, menu_items_t mid, bootDFU_t *p);

const dfu_lst dfu_items_list[1] = {
    {"updt", UPDATE_TIME}   
};


int16_t initDfuResource(void)
{   
    bootDFU_t ae;  
    memset(&ae,0,sizeof(ae));

    dfu_index_object_index = obj_malloc(sizeof(bootDFU_t),"RSRC");
    
    ae.updateTime = 5456234;    
    
    obj_setObjectAtIndex(dfu_index_object_index, (const void *)&ae);
   
     
    return dfu_index_object_index ;
}


error_msg dfuInit(void *rdata, obj_index_t objIdx)
{
    return SUCCESS;
    
}

error_msg dfuGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{    
    bootDFU_t p;   
    static uint8_t dfutElements;
    char buf[8]; //jira: CAE_MCU8-5647
    
    dfutElements = DFU_ELEMENTS;
  
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    /**** Use Encode to write data to the buffer ****/  
    
    writeAuxData(&dfutElements,MAP);    
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    Get_ToEthernet((char *)"updt",TEXT_STRING);
    ltoa(buf,p.updateTime, 10);  //jira: CAE_MCU8-5647
    Get_ToEthernet(buf,UNSIGNED_INTEGER32);   //jira: CAE_MCU8-5647 
     
    return SUCCESS;
}

error_msg dfuPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret=ERROR;
    char read_ptr[MAX_WORD_SIZE];  
    menu_items_t mid;
    uint8_t len =0, i;
    const dfu_lst *choice = NULL;
    choice = dfu_items_list;
    uint8_t payload_length;
    bootDFU_t p;   


    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    payload_length = PayloadLength;
    while(payload_length)
    {
        len =0;
        memset(read_ptr,0,sizeof(read_ptr));
        if(CfElements.array_map_elements > 0)
        {
            len += parseData(read_ptr,sizeof(read_ptr));
            for(i = 0, choice = NULL; i < 1; i++)
            {
                if (strncmp(read_ptr, dfu_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0)
                {
                    choice = dfu_items_list + i;
                    break;
                }
            }          
            memset(read_ptr,0,sizeof(read_ptr));
            mid = choice ? choice->id : UNKNOWN;           
            len += readFromEthDfuData(objIdx,mid,&p);   
            payload_length -= len;            
        }
    }
    obj_setObjectAtIndex(objIdx, (const void *)&p);  

    
    return SUCCESS;  
}


static uint8_t readFromEthDfuData(obj_index_t objIdx, menu_items_t mid, bootDFU_t *p)
{
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;
    
    memset(read_ptr,0,sizeof(read_ptr));    
    switch(mid)
        {            
            case UPDATE_TIME:
                len += parseData(read_ptr,sizeof(read_ptr));
                Put_ToResource(read_ptr,&p->updateTime,UNSIGNED_INTEGER32); 
                
                 CfElements.array_map_elements--;
                break;           
        default:
            break;
    }
    
    return len;
}


error_msg dfuPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    return ERROR;
}

error_msg dfuDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    return SUCCESS;
}

error_msg dfuPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    return ERROR;
}




restfulMethodStructure_t dfuMethods = {dfuInit,dfuGetter,dfuPutter,dfuPatcher,dfuDeleter,dfuPoster,initDfuResource};
