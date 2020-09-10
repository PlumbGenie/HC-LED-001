#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ethernet_driver.h"
#include "contentFormat.h"
#include "textPlainCF.h"


#define ARRAYSIZE(a)    (sizeof(a) / sizeof(*(a)))

char  separator =  ',';



void TEXT_PLAINEncoder(char *data, ContentDataType_t type) //jira: CAE_MCU8-5647
{
//    uint8_t i =0;    //jira: CAE_MCU8-5647
    char str_val[11];
    memset(str_val,0,11);  
            
    switch(type)
    {
        case SIGNED_INTEGER8:
        case SIGNED_INTEGER16:
        case SIGNED_INTEGER32:
            sprintf(str_val,"%lu",(int32_t)atol((const char *)data));  //jira: CAE_MCU8-5647
            ETH_WriteString(str_val);
            ETH_Write8(separator);
            break;
        case UNSIGNED_INTEGER8:
        case UNSIGNED_INTEGER16:
        case UNSIGNED_INTEGER32:
            sprintf(str_val,"%lu",(uint32_t)atol((const char *)data));  //jira: CAE_MCU8-5647
            ETH_WriteString(str_val);
            ETH_Write8(separator);
            break;
        case BOOLEAN:            
            sprintf(str_val,"%u",(bool)atoi((const char *)data));     //jira: CAE_MCU8-5647
            ETH_WriteString(str_val);
            ETH_Write8(separator);
            break;
        case BYTE_STRING:            
        case TEXT_STRING:            
            ETH_WriteString((char *)data);
            ETH_Write8(separator);
            break;        
        default:
            break;
    }    
    
}

void TEXT_PLAINDecoder(char *data, void *variable, ContentDataType_t type)
{   
    uint32_t *x32 = variable;   
    uint16_t *x16 = variable; 
    uint8_t *x8 = variable; 
    uint32_t temp;
    switch(type)
    {
        case SIGNED_INTEGER8:           
        case UNSIGNED_INTEGER8:
        case SIGNED_INTEGER16:
        case UNSIGNED_INTEGER16:
        case SIGNED_INTEGER32:
         case UNSIGNED_INTEGER32:
        case BOOLEAN:   
            temp = (uint32_t)atol((const char *)data);    //jira: CAE_MCU8-5647
            if(temp <= 255)
            {
               *x8 = (uint8_t)temp; 
            }
            else if(temp > 255 && temp <65535)
            {
                *x16 = (uint16_t)temp;
            }
            else
            {
                *x32 = temp;
            }
            
            break;       
        case BYTE_STRING:            
        case TEXT_STRING: 
             strncpy(variable,data,strlen(data));      
            break;        
        default:
            break;
    }
    
}

uint8_t TEXT_PLAINParseData(char * data)
{
    uint8_t len =0;
    uint8_t str;
    char *x;
    
    x = data;    
    
    while(str = ETH_Read8())
    {
        if(str != separator)
        {
            *data = str;
            *data++;
            len++;           
        }
        else
        {
            len++;
            break;
        }
    }
    if(strncmp(x,"MAP",4)==0)
    {        
        str = ETH_Read8();
        len++;
        CfElements.array_map_elements = (uint8_t)atol((const char *)&str);    //jira: CAE_MCU8-5647
        printf("CfElements.array_map_elements=%d\r\n",CfElements.array_map_elements);
        str = ETH_Read8();
        len++;
    }
    else if(strncmp(x,"ARRAY",6)==0)
    {        
        str = ETH_Read8();
        len++;
        CfElements.array_elements = (uint8_t)atol((const char *)&str);        //jira: CAE_MCU8-5647
        printf("CfElements.array_elements=%d\r\n",CfElements.array_elements);
        str = ETH_Read8();
        len++;
    }
       
    return len;
}

void TEXT_PLAINWriteData(char * data, cFMenuItems items)
{    
    char str_val[11];
    memset(str_val,0,11);
    
    ETH_Write8(separator);
    sprintf(str_val,"%u",*data);  
    ETH_WriteString(str_val);
    ETH_Write8(separator);    
}

const contentFormatHandlers_t TEXT_PLAINdataHandlers = {TEXT_PLAINEncoder,TEXT_PLAINDecoder, TEXT_PLAINParseData, TEXT_PLAINWriteData};



