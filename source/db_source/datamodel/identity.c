/**
  Identity Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    identity.c

  Summary:
     This is the implementation of Identity Resource.

  Description:
    This source file provides the implementation of the API for the Identity Resource.

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "identity.h"
#include "tcpip_types.h"
#include "object_heap.h"
#include "data_model.h"
//#include "coap_contentFormat.h"

#define IDENTITY_EEP_ELEMENTS 4
#define SUPPORTED_FORMATS_COUNT 1

obj_index_t identity_index_object_index;
static error_msg writeToEthIdentityData(identity_eeprom_t *p);
static uint8_t readFromEthIdentityData(obj_index_t objIdx, menu_items_t mid, identity_eeprom_t *p);

const identity_lst idt_items_list[3] = {

  { "enam", ENAM },
  { "ecla", ECLA },
  { "akey", AKEY }
};

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))



int16_t initIdentityResource(void)
{
    identity_eeprom_t ae;    
    memset(&ae,0,sizeof(ae));

 
    identity_index_object_index = obj_malloc(sizeof(identity_eeprom_t),"RSRC");

    
    ae.entPhysicalNameIdx = string_insertWord("MCU8EndPt",ae.entPhysicalNameIdx);
    ae.entPhysicalClassIdx = string_insertWord("EndPtClass",ae.entPhysicalClassIdx);
    ae.alternateKeyIdx = string_insertWord("EndpPtAltKey",ae.alternateKeyIdx);
    
    obj_setObjectAtIndex(identity_index_object_index, (const void *)&ae);   
     
    return identity_index_object_index;
}

error_msg deleteIdentityResource(obj_index_t objIdx)
{    
    obj_free(objIdx);

    return SUCCESS;
}

error_msg identityInit(void *rdata, obj_index_t objIdx)
{  
    
    return SUCCESS;    
}

error_msg identityGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
    identity_eeprom_t p;   
    static uint8_t identityElements;
    
    identityElements = IDENTITY_EEP_ELEMENTS;

    obj_getObjectAtIndex(objIdx, (const void *)&p);

    /**** Use Encode to write data to the buffer ****/
    
    writeAuxData(&identityElements,MAP);  
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    ret = writeToEthIdentityData(&p);
    
    return ret;
    
}

error_msg identityPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
     error_msg ret = SUCCESS;
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;
    uint16_t i; //jira: CAE_MCU8-5647
    menu_items_t mid;
    const identity_lst *choice = NULL;
    choice = idt_items_list;
    uint8_t payload_length;
    identity_eeprom_t p; 
    
    memset(read_ptr,0,sizeof(read_ptr));   
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    payload_length = PayloadLength;
    
    while(payload_length)
    {
        len =0;
        len += parseData(read_ptr,sizeof(read_ptr));
        for(i = 0, choice = NULL; i < 3; i++)
        {
            if (strncmp(read_ptr, idt_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0)
            {
                choice = idt_items_list + i;
                break;
            }
        }           
        memset(read_ptr,0,sizeof(read_ptr));
        mid = choice ? choice->id : UNKNOWN;      
       
        switch(mid)
        {          
            default:
                len += readFromEthIdentityData(objIdx, mid, &p);
                break;
        }        
        payload_length -= len;
    }
    obj_setObjectAtIndex(objIdx, (const void *)&p);    
    
    return ret; 

    
}

static error_msg writeToEthIdentityData(identity_eeprom_t *p)
{    
    Get_ToEthernet((char *)"enam",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->entPhysicalNameIdx),TEXT_STRING);
    
    Get_ToEthernet((char *)"ecla",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->entPhysicalClassIdx),TEXT_STRING);        
    
    Get_ToEthernet((char *)"akey",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->alternateKeyIdx),TEXT_STRING);  
    
    return SUCCESS;
     
}

static uint8_t readFromEthIdentityData(obj_index_t objIdx, menu_items_t mid, identity_eeprom_t *p)
{   
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;    
   
    memset(read_ptr,0,sizeof(read_ptr));
    switch(mid)
    {
        case ENAM:
            len += parseData(read_ptr,sizeof(read_ptr));      
            p->entPhysicalNameIdx = string_insertWord(read_ptr,p->entPhysicalNameIdx);                
            break;
        case ECLA:
            len += parseData(read_ptr,sizeof(read_ptr));            
            p->entPhysicalClassIdx = string_insertWord(read_ptr,p->entPhysicalClassIdx); 
            break;
        case AKEY:
            len += parseData(read_ptr,sizeof(read_ptr));        
            p->alternateKeyIdx = string_insertWord(read_ptr,p->alternateKeyIdx);                      
            break;             
        default:                
            break;  
    }
    return len;
}

error_msg identityPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{    
     
    return ERROR;
}

error_msg identityPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{    
    return ERROR;
}

error_msg identityDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{    
    identity_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.alternateKeyIdx;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.entPhysicalClassIdx;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.entPhysicalNameIdx;
    ret = string_removeWord(stringIdxToBeRemoved);


    return ret;
}
    



restfulMethodStructure_t identityMethods = {identityInit,identityGetter,identityPutter,identityPatcher,identityDeleter,identityPoster,initIdentityResource};

/**************************************************************Query Handlers**************************************************************************/
//bool queryEnamHandler(obj_index_t objIdx,uint32_t query_value)
//{     
//    bool ret = false;
//    identity_eeprom_t p;    
//     
//    if(enam_query==0)
//    {
//        obj_getObjectAtIndex(objIdx, (const void *)&p);
//        if(p.entPhysicalNameIdx == query_value)
//        {
//                     enam_query = 1;
//                     ret = true;
//        }
//        else
//        {
//                 ret = false;
//        }
//    }
//     else     
//         ret = true;        
//     
//     return ret;              
//}
//
//bool queryEclaHandler(obj_index_t objIdx,uint32_t query_value)
//{     
//    bool ret = false;
//    identity_eeprom_t p;    
//     
//     if(ecla_query==0)
//     {
//        obj_getObjectAtIndex(objIdx, (const void *)&p);
//        if(p.entPhysicalClassIdx == query_value)
//        {
//            ecla_query = 1;
//            ret = true;
//        }
//        else
//        {
//            ret = false;
//        }
//    }
//     else     
//         ret = true;        
//     
//     return ret;              
//}
//
//bool queryAkeyHandler(obj_index_t objIdx,uint32_t query_value)
//{     
//    bool ret = false;
//    identity_eeprom_t p;    
//     
//     if(akey_query==0)
//     {        
//         
//        obj_getObjectAtIndex(objIdx, (const void *)&p);
//        if(p.alternateKeyIdx == query_value)
//        {
//            akey_query = 1;
//            ret = true;
//        }
//        else
//        {
//            ret = false;
//        }
//
//     }
//     else     
//         ret = true;        
//     
//     return ret;              
//}

