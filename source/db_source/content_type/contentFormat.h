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

  
#ifndef CONTENT_FORMAT_H
#define	CONTENT_FORMAT_H

#define CONTENT_FORMAT_SIZE    (2)



typedef enum
{
    SIGNED_INTEGER8,
    UNSIGNED_INTEGER8,
    SIGNED_INTEGER16,
    UNSIGNED_INTEGER16,
    SIGNED_INTEGER32,
    UNSIGNED_INTEGER32,
    BYTE_STRING,
    TEXT_STRING,
    BOOLEAN
}ContentDataType_t;

typedef enum
{
    TEXT_PLAIN = 0,           
    LINK_FORMAT = 40,            
    APPLICATION_JSON = 50,
    CBOR = 60,     
}content_format_t;

typedef enum 
{
     MAP, ARRAY, UN_KNOWN }cFMenuItems; 

typedef void (*contentFormatFunctionPointer)(char * ,ContentDataType_t);  //jira: CAE_MCU8-5647
typedef void (*contentFormatDecodeFunctionPointer)(char *,void *,ContentDataType_t);
typedef uint8_t (*parseFunctionPointer)(char *);
typedef void (*writeFunctionPointer)(char *, cFMenuItems);  //jira: CAE_MCU8-5647



typedef struct
{    
    contentFormatFunctionPointer encodeFormatter;       
    contentFormatDecodeFunctionPointer decodeFormatter;  
    parseFunctionPointer parseDataHandler;
    writeFunctionPointer writeDataHandler;    
}contentFormatHandlers_t;


typedef struct 
{
    uint8_t object_map_elements;
    uint8_t array_map_elements;
    uint8_t array_elements;
}elements_t;

 
extern elements_t CfElements;
//extern const coapContentFormatHandler_t coapContentFormatHandler[CONTENT_FORMAT_SIZE];

void Get_ToEthernet(char *data, ContentDataType_t type);   //jira: CAE_MCU8-5647
void Put_ToResource(char *data, void *variable, ContentDataType_t type);
uint8_t parseData(char *data,uint8_t sizeOfData);
void writeAuxData(void *data,cFMenuItems items);
void setContentFormatDataHandlers(uint8_t index);
bool iscfMatched(content_format_t cfType);

#endif	/* CONTENT_FORMAT_H */

