/**
  IPv6 Utility Functions file

  Company:
    Microchip Technology Inc.

  File Name:
    ipv6_util.h

  Summary:
    This is the implementation file for IPv6 utility functions

  Description:
    This header file provides the API for the IPv6 Layer utility functions

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


#ifndef IPV6_UTIL_H
#define IPV6_UTIL_H

#include "ipv6.h"
#include "mac_address.h"
#include "network.h"

/** Address Testing Macros */

/*
 * netint/in.h
 */

/* Test the type of an address:
 * return true if the address is of the specified type, or false otherwise
 */
#define in6IsAddrUnspecified(a) \
    (((const uint32_t *) (a))[0] == 0                   \
     && ((const uint32_t *) (a))[1] == 0				    \
     && ((const uint32_t *) (a))[2] == 0				    \
     && ((const uint32_t *) (a))[3] == 0)

#define in6IsAddrLoopback(a) \
    (((const uint32_t *) (a))[0] == 0                   \
     && ((const uint32_t *) (a))[1] == 0				    \
     && ((const uint32_t *) (a))[2] == 0				    \
     && ((const uint32_t *) (a))[3] == htonl (1))

#define in6IsAddrMulticast(a) (((const uint8_t *) (a))[0] == 0xff)

#define in6IsAddrLinkLocal(a) \
    ((((const uint32_t *) (a))[0] & htonl (0xffc00000))	\
     == htonl (0xfe800000))

#define in6IsAddrV4Mapped(a) \
    ((((const uint32_t *) (a))[0] == 0)             \
     && (((const uint32_t *) (a))[1] == 0)			    \
     && (((const uint32_t *) (a))[2] == htonl (0xffff)))

/* Test the scope of a multicast address:
 * return true if the address is a multicast address of the specified scope
 * or false if the address is either not a multicast address
 * or not of the specified scope
 */
#define in6IsAddrMcNodeLocal(a) \
    (in6IsAddrMulticast(a)                           \
     && ((((const uint8_t *) (a))[1] & 0xf) == 0x1))

#define in6IsAddrMcLinkLocal(a) \
    (in6IsAddrMulticast(a)                           \
     && ((((const uint8_t *) (a))[1] & 0xf) == 0x2))

#define in6IsAddrMcSiteLocal(a) \
    (in6IsAddrMulticast(a)                           \
     && ((((const uint8_t *) (a))[1] & 0xf) == 0x5))

#define in6IsAddrMcOrgLocal(a) \
    (in6IsAddrMulticast(a)                           \
     && ((((const uint8_t *) (a))[1] & 0xf) == 0x8))

#define in6IsAddrMcGlobal(a) \
    (in6IsAddrMulticast(a)                           \
     && ((((const uint8_t *) (a))[1] & 0xf) == 0xe))

#define ipv6AreEthAddrEqual(a,b) \
    ((((const uint16_t *) (a))[0] == ((const uint16_t *) (b))[0])   \
     && (((const uint16_t *) (a))[1] == ((const uint16_t *) (b))[1])    \
     && (((const uint16_t *) (a))[2] == ((const uint16_t *) (b))[2]) )

#define ipv6IsEthMcAllNodes(a) \
       ((((const uint8_t *) (a))[0] == 0x33)   \
     && (((const uint8_t *) (a))[1] == 0x33)   \
     && (((const uint8_t *) (a))[2] == 0x00)   \
     && (((const uint8_t *) (a))[3] == 0x00)   \
     && (((const uint8_t *) (a))[4] == 0x00)   \
     && (((const uint8_t *) (a))[5] == 0x01))

#define ipv6MakeAddr(a0, a1, a2, a3, a4, a5, a6, a7) \
    { htons(a1), htons(a0), \
      htons(a3), htons(a2), \
      htons(a5), htons(a4),\
      htons(a7), htons(a6)}

#define ethAddrCopy(dst, src) \
    ((((uint8_t *) (dst))[0] = ((const uint8_t *) (src))[0])   \
     && (((uint8_t *) (dst))[1] = ((const uint8_t *) (src))[1])    \
     && (((uint8_t *) (dst))[2] = ((const uint8_t *) (src))[2])    \
     && (((uint8_t *) (dst))[3] = ((const uint8_t *) (src))[3])    \
     && (((uint8_t *) (dst))[4] = ((const uint8_t *) (src))[4])    \
     && (((uint8_t *) (dst))[5] = ((const uint8_t *) (src))[5]) )

/** String conversion functions */
#if 0
error_msg makeStrToIpv6Address(const char *str, ipv6Address_t *addr);
error_msg makeIpv6AddressToStr(const ipv6Address_t *addr, char *strAddr);
#endif

bool in6AreAddrEqual(ipv6Address_t*, ipv6Address_t*);

void in6CopyAddr(ipv6Address_t *dst, ipv6Address_t *src);

/** Hash functions */
uint16_t makeIpv6ToHash(ipv6Address_t *addr);

/** Filter functions */
bool ipv6FilterOnMulticastEth(mac48Address_t* macAddr);

uint8_t ipv6GenerateRandom(void);
bool ipv6IsAddrOnLink(ipv6Address_t* dstAddr);

bool isSolicitedNodeMulticastAddress(ipv6Address_t* dstAddr);

void ntohipv6(ipv6Address_t* hAddr, ipv6Address_t* nAddr);

bool isLinkLocal(ipv6Address_t* dstAddr);

#endif  /* IPV6_UTIL_H */
