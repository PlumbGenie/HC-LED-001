
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ethernet_driver.h"
#include "contentFormat.h"
#include "cborCF.h"
#include "cbor.h"


#define ARRAYSIZE(a)    (sizeof(a) / sizeof(*(a)))

/*CBOR*/

void CBOREncoder(char *data, ContentDataType_t type)  //jira: CAE_MCU8-5647
{
//    uint8_t i =0;     //jira: CAE_MCU8-5647
    char str_val[11];
    memset(str_val,0,11);   
            
    switch(type)
    {
        case SIGNED_INTEGER8:
            CBOR_Encode_Signed_Int((int8_t)atol((const char *)data));
            break;
        case SIGNED_INTEGER16: 
            CBOR_Encode_Signed_Int((int16_t)atol((const char *)data));
            break;
        case SIGNED_INTEGER32:                
            CBOR_Encode_Signed_Int((int32_t)atol((const char *)data));  //jira: CAE_MCU8-5647
            break;
        case UNSIGNED_INTEGER8:
            CBOR_Encode_Unsigned_Int((uint8_t)atol((const char *)data));
            break;
        case UNSIGNED_INTEGER16:
            CBOR_Encode_Unsigned_Int((uint16_t)atol((const char *)data));
            break;
        case UNSIGNED_INTEGER32:
            CBOR_Encode_Unsigned_Int((uint32_t)atol((const char *)data)); //jira: CAE_MCU8-5647
            break;
        case BOOLEAN:
            CBOR_Encode_Bool((bool)atoi((const char *)data)); //jira: CAE_MCU8-5647
            break;
        case BYTE_STRING: 
            CBOR_Encode_Byte_String((char *)data);
            break;
        case TEXT_STRING:
            CBOR_Encode_Text_String((char *)data);
            break;        
        default:
            break;
    }    
    
}

void CBORDecoder(char *data ,void *variable, ContentDataType_t type)
{
    char *x;
    x = variable;
     switch(type)
    {
        
        case SIGNED_INTEGER16:           
        case UNSIGNED_INTEGER16:
            for (uint8_t i=0; i<2;i++)
            {
                x[i]= data[i];
            }
            break;
        case SIGNED_INTEGER32:           
        case UNSIGNED_INTEGER32:
            for (uint8_t i=0; i<4;i++)
            {
                x[i]= data[i];
            }
            break;
        case SIGNED_INTEGER8:           
        case UNSIGNED_INTEGER8:
        case BOOLEAN:     
        case BYTE_STRING:            
        case TEXT_STRING:
        
            *x = *data;
            break;      
         
        default:
            break;
    }
    
}

uint8_t CBORParseData(char * data)
{
    uint8_t len =0;
    
    len = CBOR_Decode_Read_Data(data);
    
    if(strncmp(data,"MAP",4)==0)
    {
        CfElements.array_map_elements = cbor_elements();
    }
    else if(strncmp(data,"ARRAY",6)==0)
    {
        CfElements.array_elements = cbor_elements();
    }
       
    return len;
}

void CBORWriteData(char *data,cFMenuItems items)     //jira: CAE_MCU8-5647
{
    volatile uint8_t x =0;   
    char *value = data;
    
    switch(items)
    {
        case MAP:
            ETH_Write8(CBOR_MAJOR_MAP | *value);    
            break;
        case ARRAY:
             ETH_Write8(CBOR_MAJOR_ARRAY | *value);    
            break;
        default:
            break;
    }
}

const contentFormatHandlers_t CBORdataHandlers = {CBOREncoder,CBORDecoder, CBORParseData, CBORWriteData};