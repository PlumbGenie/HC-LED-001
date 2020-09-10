/**
  Inventory Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    inventory.c

  Summary:
     This is the implementation of Inventory Resource.

  Description:
    This source file provides the implementation of the API for the Inventory Resource.

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
#include "inventory.h"
#include "tcpip_types.h"
#include "object_heap.h"
//#include "coap_contentFormat.h"


#define INVENTORY_EEP_ELEMENTS 7


obj_index_t inventory_index_object_index; 

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))




static error_msg writeToEthInventoryData(inventory_eeprom_t *p);


error_msg inventoryInit(void *ptr ,obj_index_t objIdx)  //jira: CAE_MCU8-5647
{    
    return SUCCESS;   
}


int16_t initInventoryResource(void)
{      
    inventory_eeprom_t ae;
    
    memset(&ae,0,sizeof(ae));
    inventory_index_object_index = obj_malloc(sizeof(inventory_eeprom_t),"RSRC");
    
    ae.firmwareRevision = string_insertWord("v1.0.1",ae.firmwareRevision);//This is Digital Building's database version
    ae.hardwareRevision = string_insertWord("2.95F2",ae.hardwareRevision);
    ae.softwareRevision = string_insertWord("V1.25alpha",ae.softwareRevision);
    ae.serialNumber = string_insertWord("ABCDE123",ae.serialNumber);
    ae.manufacturer = string_insertWord("ConnectedLight",ae.manufacturer);
    ae.model = string_insertWord("SensorModelDescription",ae.model);
  
    
    obj_setObjectAtIndex(inventory_index_object_index, (const void *)&ae);  
     
    return inventory_index_object_index;

}

error_msg deleteInventoryResource(obj_index_t objIdx)
{
    obj_free(objIdx);    
        
    return SUCCESS;
}


error_msg inventoryGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
    inventory_eeprom_t p;
    static uint8_t inventoryElements;
    
    inventoryElements = INVENTORY_EEP_ELEMENTS;
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    /**** Use Encode to write data to the buffer ****/
    
    writeAuxData(&inventoryElements,MAP);  
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    ret = writeToEthInventoryData(&p);   
    
    return ret;
}

static error_msg writeToEthInventoryData(inventory_eeprom_t *p)
{    
    Get_ToEthernet((char *)"hwrv",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->hardwareRevision),TEXT_STRING);
    
    Get_ToEthernet((char *)"fwrv",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->firmwareRevision),TEXT_STRING);
    
    Get_ToEthernet((char *)"swrv",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->softwareRevision),TEXT_STRING);
    
    Get_ToEthernet((char *)"snum",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->serialNumber),TEXT_STRING);
    
    Get_ToEthernet((char *)"manu",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->manufacturer),TEXT_STRING);
    
    Get_ToEthernet((char *)"modl",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->model),TEXT_STRING);
 
    
    return SUCCESS;
}

error_msg inventoryPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg inventoryPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg inventoryPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg inventoryDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    inventory_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.firmwareRevision;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.hardwareRevision;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.manufacturer;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.serialNumber;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.softwareRevision;
    ret = string_removeWord(stringIdxToBeRemoved);

    return ret;
}


restfulMethodStructure_t inventoryMethods = {inventoryInit,inventoryGetter,inventoryPutter,inventoryPatcher,inventoryDeleter,inventoryPoster,initInventoryResource};


/**************************************************************Query Handlers**************************************************************************/
//Mo
bool queryHwrvHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    inventory_eeprom_t p;    
     
     if(hwrv_query==0)
     {
        obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.hardwareRevision == query_value)   //jira: CAE_MCU8-5647
        {                
            hwrv_query = 1;
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

bool queryFwrvHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    inventory_eeprom_t p;    
     
     if(fwrv_query==0)
     {        
       
        obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.firmwareRevision == query_value)   //jira: CAE_MCU8-5647
        {
            fwrv_query = 1;
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

bool querySwrvHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    inventory_eeprom_t p;    
     
     if(swrv_query==0)
     { 
        obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.softwareRevision == query_value)   //jira: CAE_MCU8-5647
        {
            swrv_query = 1;
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

bool querySnumHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    inventory_eeprom_t p;    
     
     if(snum_query==0)
     { 
        obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.serialNumber == query_value)   //jira: CAE_MCU8-5647
        {
            snum_query = 1;
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

bool queryManuHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    inventory_eeprom_t p;    
     
    if(manu_query==0)
    {
         obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.manufacturer == query_value)  //jira: CAE_MCU8-5647
        {
            manu_query = 1;
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

bool queryModlHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    inventory_eeprom_t p;    
     
    if(modl_query==0)
    {
         obj_getObjectAtIndex(objIdx, (const void *)&p);
        if((uint16_t)p.model == query_value)  //jira: CAE_MCU8-5647
        {
            modl_query = 1;
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


char* getMyManufacturerName(obj_index_t objIdx)
{

    inventory_eeprom_t p;

    obj_getObjectAtIndex(objIdx, (const void *)&p);
    return (string_getWordAtIndex(p.manufacturer));
}
