/**
  ICMPv6 protocol implementation

  Company:
    Microchip Technology Inc.

  File Name:
    icmpv6.c

  Summary:
     This is the implementation of ICMP version 6 protocol.

  Description:
    This source file provides the implementation of the API for the ICMP Echo Ping Request/Reply.

 */

/*

©  [2016] Microchip Technology Inc. and its subsidiaries.  You may use this software
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
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "network.h"
#include "tcpip_types.h"
#include "ethernet_driver.h"
#include "icmpv6.h"
#include "ipv6.h"
#include "ipv6_util.h"
#include "ndp.h"
#include "mac_address.h"
#include "ipv6_database.h"

#ifdef ENABLE_NETWORK_DEBUG
#include "log.h"
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest)  
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif


#define ICMPV6_MTU_OPT_SIZE                             (48)
#define ICMPV6_MLR_MESSAGE_SIZE                         (0)
#define ICMPV6_MLD_MESSAGE_SIZE                         (0)
#define ICMPV6_RS_W_SOURCE_MAC_OPTION_MESSAGE_SIZE      (8 + 1 + 1 + 6)
#define ICMPV6_RS_MESSAGE_SIZE                          (8)
#define ICMPV6_NS_MESSAGE_SIZE                          (8 + 16 + 1 + 1 + 6)
#define ICMPV6_DAD_NS_MESSAGE_SIZE                      (16 + 1 + 1 + 6)
#define ICMPV6_NA_MESSAGE_SIZE                          (8 + 16 + 1 + 1 + 6)

#define ICMPV6_PORT_UNREACHABLE     (4)
#define ICMPV6_ERR_MESSAGE_OPT      (4)

#define ICMPV6_PORT_UNREACHABLE_ERROR                   (ICMPV6_PORT_UNREACHABLE_CODE << 8 | ICMPV6_DESTINATION_UNREACHABLE_ERROR)


static uint16_t portUnreachable = 0; 

void (*icmpv6Cb)(icmpv6Header_t *icmpv6Hdr);
error_msg ICMPv6_EchoReply(ipv6Address_t* srcAddr, uint16_t pldLength);

/**
 * ICMP packet receive
 * @param ipv6_header
 * @return
 */
void ICMPv6_Receive(ipv6HeaderRx_t *ipv6Hdr)
{
    icmpv6Header_t icmpv6Hdr;
    bool partialValidMessage = false;
    uint16_t msgLen = ntohs(ipv6Hdr->payloadLength);
    
    ETH_ReadBlock(&icmpv6Hdr, sizeof(icmpv6Header_t));
    ETH_SaveRDPT();
    
    if( (ipv6Hdr->hopLimit == 255) && (icmpv6Hdr.code == 0) )
    {
        partialValidMessage = true;
    }
    
    switch( icmpv6Hdr.typeCode )
    {
        case ICMPV6_PORT_UNREACHABLE_ERROR:
            ETH_Dump(4 + sizeof(ipv6Header_t)); // Reserved bytes + IPv6 header
            portUnreachable = ntohs(ETH_Read16());
            break;

        case ICMPV6_ECHO_REQUEST:
            ICMPv6_EchoReply(ipv6Hdr->srcAddress, msgLen);
            break;

        case ICMPV6_MLD_QUERY:
            // If in the multicast group, respond with Multicast Listener Report
            // wait a random delay before sending MLR and listen to other Reports
            // MLD_Querry();)
            break;

        case ICMPV6_ROUTER_ADVERTISMENT:
            if( partialValidMessage )
            {
                NDP_HandleRouterAdvertisement(ipv6Hdr->srcAddress, msgLen);
            }
            break;
            
        case ICMPV6_NEIGHBOR_SOLICITATION:
            if( partialValidMessage )
            {
                NDP_HandleNeighborSolicitation(ipv6Hdr->srcAddress, msgLen);
            }
            break;

        case ICMPV6_NEIGHBOR_ADVERTISEMENT:
            if( partialValidMessage )
            {
                NDP_HandleNeighborAdvertisement(ipv6Hdr->srcAddress, ipv6Hdr->dstAddress, msgLen);
            }
            break;

        case ICMPV6_REDIRECT:
            if( partialValidMessage )
            {
                NDP_HandleRedirect(ipv6Hdr->srcAddress, msgLen);
            }
            break;

        default:
            if( icmpv6Cb )
            {
                icmpv6Cb(&icmpv6Hdr);
            }
            break;
    }

    return;
}

void ICMPv6_SetErrorCallback(void (*callbackFunction)(icmpv6Header_t *icmpv6Hdr))
{
    icmpv6Cb = callbackFunction;
}

bool ICMPv6_IsPortUnreachable(uint16_t port)
{
    if(portUnreachable == port)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ICMPv6_ResetPortUnreachable(void)
{
    portUnreachable = 0;
}

/**
 * ICMPv6 Error Message
 * @param Error message type
 * @param Error code
 * @return
 */
error_msg ICMPv6_DestinationUnreachableMessage(ipv6Address_t* dstAddr, uint16_t pldLength)
{
    //rfc4443
    error_msg result = ERROR;
    uint16_t oldPacketSize, ICMPv6PacketSize;
    
    result = IPv6_Start(dstAddr, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_DESTINATION_UNREACHABLE_ERROR);           //type
        ETH_Write8(ICMPV6_PORT_UNREACHABLE);                        //code
        ETH_Write16(0);                                             //checksum
        ETH_Write32(0);                                             //unused

        oldPacketSize = pldLength + sizeof(ipv6Header_t);           //old packet size
        ETH_SetReadPtr(Network_GetStartPosition());                 //point to the start of the old packet
        
        //As much of invoking packet                 |
        //as possible without the ICMPv6 packet          +
        //exceeding the minimum IPv6 MTU [IPv6]      
        ICMPv6PacketSize = oldPacketSize + sizeof(icmpv6Header_t) + ICMPV6_ERR_MESSAGE_OPT; 
        if(ICMPv6PacketSize > MIN_MTU)
        {
            oldPacketSize = MIN_MTU - sizeof(icmpv6Header_t) - ICMPV6_ERR_MESSAGE_OPT;
        }
        
        result = ETH_Copy(oldPacketSize);
        if(result == SUCCESS) // copy can timeout in heavy network situations like flood ping
        {
            result = IPv6_Send(pldLength);
        }
    }
    
    return result;
}

error_msg ICMPv6_ParameterProblemErrorMessage(ipv6Address_t* dstAddr, uint16_t pldLength, uint8_t code, uint32_t error_offset)
{
    //rfc4443
    error_msg result = ERROR;
    uint16_t oldPacketSize, ICMPv6PacketSize;
    
    result = IPv6_Start(dstAddr, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_PARAMETER_PROBLEM_ERROR);         //type
        ETH_Write8(code);                                   //code
        ETH_Write16(0);                                     //checksum
        ETH_Write32(error_offset);                          //Identifies the octet offset within the
                                                            //invoking packet where the error was detected.
       
        oldPacketSize = pldLength + sizeof(ipv6Header_t);   //old packet size
        ETH_SetReadPtr(Network_GetStartPosition());         //point to the start of the old packet
        
        //As much of invoking packet                 |
        //as possible without the ICMPv6 packet          +
        //exceeding the minimum IPv6 MTU [IPv6]      
        ICMPv6PacketSize = oldPacketSize + sizeof(icmpv6Header_t) + ICMPV6_ERR_MESSAGE_OPT;
        if(ICMPv6PacketSize > MIN_MTU)
        {
            oldPacketSize = MIN_MTU - sizeof(icmpv6Header_t) - ICMPV6_ERR_MESSAGE_OPT;
        }
        
        result = ETH_Copy(oldPacketSize);
        if(result == SUCCESS) // copy can timeout in heavy network situations like flood ping
        {
            result = IPv6_Send(pldLength);
        }
    }
    
    return result;    
}

/**
 * ICMP Echo Reply
 * @param icmpv6Hdr
 * @param ipv6Hdr
 * @return
 */
error_msg ICMPv6_EchoReply(ipv6Address_t* srcAddr, uint16_t pldLength)
{
    error_msg result = ERROR;

    result = IPv6_Start(srcAddr, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_ECHO_REPLY);           // Type = 129
        ETH_Write8(0);                           // Code = 0  
        
        //For computing the checksum, the checksum field is first set to zero RFC 4443
        ETH_Write16(0);                          // Checksum

        result = ETH_Copy(pldLength - sizeof(icmpv6Header_t));
        if(result == SUCCESS) // copy can timeout in heavy network situations like flood ping
        {
            result = IPv6_Send(pldLength);
        }
    }
    return result;
}


/**
 * ICMPv6 Multicast Listener Report
 * 
 *  All MLDv2 messages MUST be sent with a link-local IPv6 Source Address,
 *  an IPv6 Hop Limit of 1, and an IPv6 Router Alert option in a Hop-by-Hop Options header.
 *  MLDv2 Reports can be sent with the source address set to the unspecified address,
 *  if a valid link-local IPv6 source address has not been acquired yet for the sending interface.
 * 
 * @param 
 * @return
 */
error_msg ICMPv6_MulticastListenerReport(void)
{
    error_msg result = ERROR;

    // Destination address - all MLDv2-capable routers - FF02::16
    // Hop limit - set to TBD

    result = IPv6_Start( (ipv6Address_t*)&mcAllMLDv2Routers, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_MLD_REPORT);              // Type = 131
        ETH_Write8(0);                              // Code = 0
        ETH_Write16(0);                          // Checksum - place the seed here
        ETH_Write32(0);                             // Reserved

        // Compute Message Length
        result = IPv6_Send(ICMPV6_MLR_MESSAGE_SIZE);
    }
    
    return result;    
}


/**
 * ICMPv6 Multicast Listener Done
 * 
 *  All MLDv2 messages MUST be sent with a link-local IPv6 Source Address,
 *  an IPv6 Hop Limit of 1, and an IPv6 Router Alert option in a Hop-by-Hop Options header.
 * 
 * @param 
 * @return
 */
error_msg ICMPv6_MulticastListenerDone(void)
{
    error_msg result = ERROR;

    // Destination address - link-scope all-router multicast - FF02::2
    // Hop limit - set to 

    result = IPv6_Start( (ipv6Address_t*)&mcAllRouters, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_MLD_DONE);                // Type = 132
        ETH_Write8(0);                              // Code = 0
        ETH_Write16(0);                          // Checksum - place the seed here
        ETH_Write32(0);                             // Reserved

        result = IPv6_Send(ICMPV6_MLD_MESSAGE_SIZE);        
    }

    return result;
}


/**
 * ICMPv6 Router Solicitation
 * 
 *  RS can be sent with the source address set to the unspecified address,
 *  if a valid link-local IPv6 source address has not been acquired yet for the sending interface.
 * 
 * @param 
 * @return
 */
error_msg ICMPv6_RouterSolicitation(void)
{
    error_msg result = ERROR;
   
    // Destination address - all-router multicast - FF02::2
    // Hop limit - always set to 255
    
    // Source address - IPv6 already assigned / or unspecified

    result = IPv6_Start( (ipv6Address_t*)&mcAllRouters, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_ROUTER_SOLICITATION);     // Type = 133
        ETH_Write8(0);                              // Code = 0
        ETH_Write16(0);                             // Checksum 0 when computing
        ETH_Write32(0);                             // Reserved

        /* Valid Options: Source link-layer address
         * The link-layer address of the sender, if known. MUST NOT be included 
         * if the Source Address is the unspecified address.
         * Otherwise, it SHOULD be included on link layers that have addresses. */
        ipv6Address_t *srcAddr = (ipv6Address_t*)ipv6db_getLinkLocalAddr();
        if( in6IsAddrUnspecified(srcAddr) )
        {
            result = IPv6_Send(ICMPV6_RS_MESSAGE_SIZE);
        }
        else
        {
            // Option type: Source link-layer address 
            ETH_Write8(0x01);
            // The length of the option (including the type and length fields)
            ETH_Write8(0x01);

            // Source link-layer address 
            ETH_WriteBlock(MAC_getAddress(), sizeof(mac48Address_t));

            result = IPv6_Send(ICMPV6_RS_W_SOURCE_MAC_OPTION_MESSAGE_SIZE);
        }
 
    }

    return result;
}
   

/**
 * ICMPv6 Neighbor Solicitation
 * @param 
 * @return
 */
error_msg ICMPv6_NeighborSolicitation(ipv6Address_t* targetAddr)
{
    error_msg result = ERROR;
    
    ipv6Address_t dstAddr;
    dstAddr = solicitedNodeMulticast;
    dstAddr.s6_addr16[6] = 0xff00 | targetAddr->s6_addr16[6];
    dstAddr.s6_addr16[7] = targetAddr->s6_addr16[7];
    
    result = IPv6_Start( &dstAddr, IPV6_ICMP_TCPIP, 0);
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_NEIGHBOR_SOLICITATION);   // Type = 135 
        ETH_Write8(0);                              // Code = 0
        ETH_Write16(0);                             // Checksum zero when computing
        ETH_Write32(0);                             // Reserved
        
        // Target Address
        ETH_Write16(targetAddr->s6_addr16[0]);
        ETH_Write16(targetAddr->s6_addr16[1]);
        ETH_Write16(targetAddr->s6_addr16[2]);
        ETH_Write16(targetAddr->s6_addr16[3]);
        ETH_Write16(targetAddr->s6_addr16[4]);
        ETH_Write16(targetAddr->s6_addr16[5]);
        ETH_Write16(targetAddr->s6_addr16[6]);
        ETH_Write16(targetAddr->s6_addr16[7]);
        
        if( !NDP_IsDAD() ) 
        {
            // Option type: Source link-layer address 
            ETH_Write8(0x01);
            // The length of the option (including the type and length fields)
            ETH_Write8(0x01);

            // Source link-layer address 
            ETH_WriteBlock(MAC_getAddress(), sizeof(mac48Address_t));

            result = IPv6_Send(ICMPV6_NS_MESSAGE_SIZE);     
        }
        else
        {
            result = IPv6_Send(ICMPV6_DAD_NS_MESSAGE_SIZE);
        }
    }

    return result;
}


/**
 * ICMPv6 Neighbor Advertisement
 * @param 
 * @return
 */
error_msg ICMPv6_NeighborAdvertisement(ipv6Address_t* targetAddr)
{
    error_msg result = ERROR;

    result = IPv6_Start( (ipv6Address_t*)targetAddr, IPV6_ICMP_TCPIP, 0); 
    
    if(result == SUCCESS)
    {
        ETH_Write8(ICMPV6_NEIGHBOR_ADVERTISEMENT);  // Type = 136
        ETH_Write8(0);                              // Code = 0
        ETH_Write16(0);                             // Checksum placeholder
        ETH_Write16(0x6000);                        // Solicited and Override flags are set
        ETH_Write16(0x0000);                        // TODO: send as 32b number (ETH_Write32 will not work even with htonl)
        
        ipv6Address_t *sourceAddress;
        sourceAddress = ipv6db_getLinkLocalAddr();
        // Target Address (field is called target address as it should be copied from NS target address
        // but is actually the source address of the device)
        ETH_Write16(sourceAddress->s6_addr16[0]);
        ETH_Write16(sourceAddress->s6_addr16[1]);
        ETH_Write16(sourceAddress->s6_addr16[2]);
        ETH_Write16(sourceAddress->s6_addr16[3]);
        ETH_Write16(sourceAddress->s6_addr16[4]);
        ETH_Write16(sourceAddress->s6_addr16[5]);
        ETH_Write16(sourceAddress->s6_addr16[6]);
        ETH_Write16(sourceAddress->s6_addr16[7]);
        
        // Option type: Target link-layer address 
        ETH_Write8(0x02);
        // The length of the option (including the type and length fields)
        ETH_Write8(0x01);

        // Source link-layer address 
        ETH_WriteBlock(MAC_getAddress(), sizeof(mac48Address_t));

        result = IPv6_Send(ICMPV6_NA_MESSAGE_SIZE);        
    }

    return result;
}