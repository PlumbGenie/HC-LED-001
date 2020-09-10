
/**
 udpv4_echo_server_client.c
	
  Company:
    Microchip Technology Inc.


  Description:
 This file was created to register my device with the GUI

 */

/*

[2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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
#include "udpv4.h"
#include "udpv4_echo_server_client.h"
#include <stdio.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(*a))

void UDP_EchoServer(int length)
{
    static uint16_t bytesSent=0;
    uint16_t i;
    printf("Echo_payload_length=%d\r\n",length);
    if(length>0 && length<=1500) //max mtu size
    {
        if(UDP_Start(UDP_GetDestIP(),7,UDP_GetDestPort()))
        {
            while(length--)
            {
                UDP_Write8(UDP_Read8());
                bytesSent++;
            }
            printf("-----ECHO BYTES SENT = %d\r\n",bytesSent);
            UDP_Send();
        }
        bytesSent=0;
    }
    return;
}

void UDP_EchoClient(uint8_t *buffer,uint16_t buffLen, uint32_t destIP)
{

    if(UDP_Start(destIP,ECHO_SOURCE_PORT,7))
    {
//        printf("Beginning buffLen=%d\r\n",buffLen);
        while(buffLen)
        {
//            printf("buffLen=%d\r\n",buffLen);
            UDP_Write8(*buffer++);
            buffLen--;
        }
        UDP_Send();
    }
    
}
