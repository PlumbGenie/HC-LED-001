/**
  TCP IP Generic interface header file

  Company:
    Microchip Technology Inc.

  File Name:
    tcpip_if.h

  Summary:
    This is the header file for the tcpip_if.c

  Description:
    This header file provides the generic API for the TCP and UDP

 */

/*

©  [2017] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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

/*
 * File:   tcpip_if.h
 * Author: virgil
 *
 * Created on February 14, 2017
 */

#ifndef TCPIP_IF_H
#define TCPIP_IF_H

/**
  Section: Included Files
*/
#include <stdbool.h>
#include "tcpip_types.h"
#include "ethernet_driver.h"
#include "mac_address.h"


/*
 * Macros
 */

/*
 * Types
 */

typedef uint16_t sa_family_t;


typedef struct {
    sa_family_t         in6_family;          /* AF_INET6 */
    uint16_t            in6_port;            /* transport layer port # */
    uint32_t            in6_flowinfo;        /* IPv6 flow information */
    in6Addr_t           in6_addr;            /* IPv6 address */
    uint32_t            in6_scope_id;        /* set of interfaces for a scope */
}sockaddr_in6_t;


typedef struct inAddrHandle_
{
    struct inAddrHandle_ *pNext;
    sa_family_t         in_family;          /* AF_INET / AF_INET6 */
    union
    {
        inAddr_t  ipv4Addr;
        in6Addr_t ipv6Addr;
    };
    mac48Address_t macAddr;
    uint8_t buffer[8];
}inAddrHandle_t;

bool makeIPv6Addr(inAddrHandle_t const* addrHdl, in6Addr_t* ipv6ddr);
bool makeIPv4Addr(inAddrHandle_t const* addrHdl, inAddr_t* ipv4ddr);

#endif /* TCPIP_IF_H */

