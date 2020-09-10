/**
 device_registry.c
	
  Company:
    Microchip Technology Inc.


  Description:
 This file was created to register my device with the GUI

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
#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "udpv4.h"
#include "device_registry.h"


uint8_t    STAY_ALIVE_FLAG=0;
char txDevicedata[] = "mchp_coap";
//char rxDevicedata[9];  //jira: CAE_MCU8-5647


void COAP_CL_deviceRegistry(int length)
{
    if (length >= 1)
        STAY_ALIVE_FLAG = UDP_Read8();
}

void devDiscoveHandler(int length)
{
//    if(length > 0)
//    {
//        memset(rxDevicedata,0,sizeof(rxDevicedata));
//        UDP_ReadBlock(rxDevicedata,length);
//        if(strcmp(txDevicedata,rxDevicedata)==0)
//        {
            DeviceDiscover(UDP_GetDestIP(),UDP_GetDestPort());
//        }
//    }
}

void DeviceDiscover(uint32_t destIP, uint16_t port)
{
    error_msg ret;
    
    
    ret = UDP_Start(destIP,65525,port);
    if(ret==SUCCESS)
    {
        UDP_WriteString(txDevicedata);
        UDP_Send();
    }    
}

void devReqPowerLevel(int length)
{
    char reqPwrLvl[5];
    
    memset(reqPwrLvl,0,sizeof(reqPwrLvl));
    
    UDP_ReadBlock(reqPwrLvl,UDP_DataLength());
    
    requiredPower = (uint16_t)atol((const char *)reqPwrLvl);  //jira: CAE_MCU8-5647
//    printf("Required Power Level %d \r\n",requiredPower);
    
}

