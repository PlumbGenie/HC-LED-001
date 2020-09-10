/**
  CBOR.C  implementation

  Company:
    Microchip Technology Inc.

  File Name:
    cbor.c

  Summary:
     This is the implementation of CBOR (RFC 7049).

  Description:
    This source file provides the implementation of the APIs for CBOR Binary encoding and decoding.

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
#include "cbor.h"

//#include "byteorder.h"
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "ethernet_driver.h"
#include "log.h"


#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_CBOR, msgSeverity, msgLogDest) 
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

#define CBOR_MAJORTYPE_MASK 0xE0u   /*   3 high bits    */     //jira: CAE_MCU8-5647
#define CBOR_ADDEDINFO_MASK 0X1Fu   /*   5 lower bits   */     //jira: CAE_MCU8-5647

//uint8_t additionalInfo;

typedef struct{
    uint8_t dataType;
    uint16_t dataBytes;
    uint8_t additionalInfo;
}data_type_len_info_t;

data_type_len_info_t data_tli;


/***************************************************************************************/
/****************************************Helper Functions*******************************/
/***************************************************************************************/

static uint8_t  cbor_encode_uint_additional_info(uint32_t val);
static uint8_t  cbor_expected_length(uint8_t val);
static uint8_t  encode_byte_follows_helper(uint8_t val);
static void     int8_encodehandler(int32_t val);
static void     int16_encodehandler(int32_t val);
static void     int32_encodehandler(int32_t val);
static void     CBOR_Encode_ArrayHelper(Array_or_Map_Item_t *Array);
static void     CBOR_Encode_MapsHelper(Array_or_Map_Item_t  *Map);

uint8_t         cbor_decode_read_int(char *readPtr);
uint8_t         cbor_decode_read_string(char *readPtr);
uint8_t         cbor_decode_tags(char *readPtr);
uint8_t         cbor_decode_bool(char *readPtr);

//char maps[] = "MAP";            //jira: CAE_MCU8-5647
//char arrays[] = "ARRAY";        //jira: CAE_MCU8-5647

static uint8_t cbor_uint_additional_info(uint32_t val)
{
    if (val < CBOR_UINT8_FOLLOWS) return val;
    else if (val <= 0xFF) return CBOR_UINT8_FOLLOWS;
    else if (val > 0xFF && val  <=0xFFFF) return CBOR_UINT16_FOLLOWS;
    else if (val > 0xFFFF && val<0XFFFFFFFF)return CBOR_UINT32_FOLLOWS;
    else return 0;
}
static uint8_t encode_byte_follows_helper(uint8_t val)
{
    if (val == CBOR_UINT8_FOLLOWS)       return 0;
    else if (val == CBOR_UINT16_FOLLOWS) return 1;
    else if (val == CBOR_UINT32_FOLLOWS) return 2;
    return 0;
}

//jira: CAE_MCU8-5647
//static uint8_t decode_CBORType_helper(uint8_t val)
//{
//    if ((val == CBOR_MAJOR_UINT) || (val == CBOR_MAJOR_NEGINT)) return 0;
//    else if ((val == CBOR_MAJOR_BYTE_STRING) || (val == CBOR_MAJOR_TEXT_STRING)) return 1;
//    else if (val == CBOR_MAJOR_ARRAY)      return 2;
//    else if (val == CBOR_MAJOR_MAP)        return 3;
//    else if (val == CBOR_MAJOR_TAG)        return 4;
//    else if (val == CBOR_MAJOR_FLOATNMORE) return 5;
//    return 0;
//}

static void int8_encodehandler(int32_t val)
{
    ETH_Write8((uint32_t)val);                //jira: CAE_MCU8-5647
}
static void int16_encodehandler(int32_t val)
{
    ETH_Write16((uint32_t)val);               //jira: CAE_MCU8-5647
}

static void int32_encodehandler(int32_t val)
{
    ETH_Write32((uint32_t)val);              //jira: CAE_MCU8-5647
}

static void uint8_encodehandler(uint32_t val)
{
    ETH_Write8(val);
}
static void uint16_encodehandler(uint32_t val)
{
    ETH_Write16(val);
}

static void uint32_encodehandler(uint32_t val)
{
    ETH_Write32(val);
}

typedef void (*encodefunctptr_signed_int_t)   (int32_t);
typedef void (*encodefunctptr_unsigned_uint_t)(uint32_t);
typedef uint8_t (*decodefunctptr_t)(uint8_t *);
const encodefunctptr_signed_int_t    encodefunctptr_int_t[] =  {int8_encodehandler, int16_encodehandler, int32_encodehandler};
const encodefunctptr_unsigned_uint_t encodefunctptr_uint_t[] = {uint8_encodehandler, uint16_encodehandler, uint32_encodehandler};

/***************************************************************************************/
/********************************************Encoding***********************************/
/***************************************************************************************/

void CBOR_Encode_Signed_Int(int32_t val)
{
    if (val>=0)
    {
        data_tli.additionalInfo = cbor_uint_additional_info((uint32_t)val);        //jira: CAE_MCU8-5647
        ETH_Write8 ( CBOR_MAJOR_UINT | data_tli.additionalInfo );
    }
    else
    {
        data_tli.additionalInfo = cbor_uint_additional_info((uint32_t)(-1-val));   //jira: CAE_MCU8-5647
        ETH_Write8 ( CBOR_MAJOR_NEGINT | data_tli.additionalInfo );
    }
    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS )
    {
        if (val>=0)
            encodefunctptr_uint_t[encode_byte_follows_helper(data_tli.additionalInfo)]((uint32_t)val);      //jira: CAE_MCU8-5647
        else
            encodefunctptr_int_t[encode_byte_follows_helper(data_tli.additionalInfo)]((uint32_t)(-1-val));  //jira: CAE_MCU8-5647
    }
}

void CBOR_Encode_Unsigned_Int(uint32_t val)
{
    data_tli.additionalInfo = cbor_uint_additional_info(val);
    ETH_Write8 ( CBOR_MAJOR_UINT | data_tli.additionalInfo );
    
    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS )
    {
        encodefunctptr_uint_t[encode_byte_follows_helper(data_tli.additionalInfo)](val);
    }
}

void CBOR_Encode_Byte_String(char *byteString) 
{
    uint32_t len;
    uint32_t length = 0;
    len = strlen(byteString);
    
    data_tli.additionalInfo = cbor_uint_additional_info(len);
    ETH_Write8 (CBOR_MAJOR_BYTE_STRING | data_tli.additionalInfo);
    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS)
    {
        switch(data_tli.additionalInfo)
        {
                case CBOR_UINT8_FOLLOWS:
                    ETH_Write8(len);
                    break;
                case CBOR_UINT16_FOLLOWS:
                    ETH_Write16(len);
                    break;
                case CBOR_UINT32_FOLLOWS:
                    ETH_Write32(len);
                    break;
                default:
                        break;        
        }        
    }
    for(length=0; length<len; length++)
    {
        ETH_Write8(*byteString++);
    }
}

void CBOR_Encode_Text_String(const char *textString)
{
    uint32_t len = 0, length = 0;
	
    len = strlen(textString);    
    data_tli.additionalInfo = cbor_uint_additional_info(len);
    ETH_Write8 (CBOR_MAJOR_TEXT_STRING | data_tli.additionalInfo);
	
    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS )
    {
        switch(data_tli.additionalInfo)
        {
                case CBOR_UINT8_FOLLOWS:
                    ETH_Write8(len);
                    break;
                case CBOR_UINT16_FOLLOWS:
                    ETH_Write16(len);
                    break;
                case CBOR_UINT32_FOLLOWS:
                    ETH_Write32(len);
                    break;
                default:
                    break;  
        }
    }
    for(length=0; length<len; length++)
    {
        ETH_Write8(*textString++);
    }    
}

void CBOR_Encode_ArrayDefinite(Array_or_Map_Item_t *Array, uint32_t length) 
{
    static uint8_t Array_Type;
    int32_t int_t_temp;
    uint32_t uint_t_temp, len = length;
    
    data_tli.additionalInfo = cbor_uint_additional_info(len);
    ETH_Write8 (CBOR_MAJOR_ARRAY | data_tli.additionalInfo);

    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS )
        encodefunctptr_uint_t[encode_byte_follows_helper(data_tli.additionalInfo)](length); /*To provide information about Array length*/
    
    for (len=0; len<length; len++)        
    {
        Array_Type = (uint8_t)Array->Array_or_Map_majortype;    //jira: CAE_MCU8-5647
        switch (Array_Type)
        {
            case CBOR_MAJOR_UINT :
                uint_t_temp =  Array->Element.uint_Element;
                CBOR_Encode_Unsigned_Int(uint_t_temp);
                Array++;
                break;
            case CBOR_MAJOR_NEGINT:                
                int_t_temp = Array->Element.int_Element;
                CBOR_Encode_Signed_Int(int_t_temp);
                Array++;
                break;
            case CBOR_MAJOR_BYTE_STRING:
                CBOR_Encode_Byte_String(Array->Element.char_Element);
                Array++;
                break;
            case CBOR_MAJOR_TEXT_STRING:
                CBOR_Encode_Text_String(Array->Element.char_Element);
                Array++;
                break;
            case CBOR_MAJOR_ARRAY:
                CBOR_Encode_ArrayHelper(Array);
                Array++;
                break;
            case CBOR_MAJOR_MAP:
                CBOR_Encode_MapsHelper(Array);
                Array++;
                break;   
            default:
                logMsg("Array element of unsupported major type", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                break;
                
        }
//        }
    }
}

static void CBOR_Encode_ArrayHelper(Array_or_Map_Item_t *Array)
{
    uint32_t uint_t_temp, len, length;
    int32_t int_t_temp;
    
    length  = Array->Array_or_Map_internalMemberLength;
    data_tli.additionalInfo = cbor_uint_additional_info(length);
    ETH_Write8 (CBOR_MAJOR_ARRAY << 5 | data_tli.additionalInfo);

    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS)
        encodefunctptr_uint_t[encode_byte_follows_helper(data_tli.additionalInfo)](length); /*To provide information about internal Array length*/
    
    for (len=0; len< length; len++)                 /*To provide the actual array elements*/
    {   
        switch(Array->Array_or_Map_internalMemberType)
        {
            case CBOR_MAJOR_UINT :
                uint_t_temp =  Array->Element.uint_Element;
                CBOR_Encode_Unsigned_Int(uint_t_temp);
                break;                
            case CBOR_MAJOR_NEGINT:                 
                int_t_temp = Array->Element.int_Element;
                CBOR_Encode_Signed_Int(int_t_temp);                
                break;                
            case CBOR_MAJOR_BYTE_STRING:
                CBOR_Encode_Text_String(Array->Element.char_Element);
            break;                
            case CBOR_MAJOR_TEXT_STRING:
                CBOR_Encode_Text_String((char*)Array->Element.char_Element);
            break;            
            case CBOR_MAJOR_MAP:   
                CBOR_Encode_MapsHelper(Array);
            break;             
            default:
                logMsg("Array element of unsupported major type", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            break;
        }
    }
}

void CBOR_Encode_MapsDefinite(Array_or_Map_Item_t *Map, uint32_t DataItemPairs) 
{
    
    uint8_t Map_Type;
    uint32_t len, length, uint_t_temp;
    int32_t int_t_temp;
    
    length = (DataItemPairs*2);
    data_tli.additionalInfo = cbor_uint_additional_info(DataItemPairs);
    ETH_Write8 (CBOR_MAJOR_MAP | data_tli.additionalInfo);

    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS )
        encodefunctptr_uint_t[encode_byte_follows_helper(data_tli.additionalInfo)](DataItemPairs);  /*To provide information about Map length*/
    
    for (len=0; len< length; len++)        
    {
        Map_Type = (uint8_t)Map->Array_or_Map_majortype;   //jira: CAE_MCU8-5647
        switch (Map_Type)
        {
            case CBOR_MAJOR_UINT :
                uint_t_temp =  Map->Element.uint_Element;
                CBOR_Encode_Unsigned_Int(uint_t_temp);
                Map++;
                break;
            case CBOR_MAJOR_NEGINT:    
                int_t_temp = Map->Element.int_Element;
                CBOR_Encode_Signed_Int(int_t_temp);
                Map++;
                break;
            case CBOR_MAJOR_BYTE_STRING:
                CBOR_Encode_Byte_String((char*)Map->Element.char_Element);
                Map++;
                break;
            case CBOR_MAJOR_TEXT_STRING:
                CBOR_Encode_Text_String((char*)Map->Element.char_Element);
                Map++;
                break;
            case CBOR_MAJOR_ARRAY:
                CBOR_Encode_ArrayHelper(Map);
                Map++;
                break;
            case CBOR_MAJOR_MAP:                         
                CBOR_Encode_MapsHelper(Map);
                Map++;
                break;     
            default:
                logMsg("Map element of unsupported major type", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            break;                 
        }
    }
}

static void CBOR_Encode_MapsHelper(Array_or_Map_Item_t *Map)
{    
    int32_t int_t_temp;
    uint32_t len, numofpairs, length, uint_t_temp;
    
    length  = Map->Array_or_Map_internalMemberLength;
    numofpairs = length*2;
    data_tli.additionalInfo = cbor_uint_additional_info(length);
    ETH_Write8 (CBOR_MAJOR_ARRAY << 5 | data_tli.additionalInfo);

    if(data_tli.additionalInfo>= CBOR_UINT8_FOLLOWS)
        encodefunctptr_uint_t[encode_byte_follows_helper(data_tli.additionalInfo)](length); /*To provide information about internal Map length*/
    
    for (len=0; len< numofpairs; len++)                 /*To provide the actual array elements*/
    {   
        switch(Map->Array_or_Map_internalMemberType)
        {
            case CBOR_MAJOR_UINT :
                uint_t_temp =  Map->Element.uint_Element;
                CBOR_Encode_Unsigned_Int(uint_t_temp);
                break;
            case CBOR_MAJOR_NEGINT:                 
                int_t_temp = Map->Element.int_Element;
                CBOR_Encode_Signed_Int(int_t_temp);                
                break;                
            case CBOR_MAJOR_BYTE_STRING:
                CBOR_Encode_Text_String((char*)Map->Element.char_Element);
            break;                
            case CBOR_MAJOR_TEXT_STRING:
                CBOR_Encode_Text_String((char*)Map->Element.char_Element);
            break;             
            default:
                logMsg("Map element of unsupported major type", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            break;
        }
    }
}

void CBOR_Encode_Bool(bool boolvalue)
{
    if (boolvalue == true || boolvalue == 1)
        ETH_Write8 (CBOR_MAJOR_FLOATNMORE_TRUE);
    else if (boolvalue == false || boolvalue == 0)
        ETH_Write8 (CBOR_MAJOR_FLOATNMORE_FALSE);
}

/***************************************************************************************/
/********************************************Decoding***********************************/
/***************************************************************************************/

int8_t CBOR_Decode_Bytes_To_Follow(uint8_t val)
{
    uint8_t bytesToFollow[]={1,2,4};
    val = val & 0x1Fu;                              //jira: CAE_MCU8-5647
    if (val < CBOR_UINT8_FOLLOWS) {
        return 0;
    }
    if (val > CBOR_UINT32_FOLLOWS) {
        return -1;
    }
    return bytesToFollow[data_tli.additionalInfo - CBOR_UINT8_FOLLOWS];
}

data_type_t CBOR_Decode_Read_Data(char *readPtr)
{
    volatile uint8_t infoByte;
    uint8_t len=0;
    infoByte= ETH_Read8();
    
    data_tli.dataType = infoByte & 0xE0u;          //jira: CAE_MCU8-5647
    data_tli.dataBytes =infoByte & 0x1Fu;          //jira: CAE_MCU8-5647
    len++;
    switch(data_tli.dataType)
    {
        case CBOR_MAJOR_UINT:
        case CBOR_MAJOR_NEGINT:
            len += cbor_decode_read_int(readPtr);
            break;
        case CBOR_MAJOR_BYTE_STRING:
        case CBOR_MAJOR_TEXT_STRING:
            len += cbor_decode_read_string(readPtr);
            break;
        case CBOR_MAJOR_ARRAY:
            strcpy(readPtr,"ARRAY");
            break;
        case CBOR_MAJOR_MAP:
            strcpy(readPtr,"MAP");
            break;
        case CBOR_MAJOR_TAG:
            len += cbor_decode_tags(readPtr);                  
            break;
        case CBOR_MAJOR_FLOATNMORE:
            if ((CBOR_MAJOR_FLOATNMORE || data_tli.dataBytes ==  CBOR_MAJOR_FLOATNMORE_TRUE) ||(CBOR_MAJOR_FLOATNMORE || data_tli.dataBytes ==  CBOR_MAJOR_FLOATNMORE_FALSE))
                len += cbor_decode_bool (readPtr);
            break;
        default:
            break;
    }
    return len;
}

uint8_t cbor_decode_read_int(char *readPtr)
{
    int8_t i;
    uint8_t bytesRead = 0;
   
        switch (data_tli.dataBytes)
        {
            case CBOR_UINT8_FOLLOWS:
                *readPtr = ETH_Read8();
                if (data_tli.dataType == CBOR_MAJOR_NEGINT ) *readPtr +=1;
                break;
            case CBOR_UINT16_FOLLOWS: // start here
                for(i=1; i>=0; i--)
                {
                    readPtr[i] = ETH_Read8();
                }
                if (data_tli.dataType == CBOR_MAJOR_NEGINT ) readPtr[0] +=1;
                break;
            case CBOR_UINT32_FOLLOWS:
                for(i=3; i>=0; i--)
                {
                    readPtr[i] = ETH_Read8();
                }
                if (data_tli.dataType == CBOR_MAJOR_NEGINT ) readPtr[0] +=1;
                break;
            default:
                if (data_tli.dataBytes < 24)
                    *readPtr = data_tli.dataBytes;
                if (data_tli.dataType == CBOR_MAJOR_NEGINT ) *readPtr += 1;
                break;
        }
        if((data_tli.dataBytes)<24)
        {
            bytesRead = 0;
        }
        else if((data_tli.dataBytes)== CBOR_UINT8_FOLLOWS)
        {
            bytesRead = 1;
        }
        else if((data_tli.dataBytes)== CBOR_UINT16_FOLLOWS)
        {
            bytesRead = 2;
        }
        else if((data_tli.dataBytes)== CBOR_UINT32_FOLLOWS)
        {
            bytesRead = 4;
        }

    
    return bytesRead;
}

uint8_t cbor_decode_read_string(char *readPtr)
{
    int32_t i;  
    uint32_t bytesToRead = 0;
   
        switch (data_tli.dataBytes)
        {
            case CBOR_UINT8_FOLLOWS:
                bytesToRead = ETH_Read8();
                for(i=bytesToRead; i>0; i--)
                {
                    *readPtr++ = ETH_Read8();
                }
                break;
            case CBOR_UINT16_FOLLOWS: // start here
                bytesToRead = ETH_Read16();
                for(i=bytesToRead; i>0; i--)
                {
                    *readPtr++ = ETH_Read8();
                }
                break;
            case CBOR_UINT32_FOLLOWS:
                bytesToRead = ETH_Read32();
                for(i=bytesToRead; i>0; i--)
                {
                    *readPtr++ = ETH_Read8();
                }
                break;
            default:
                if (data_tli.dataBytes < 24)
                {
                    bytesToRead = data_tli.dataBytes;
                    for(i=bytesToRead; i>0; i--)
                    {
                        *readPtr++ = ETH_Read8();
                    }
                }
                break;
        }
        if((data_tli.dataBytes)<24)
        {
            bytesToRead += 0;
        }
        else if((data_tli.dataBytes)== CBOR_UINT8_FOLLOWS)
        {
            bytesToRead += 1;
        }
        else if((data_tli.dataBytes)== CBOR_UINT16_FOLLOWS)
        {
            bytesToRead += 2;
        }
        else if((data_tli.dataBytes)== CBOR_UINT32_FOLLOWS)
        {
            bytesToRead += 4;
        }


    return bytesToRead;
    
}
uint8_t cbor_elements()
{
    return data_tli.dataBytes;
}

uint8_t cbor_decode_tags(unsigned char *readPtr)           //jira: CAE_MCU8-5647
{
 uint8_t length = 0;   
    switch (data_tli.dataBytes)
    {
            case CBOR_DATETIME_STRING_FOLLOWS:
                *readPtr = ETH_Read8();
                data_tli.dataType  = (*readPtr & CBOR_MAJORTYPE_MASK);     /*Get the major type*/
                if (data_tli.dataType == CBOR_MAJOR_BYTE_STRING)           /*Data item has to be a UTF-8 string*/
                {
                    data_tli.dataBytes = (*readPtr & CBOR_ADDEDINFO_MASK);
                    length=cbor_decode_read_string((unsigned char *)readPtr);        //jira: CAE_MCU8-5647
                }
                break;
            case CBOR_DATETIME_EPOCH_FOLLOWS:
                *readPtr = ETH_Read8();
                data_tli.dataType = (*readPtr & CBOR_MAJORTYPE_MASK);      /*Check whether int or float*/
                data_tli.dataBytes = (*readPtr & CBOR_ADDEDINFO_MASK); 
                        switch (data_tli.dataType)
                        {
                            case CBOR_MAJOR_UINT:           
                            case CBOR_MAJOR_NEGINT:
                                length=cbor_decode_read_int(readPtr);
                                break;
                            case CBOR_MAJOR_FLOATNMORE:      /**/
                                break;
                            default:
                                break;
                        }       
                break;
            default:    
                break;               
    }
    return length;
}

uint8_t cbor_decode_bool(unsigned char *readPtr)             //jira: CAE_MCU8-5647
{
    uint8_t length=0;
    *readPtr = (unsigned char)(CBOR_MAJOR_FLOATNMORE || data_tli.dataBytes);          /*For true and false values*/ //jira: CAE_MCU8-5647
    return length;
}