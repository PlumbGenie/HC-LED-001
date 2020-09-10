/**
  Location Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    location.c

  Summary:
     This is the implementation of Location Resource.

  Description:
    This source file provides the implementation of the API for the Location Resource.

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

#include "location.h"
#include "tcpip_types.h"
#include "object_heap.h"
//#include "coap_contentFormat.h"

#define LOCATION_EEP_ELEMENTS 2

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

obj_index_t location_index_object_index;
static error_msg writeToEthLocationData(location_eeprom_t *p);




obj_index_t initLocationResource(void)
{
    location_eeprom_t ae;    
   memset(&ae,0,sizeof(ae));
    location_index_object_index = obj_malloc(sizeof(location_eeprom_t),"RSRC");

    ae.payload = string_insertWord("Building1/Hallway-102",ae.payload);   
    
    obj_setObjectAtIndex(location_index_object_index, (const void *)&ae);   
     
    return location_index_object_index;
}


error_msg deleteLocationResource(obj_index_t objIdx)
{  
    obj_free(objIdx); 

    return SUCCESS;
}

error_msg locationInit( void *ptr, obj_index_t objIdx)  //jira: CAE_MCU8-5647
{    
    return SUCCESS;   
}

error_msg locationGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
    location_eeprom_t p;
    static uint8_t locationElements;
    
    locationElements = LOCATION_EEP_ELEMENTS;
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    /**** Use Encode to write data to the buffer ****/
    
    writeAuxData(&locationElements,MAP);      
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    ret = writeToEthLocationData(&p);   
    
    return ret;

}

static error_msg writeToEthLocationData(location_eeprom_t *p)
{    
    Get_ToEthernet((char *)"payl",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->payload),TEXT_STRING);
   
    
    return SUCCESS;
}

error_msg locationPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg locationPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg locationPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg locationDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    location_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.payload;
    ret = string_removeWord(stringIdxToBeRemoved);

    return ret;
}



restfulMethodStructure_t locationMethods = {locationInit,locationGetter,locationPutter,locationPatcher,locationDeleter,locationPoster,initLocationResource};

/**************************************Query Handlers*******************************************/

bool queryPaylHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    location_eeprom_t p;    
     
     if(payl_query==0)
     {
        obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.payload == query_value)  //jira: CAE_MCU8-5647
        {
            payl_query = 1;
            ret = true;
        }
        else
        {
            ret = false;
        }
     }
     else     
         ret = true;        
     
     return ret;   
}
