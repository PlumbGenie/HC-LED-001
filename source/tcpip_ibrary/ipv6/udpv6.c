/**
  UDP protocol v6 implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    udpv6.c

  Summary:
     This is the implementation of UDP version 6 protocol

  Description:
    This source file provides the implementation of the API for the UDP v6 protocol.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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
#include <stdbool.h>
#include <stddef.h>
#include "ipv4.h"
#include "udpv6.h"
#include "udpv6_port_handler_table.h"
#include "network.h"
#include "ethernet_driver.h"
#include "../tcpip_types.h"
#include "ipv6.h"
#include "icmpv6.h"

/**
  Section: Macro Declarations
*/

/**
  Section: Local Variables
*/

uint16_t destPort;
udpHeader_t udpHeader;

/**
  Section: UDP Library APIs
*/

error_msg UDPv6_Start(ipv6Address_t destIP, uint16_t srcPort, uint16_t dstPort)
{
     // Start a UDP Packet to Write UDP Header
    error_msg ret = ERROR;

    // Start IPv4 Packet to Write IPv4 Header
    ret = IPv6_Start(&destIP, UDP_TCPIP, 0);
    if(ret == SUCCESS)
    {
        //Start to Count the UDP payload length Bytes
        ETH_ResetByteCount();

        // Write UDP Source Port
        ETH_Write16(srcPort);

        //Write UDP Destination Port
        ETH_Write16(dstPort);

        //Write DataLength; Initially set to '0'
        ETH_Write16(0);

        //Write UDP Checksum; Initially set to '0'
        ETH_Write16(0);

    }
    return ret;
}

error_msg UDPv6_Send()
{
    uint16_t udpLength;
    uint16_t cksm;
    error_msg ret = ERROR;

    udpLength = ETH_GetByteCount();
    udpLength = ntohs(udpLength);
    ETH_Insert((char *)&udpLength, 2, sizeof(ethernetFrame_t) + sizeof(ipv6Header_t) - 1 + offsetof(udpHeader_t,length));
    udpLength = htons(udpLength);
    
//    // add the UDP header checksum
//    cksm = udpLength + UDP;
//    cksm = ETH_TxComputeChecksum(sizeof(ethernetFrame_t) + sizeof(ipv6Header_t) - 1 - 8, udpLength + 8, cksm);
//
//    // if the computed checksum is "0" set it to 0xFFFF
//    if (cksm == 0){
//        cksm = 0xffff;
//    }
//
//    ETH_Insert((char *)&cksm, 2, sizeof(ethernetFrame_t) + sizeof(ipv6Header_t) -1 + offsetof(udpHeader_t,checksum));

    ret = IPv6_Send(udpLength);

    return ret;
}

error_msg UDPv6_Receive(ipv6Address_t* dstAddr) // catch all UDP packets and dispatch them to the appropriate callback
{
    error_msg ret = PORT_NOT_AVAILABLE;
    udp_table_iterator_t  hptr;

    ETH_ReadBlock((char *)&udpHeader,sizeof(udpHeader));

    udpHeader.dstPort = ntohs(udpHeader.dstPort); // reverse the port number
    destPort = ntohs(udpHeader.srcPort);
    udpHeader.length = ntohs(udpHeader.length);
    
    // scan the udp port handlers and find a match.
    // call the port handler callback on a match
    hptr = udpv6_table_getIterator();

    while(hptr != NULL)
    {
        if(hptr->portNumber == udpHeader.dstPort)
        {          
            hptr->callBack(0);                    
            ret = SUCCESS;
            break;
        }
        hptr = udpv6_table_nextEntry(hptr);
    }
    if(ret == PORT_NOT_AVAILABLE)
    {
        ipv6Address_t src;
        src.s6_addr[1]  = (dstAddr->s6_addr[0]);
        src.s6_addr[0]  = (dstAddr->s6_addr[1]);
        src.s6_addr[3]  = (dstAddr->s6_addr[2]);
        src.s6_addr[2]  = (dstAddr->s6_addr[3]);
        src.s6_addr[5]  = (dstAddr->s6_addr[4]);
        src.s6_addr[4]  = (dstAddr->s6_addr[5]);
        src.s6_addr[7]  = (dstAddr->s6_addr[6]);
        src.s6_addr[6]  = (dstAddr->s6_addr[7]);
        src.s6_addr[9]  = (dstAddr->s6_addr[8]);
        src.s6_addr[8]  = (dstAddr->s6_addr[9]);
        src.s6_addr[11] = (dstAddr->s6_addr[10]);
        src.s6_addr[10] = (dstAddr->s6_addr[11]);
        src.s6_addr[13] = (dstAddr->s6_addr[12]);
        src.s6_addr[12] = (dstAddr->s6_addr[13]);
        src.s6_addr[15] = (dstAddr->s6_addr[14]);
        src.s6_addr[14] = (dstAddr->s6_addr[15]);
        //Send Port unreachable                
        ICMPv6_DestinationUnreachableMessage(&src, (sizeof(udp_handler_t) + (udpHeader.length)));
    }

    return ret;
}


