/**
  IPv6 implementation

  Company:
    Microchip Technology Inc.

  File Name:
    ipv6.c

  Summary:
    This is the implementation of IP version 6 stack

  Description:
    This source file provides the implementation of the API for the IPv6 stack.

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
#include <stdbool.h>

#include "tcpip_config.h"
#include "tcpip_types.h"
#include "tcpip_if.h"
#include "network.h"
#include "ipv6.h"
#include "ndp.h"
#include "icmpv6.h"
#include "ethernet_driver.h"
#include "ipv6_database.h"
#include "ipv6_util.h"
#include "mac_address.h"
#include "udpv6.h"
#include "tcpv6.h"

#ifdef ENABLE_NETWORK_DEBUG
#include "log.h"
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest)  
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

/**
  Section: Private macro definitions
*/

/* IP state machine */
#define IP_NO_LOCAL_ADDR            0
#define IP_TENTATIVE                1
#define IP_VALID                    2
#define IP_INVALID                  3

#define IPV6_FLOW_LABEL_MASK        (0x000FFFFF)

/**
  Section: Private attributes
*/
static uint8_t          ipState;
static ipv6PseudoHeader_t checksumHdr;

/**
  Section: Private function prototypes
*/
uint32_t IPv6_PseudoHeaderChecksum(uint16_t pldLen);

extern uint8_t ipv6db_getAddressIndex(void);
extern void TCP_Recv(in6Addr_t*, uint16_t);

/**
  Section: Public functions
*/

void IPv6_Init(void)
{
    ipv6db_init();
    ipState = IP_NO_LOCAL_ADDR;

    return;
}


void IPv6_Packet(void)
{
    uint32_t cksm = 0;
    ipv6HeaderRx_t packetHeader;
    uint16_t pldLength;

    ETH_ReadBlock((char *)&packetHeader, sizeof(ipv6HeaderRx_t));
    
    pldLength = ntohs(packetHeader.payloadLength);

    for( uint8_t i = 0; i < 8; i++)
    {
        checksumHdr.srcAddress[i] = (uint16_t)(packetHeader.srcAddress[i << 1] << 8) | packetHeader.srcAddress[(i << 1) + 1];
        checksumHdr.dstAddress[i] = (uint16_t)(packetHeader.dstAddress[i << 1] << 8) | packetHeader.dstAddress[(i << 1) + 1];
    }
    checksumHdr.nextHeader = packetHeader.nextHeader & 0x000000FF;
    checksumHdr.length = 0;

    //compute next protocol checksum
    cksm = IPv6_PseudoHeaderChecksum(pldLength);
    cksm = ETH_RxComputeChecksum(pldLength, cksm);
    if(cksm != VALID_RX_CHECKSUM)
    {
        return /*IPV6_CHECKSUM_FAILS*/;
    }

    ntohipv6(packetHeader.srcAddress, packetHeader.srcAddress);
    ntohipv6(packetHeader.dstAddress, packetHeader.dstAddress);

    /*  
     * RFC 6085
     * 3.  Receiving IPv6 Multicast Packets
     * An IPv6 node receiving an IPv6 packet with a multicast destination
     * address and an Ethernet link-layer unicast address MUST NOT drop the
     * packet as a result of the use of this form of address mapping. 
     */
    
    switch( (ipProtocolNumbers)packetHeader.nextHeader ) {
        case IPV6_ICMP_TCPIP:
            ICMPv6_Receive((ipv6HeaderRx_t*)&packetHeader);
            break;

        case TCP_TCPIP:
            logMsg("RX TCP Packet", LOG_INFO, (LOG_DEST_CONSOLE));
            NeighborCache_RenewAddress((ipv6Address_t*)packetHeader.srcAddress);
            TCP_Recv((ipv6Address_t*)packetHeader.srcAddress , pldLength);
            break;

        case UDP_TCPIP:
            logMsg("RX UDP Packet", LOG_INFO, (LOG_DEST_CONSOLE));
            NeighborCache_RenewAddress((ipv6Address_t*)packetHeader.srcAddress);
            UDPv6_Receive((ipv6Address_t*)packetHeader.srcAddress); 
            break;

        case HOPOPT_TCPIP:            // IPv6 Hop-by-Hop Option    [RFC2460]
            /* Fallthrough */
        case IPV6_Route_TCPIP:        // Routing Header for IPv6    [Steve_Deering]
            /* Fallthrough */
        case IPV6_Frag_TCPIP:         // Fragment Header for IPv6    [Steve_Deering]
            /* Fallthrough */
        case ESP_TCPIP:               // Encap Security Payload    [RFC4303]
            /* Fallthrough */
        case AH_TCPIP:                // Authentication Header    [RFC4302]:
            /* Fallthrough */
        case IPV6_NoNxt_TCPIP:        // No Next Header for IPv6    [RFC2460]
            /* Fallthrough */
        case IPV6_Opts_TCPIP:         // Destination Options for IPv6    [RFC2460]
            // unsupported next header
//            break;

        default:
            ETH_Dump(pldLength);
            break;
    }

    return /* SUCCESS*/;
}

error_msg IPv6_Start(ipv6Address_t* dstAddr, ipProtocolNumbers protocol, uint32_t flowLabel)
{
    uint8_t         srcAddrIdx;
    ipv6Address_t   srcAddr;
    addrState_t     srcAddrState;
    uint16_t        flowLbl;
    error_msg       result = ERROR;
    mac48Address_t  ethNextHopAddress;
    mac48Address_t  *gatewayMAC;

    // Check if we have a valid destination IP address and it is not the loopback address
    if( in6IsAddrUnspecified(dstAddr) || in6IsAddrLoopback(dstAddr) )
    {
        return ERROR;
    }
    
    result = NeighborCache_Lookup(dstAddr, &ethNextHopAddress);
    
    switch( result ) 
    {
        case SUCCESS:
            srcAddrIdx = LINKLOCAL_ADDR;
            break;
            
        case GLOBAL_DESTINATION:
            srcAddrIdx = ipv6db_getAddressIndex();
            break;
            
        case MAC_NOT_FOUND:
            if( ipv6IsAddrOnLink(dstAddr) )
            {
                NeighborCache_CreateIncomplete(dstAddr);
                return ADDRESS_RESOLUTION;
            }
            else
            {
                gatewayMAC = ipv6db_getRouterMAC();
                if( gatewayMAC )
                {
                    NeighborCache_CreateGlobal(dstAddr, gatewayMAC);
                    srcAddrIdx = ipv6db_getAddressIndex();
                }
                else
                {
                    return NO_GATEWAY;
                }
            }
            break;
            
        default:
            return result;
    }

    srcAddr = *ipv6db_getAddress(srcAddrIdx);
    srcAddrState = ipv6db_getAddressState(srcAddrIdx);
    
    switch( srcAddrState )
    {
        case TENTATIVE:
            // Auto configuration in progress, allow packet only if it is DAD NS
            if( NDP_IsDAD() )
            {
                srcAddr.s6_addr32[0] = 0;
                srcAddr.s6_addr32[1] = 0;
                srcAddr.s6_addr32[2] = 0;
                srcAddr.s6_addr32[3] = 0;
                break;
            }
            else
            {
                return ERROR;
            }
            
        case PREFERRED:
            // Do nothing and allow packet.
            break;
            
        case DEPRECATED:
            // Loop Opened TCP sockets to determine if the there is an opened
            // connection w/ dstAddr. If yes allow this packet else drop.
            return ERROR;
            
        case INVALID:
            // Link-local address has infinite lifetime so once set it cannot be invalid
            // Hence an invalid address must be a global unique one. Invalid due to 
            // expiration. Auto configuration Manage will detect this.
            return IPV6_LOCAL_ADDR_INVALID;
        
        default:
            return ERROR;
    }
    
    result = ETH_WriteStart(&ethNextHopAddress, ETHERTYPE_IPV6);

    if(result == SUCCESS)
    {
        flowLbl = (flowLabel & IPV6_FLOW_LABEL_MASK) >> 16;
        ETH_Write16(0x6000 | flowLbl);      // version, traffic class, flow label (first nibble))

        flowLbl = flowLabel & IPV6_FLOW_LABEL_MASK;
        ETH_Write16(flowLbl);                     // flow label

        ETH_Write16(0);                     // payload length
        ETH_Write8(protocol);               // next header
        ETH_Write8(IPv6_UNICAST_HOP_LIMIT); // hop limit

        // Source Address
        ETH_Write16(srcAddr.s6_addr16[0]);
        ETH_Write16(srcAddr.s6_addr16[1]);
        ETH_Write16(srcAddr.s6_addr16[2]);
        ETH_Write16(srcAddr.s6_addr16[3]);
        ETH_Write16(srcAddr.s6_addr16[4]);
        ETH_Write16(srcAddr.s6_addr16[5]);
        ETH_Write16(srcAddr.s6_addr16[6]);
        ETH_Write16(srcAddr.s6_addr16[7]);
        
        // Destination Address
        ETH_Write16(dstAddr->s6_addr16[0]);
        ETH_Write16(dstAddr->s6_addr16[1]);
        ETH_Write16(dstAddr->s6_addr16[2]);
        ETH_Write16(dstAddr->s6_addr16[3]);
        ETH_Write16(dstAddr->s6_addr16[4]);
        ETH_Write16(dstAddr->s6_addr16[5]);
        ETH_Write16(dstAddr->s6_addr16[6]);
        ETH_Write16(dstAddr->s6_addr16[7]);

        for( uint8_t i = 0; i < 8; i++)
        {
            checksumHdr.srcAddress[i] = srcAddr.s6_addr16[i];
            checksumHdr.dstAddress[i] = dstAddr->s6_addr16[i];
        }
        checksumHdr.nextHeader = protocol & 0x000000FF;
        checksumHdr.length = 0;
    }

    // Save EWRP
    ETH_SaveWRPT();
    
    return result;
}

uint32_t IPv6_PseudoHeaderChecksum(uint16_t pldLen)
{   
    uint32_t cksm = 0;
    uint16_t *v = (uint16_t*)&checksumHdr;
    uint8_t len = sizeof(ipv6PseudoHeader_t) / 2;
 
    checksumHdr.length = pldLen;
    checksumHdr.nextHeader &= 0x000000FF;
    
    while(len) {
        cksm += *(v + --len);
    }

    // Return the resulting checksum
    return cksm;
}


error_msg IPv6_Send(uint16_t pldLen)
{
    uint16_t totalLength = ETH_GetByteCount();
    uint16_t cksm = 0;
    uint16_t cksmOffset = sizeof(ethernetFrame_t) + sizeof(ipv6Header_t);
    error_msg result = SUCCESS;
    uint32_t seed = IPv6_PseudoHeaderChecksum(pldLen);
    uint8_t optionsHeadersLen = 0; // Reserved for future implementation - used for Extension headers offsetting
    
    totalLength = htons(totalLength);
    
    //Insert IPv6 Total Length
    ETH_Insert((char *)&totalLength, 2, sizeof(ethernetFrame_t) + offsetof(ipv6Header_t, payloadLength) - 1);
    
    /* If there are extension headers, add here extra offset */
    
    switch( checksumHdr.nextHeader )
    {
        case TCP_TCPIP:
            cksmOffset += 16;
            break;

        case UDP_TCPIP:
            cksmOffset += 6;
            break;

        case IPV6_ICMP_TCPIP:
            cksmOffset += 2;
            break;

        default:
            result = ERROR;
            break;
    }
   
//    The checksum is the 16-bit one's complement of the one's complement
//    sum of the entire ICMPv6 message, starting with the ICMPv6 message
//    type field, and prepended with a "pseudo-header" of IPv6 header
//    fields, as specified in [IPv6, Section 8.1].  The Next Header value
//    used in the pseudo-header is 58.  (The inclusion of a pseudo-header
//    in the ICMPv6 checksum is a change from IPv4; see [IPv6] for the
//    rationale for this change.)
    cksm = ETH_TxComputeChecksum( 
            sizeof(ethernetFrame_t) + sizeof(ipv6Header_t) + optionsHeadersLen - 1, 
            pldLen, 
            seed);

    //Insert specific protocol header checksum
    ETH_Insert((char *)&cksm, 2, cksmOffset - 1);

    result = ETH_Send();

    return result;
}

// Static link-local address
error_msg IPv6_SetLinkLocalAddress(ipv6Address_t* addr)
{
    if( addr->s6.s6_u16[0] != 0xfe80 || addr->s6.s6_u16[1] != 0 ||
        addr->s6.s6_u16[2] != 0 || addr->s6.s6_u16[3] != 0 )
    {
        return IPV6_LOCAL_ADDR_INVALID;
    }

    ipv6db_setLinkLocalAddr(addr);
    ipState = IP_VALID;
    
    return SUCCESS;
}

/**
 * The Start Listening operation is equivalent to: 
 * IPv6MulticastListen ( socket, interface, IPv6 multicast address, EXCLUDE, {} )
 * and the Stop Listening operation is equivalent to:
 * IPv6MulticastListen ( socket, interface, IPv6 multicast address, INCLUDE, {} )
 * where {} is an empty source list.
 */


///TODO: Check RFC 3678 for examples

//error_msg IPv6_MulticastListen ( socket, interface, IPv6 multicast address, filter mode, source list )
