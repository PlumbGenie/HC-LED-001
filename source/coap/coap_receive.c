/**
  CoAP Receive implementation

  Company:
    Microchip Technology Inc.

  File Name:
    coap_receive.c

  Summary:
     This is the implementation of CoAP Receive.

  Description:
    This source file provides the implementation of the Receive side of CoAP protocol.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tcpip_types.h"
#include "udpv4.h"
#include "coap.h"
#include "coap_options.h"

/********************************************Static Variables****************************/
static coapHeader_t coapReceiveHeader= {0}; 
coapReceive_t coapRecv={0};
static char  recvdClientToken[COAP_TOKEN_SIZE];
/********************************************Local API Prototypes****************************/
static bool COAP_ReceiveNONMessage(int16_t length);
static bool COAP_ReceiveCONMessage(int16_t length);
static bool COAP_ReceiveACKMessage(int16_t length);
static bool COAP_ReceiveRSTMessage(int16_t length);
static int8_t COAP_ReadRecivedHeaderAndToken(void);
static void COAP_setReceivedClientToken(char *tkn,uint8_t tkl);

/********************************************CoAP Receive APIs Implementation ****************************/
static int8_t COAP_ReadRecivedHeaderAndToken(void)
{
    int8_t ret = 0;  
    
    memset(&coapReceiveHeader,0,sizeof(coapReceiveHeader));
    UDP_ReadBlock((uint8_t*)&coapReceiveHeader,sizeof(coapReceiveHeader));
    ret += sizeof(coapReceiveHeader);    
    
    if(coapReceiveHeader.ver == 1)
    {
        coapRecv.messageID = UDP_Read16();            
        ret += 2;        
        
        coapRecv.code = coapReceiveHeader.code;
        coapRecv.destIP = UDP_GetDestIP();
        coapRecv.destPort = UDP_GetDestPort();
        coapRecv.srcPort = UDP_GetSrcPort();
        coapRecv.type = coapReceiveHeader.t;
        if(coapReceiveHeader.tkl > COAP_TOKEN_SIZE)
        {
            return -1;
        }
        else if(coapReceiveHeader.tkl > 0 && coapReceiveHeader.tkl <= COAP_TOKEN_SIZE)
        {
            UDP_ReadBlock(coapRecv.token,coapReceiveHeader.tkl);
            COAP_setReceivedClientToken(coapRecv.token,coapReceiveHeader.tkl);
            ret += coapReceiveHeader.tkl;
        }
    }
    else
    {
        return -1;
    }
    
    return ret;
}

static void COAP_setReceivedClientToken(char *tkn,uint8_t tkl)
{
    memset(recvdClientToken,0,COAP_TOKEN_SIZE);
    memcpy(recvdClientToken, tkn, tkl);
}

static bool COAP_ReceiveCONMessage(int16_t length)
{
    bool ret = false;
    
    if(length > 0)
    {
        ret = COAP_ParseOptions(&coapRecv,length); 
        if(ret)
        {
            ret = COAP_TraverseToResource(&coapRecv);                    
        }
    }
    if(ret == false)
    {
        coapRecv.code = (uint8_t) coapRecv.retCode;   //jira: CAE_MCU8-5647
        coapRecv.type = ACK_TYPE;        
    }      
    
    return ret;
}

static bool COAP_ReceiveNONMessage(int16_t length)
{
     bool ret = false;
    
    if(length > 0)
    {
        ret = COAP_ParseOptions(&coapRecv,length); 
        if(ret)
        {
            ret = COAP_TraverseToResource(&coapRecv);                    
        }
    }
    if(ret == false)
    {
       coapRecv.code = (uint8_t)coapRecv.retCode;        //jira: CAE_MCU8-5647
       coapRecv.type = RST_TYPE;       
    } 
    return ret;
}

static bool COAP_ReceiveACKMessage(int16_t length)
{
    bool ret = false; 

    if(length > 0)
    {
        ret = COAP_ParseOptions(&coapRecv,length);
        if(ret)
        {            
            ret = COAP_TraverseToResource(&coapRecv);
        }
    }
    else
    {
        //Do Nothing
        ret = true;        
    }
    if(ret == false)
    {
        coapRecv.code = (uint8_t)coapRecv.retCode;        //jira: CAE_MCU8-5647
        coapRecv.type = RST_TYPE;        
    } 
    return ret;
}

static bool COAP_ReceiveRSTMessage(int16_t length)
{
    bool ret = false; 
    
    if(length > 0)
    {
        ret = COAP_ParseOptions(&coapRecv,length);
        if(ret)
        {            
            ret = COAP_TraverseToResource(&coapRecv);
        }
    }
    else
    {
        //Do Nothing
        ret = true;        
    }
    if(ret == false)
    {
        coapRecv.code = (uint8_t)coapRecv.retCode;        //jira: CAE_MCU8-5647
        coapRecv.type = RST_TYPE;        
    } 
    return ret;
}

bool COAP_MessageResponse(coapReceive_t *ptr)
{
    error_msg ret = ERROR;

    COAP_ResetLastOptionNumber();
    coapReceiveHeader.code = ptr->code;
    coapReceiveHeader.t = ptr->type;

    ret = UDP_Start(ptr->destIP, ptr->srcPort, ptr->destPort);

    if(ret == SUCCESS)
    {
        UDP_WriteBlock((char*)&coapReceiveHeader,sizeof(coapReceiveHeader));
        UDP_Write16(ptr->messageID);
        if(coapReceiveHeader.tkl > 0)
        {
            UDP_WriteBlock((char*)ptr->token,coapReceiveHeader.tkl);
        }
        return true;        
    }
   
    return false;
}

bool COAP_ErrorResponse(coapReceive_t *ptr)
{
     error_msg ret;

     ptr->code = (uint8_t)ptr->retCode;     //jira: CAE_MCU8-5647
     ret = COAP_MessageResponse(ptr);
     if(ret== SUCCESS)
     {
         COAP_Send();
         return true;
     }
     return false;
}
   
    

void COAP_Receive(int16_t length)
{   
    bool ret = false;  
    int16_t len = 0;
    if((size_t)length > sizeof(coapHeader_t))
    {
        len=COAP_ReadRecivedHeaderAndToken();
        if(len != -1)
        {
            length -= len; 
            
            switch(coapReceiveHeader.t)
            {
                case CON_TYPE:       
                    ret = COAP_ReceiveCONMessage(length);
                    break;
                case NON_TYPE:
                    ret = COAP_ReceiveNONMessage(length);
                    break;
                case ACK_TYPE:
                    if((COAP_FreeEndpointMessageIDTokenPairIfMatched(coapRecv.destIP,coapRecv.messageID,coapRecv.token,coapReceiveHeader.tkl,ACK_TYPE) == 1))
                    {
                        ret = COAP_ReceiveACKMessage(length);
                    }
                    break;
                case RST_TYPE:
                    if((COAP_FreeEndpointMessageIDTokenPairIfMatched(coapRecv.destIP,coapRecv.messageID,coapRecv.token,coapReceiveHeader.tkl,RST_TYPE) == 1))
                    {
                        ret = COAP_ReceiveRSTMessage(length);
                    }
                    break;
                default: 
                    coapReceiveHeader.t = RST_TYPE;
                    coapReceiveHeader.code = BAD_REQUEST;
                    ret = false;
                    break;
            }
        }
        else
        {
            coapReceiveHeader.t = RST_TYPE;
            coapReceiveHeader.code = BAD_REQUEST;
            ret = false;
        }
        if(ret==false && coapReceiveHeader.t != ACK_TYPE && coapReceiveHeader.t != RST_TYPE)
        {
            COAP_MessageResponse(&coapRecv);
            COAP_Send();
        }
    }    
}


char * COAP_getReceivedClientToken(void)
{    
    return recvdClientToken;
}

void COAP_InsertLastOptionValue(coapReceive_t *ptr,char *optionValue,uint16_t optionLength)
{    
    ETH_Insert(optionValue,optionLength,sizeof(ethernetFrame_t) + sizeof(ipv4Header_t) + sizeof(udpHeader_t) + sizeof(coapHeader_t)+sizeof(uint16_t)+ strlen(ptr->token)+(COAP_GetNumberOfOptionsAdded() - 1u)*2u+1u);   //jira: CAE_MCU8-5647
}



