/**
  IPv6 Protocol header file

  Company:
    Microchip Technology Inc.

  File Name:
    ipv6.h

  Summary:
    This is the header file for ipv6.c

  Description:
    This header file provides the API for the IPv6 protocol.

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


#ifndef IPV6_H
#define IPV6_H

/**
  Section: Included Files
*/
#include <stdbool.h>
#include "tcpip_types.h"

/**
  Section: Macro Declarations
*/
#define IPV6_MULTICAST_SCOPE_INTERFACE_LOCAL            (0x1)
#define IPV6_MULTICAST_SCOPE_LINK_LOCAL                 (0x2)
#define IPV6_MULTICAST_SCOPE_ADMIN_LOCAL                (0x4)
#define IPV6_MULTICAST_SCOPE_SITE_LOCAL                 (0x5)
#define IPV6_MULTICAST_SCOPE_ORG_LOCAL                  (0x5)
#define IPV6_MULTICAST_SCOPE_GLOBAL                     (0xe)

/* RFC4291 */
#define IPV6_MULTICAST_FLAG_TRANSIENT                   (0100b)

/* RFC3306 */
#define IPV6_MULTICAST_FLAG_PREFIX_BASED                (0010b)

/* RFC3956 */
#define IPV6_MULTICAST_FLAG_RENDEZVOUS_POINT            (0001b)


#define IPV6_UNIVERSAL_LOCAL_FLAG                       (0x02)

#define VALID_RX_CHECKSUM                               (0)
/**
  Section: Data Types Definitions
*/
typedef in6Addr_t ipv6Address_t;

/**
  Section: Enumeration Definition
*/

/* IPv4-Mapped IPv6 Address
| 80 bits | 16 | 32 bits |
+--------------------------------------+----+---------------------+
|0000..............................0000|FFFF|     IPv4 address    |
+--------------------------------------+----+---------------------+ */
const ipv6Address_t ipv4MappedIpv6          = {0, 0, 0, 0, 0, 0xffff, 0, 0};        // ::/96

const ipv6Address_t unspecified             = {0, 0, 0, 0, 0, 0, 0, 0};             // ::/128
const ipv6Address_t loopback                = {0, 0, 0, 0, 0, 0, 0, 1};             // ::1/128
const ipv6Address_t teredo                  = {0x2001, 0, 0, 0, 0, 0, 0, 0};        // 2001::/32
const ipv6Address_t documentation           = {0x2001, 0xdb8, 0, 0, 0, 0, 0, 0};    // 2001:db8::/32
const ipv6Address_t _6to4                   = {0x2002, 0, 0, 0, 0, 0, 0, 0};        // 2002::/16
const ipv6Address_t uniqueLocal             = {0xfc00, 0, 0, 0, 0, 0, 0, 0};        // fc00::/7

/* Multicast Addresses
|    8   | 4  |  4 |                 112 bits                    |
+------ -+----+----+---------------------------------------------+
|11111111|flgs|scop|                 group ID                    |
+--------+----+----+---------------------------------------------+ */
const ipv6Address_t multicast               = {0xff00, 0, 0, 0, 0, 0, 0, 0};       // ff00::/8
const ipv6Address_t solicitedNodeMulticast  = {0xff02, 0, 0, 0, 0, 1, 0xff00, 0};  // ff02::1:FF00:0000/104

const ipv6Address_t mcAllNodes              = {0xff02, 0, 0, 0, 0, 0, 0, 0x0001};       // ff02::1
const ipv6Address_t mcAllRouters            = {0xff02, 0, 0, 0, 0, 0, 0, 0x0002};       // ff02::2
const ipv6Address_t mcAllMLDv2Routers       = {0xff02, 0, 0, 0, 0, 0, 0, 0x0016};  // ff02::16
const ipv6Address_t mcDhcpv6Servers         = {0xff02, 0, 0, 0, 0, 0, 1, 2};       // ff02::1:2
const ipv6Address_t mcDhcpv6ServersLocal    = {0xff05, 0, 0, 0, 0, 0, 1, 3};       // ff05::1:3
const ipv6Address_t mcNtp                   = {0xff00, 0, 0, 0, 0, 0, 0, 0x0101};     // ff0x::101


/* Link-Local IPv6 Unicast Addresses
| 10 bits  |         54 bits         |          64 bits           |
+----------+-------------------------+----------------------------+
|1111111010|            0            |        interface ID        |
+----------+-------------------------+----------------------------+ */
const ipv6Address_t linkLocal               = {0xfe80, 0, 0, 0, 0, 0, 0, 0};      // fe80::/10


/**
  Section: Public Interface Functions
 */

/** IPv6 Initialization.
 *
 */
void IPv6_Init(void);

/** Receives IPv6 Packet.
 * This function reads the IPv6 header and filters the upper layer protocols.
 *
 * @return
 */
void IPv6_Packet(void);

/** Starts the IPv6 Packet.
 * This function starts the Ethernet Packet and writes the IPv6 header.
 * Initially Checksum and Payload length are set to '0'
 *
 * @param dest_address
 *          128-bit Destination IPv6 Address.
 *
 * @param protocol
 *          Protocol Number.
 *
 * @param flowLabel
 *          Flow Label - sequence or flow of IPv6 packets from the source
 *
 * @return
 *      An error code if something goes wrong. For the possible errors please,
 *      see the error description in tcpip_types.h
 */
error_msg IPv6_Start(ipv6Address_t* dstAddr, ipProtocolNumbers protocol, uint32_t flowLabel);

/** Send IPv6 Packet.
 * This function inserts the total length of IPv6 packet, computes and inserts the IPv6 header checksum.
 *
 * @param payload_length
 *      Data length of the transport packet.
 *
 * @return
 *      1 - IP Packet was sent Successfully
 * @return
 *      (!=1) - IP Packet sent Fails
 */
error_msg IPv6_Send(uint16_t pldLength);

/** This function sets the Link Local IPv6 Address
 *
 * @param address
 *          128-bit Link Local IPv6 Address set by the application
 *
 * @return
 *      An error code if something goes wrong. For the possible errors please,
 *      see the error description in tcpip_types.h
 */
error_msg IPv6_SetLinkLocalAddress(ipv6Address_t* addr);

/** This function sets the Global Unicast IPv6 Address
 *
 * @param address
 *          128-bit Global Unicast IPv6 Address set by the application
 *
 * @return
 *      An error code if something goes wrong. For the possible errors please,
 *      see the error description in tcpip_types.h
 */
error_msg IPv6_SetGlobalUnicastAddress(ipv6Address_t* addr);

//error_msg IPv6_MulticastListen ( socket, interface, IPv6 multicast address, filter mode, source list )


#endif  /* IPV6_H */