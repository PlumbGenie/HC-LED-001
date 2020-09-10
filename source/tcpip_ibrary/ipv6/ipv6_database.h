/**
  IPv6 Database Header file

  Company:
    Microchip Technology Inc.

  File Name:
    ipv6_database.h

  Summary:
    This is the header file for ipv6_database.c

  Description:
    This header file provides the API for the ICMPv6 protocol

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


#ifndef IPV6_DATABASE_H
#define IPV6_DATABASE_H

#include <stdbool.h>
#include "ipv6.h"
#include "mac_address.h"

#define LINKLOCAL_ADDR          0
#define GLOBAL_ADDRESS_1        1
#define GLOBAL_ADDRESS_2        2

#define MAX_UNICAST             3
#define MAX_MULTICAST           2

#define MAX_DNS                 2
#define MAX_NTP                 2


typedef enum
{
    TENTATIVE   = 0,
    INVALID     = 1,
    PREFERRED   = 2,
    DEPRECATED  = 3
} addrState_t;

#define VALID_MASK              (2)

typedef struct
{
        // LINKLOCAL_ADDR is mandatory
    struct {
        ipv6Address_t   addr;
        uint32_t        validLifetime;
        addrState_t     state;
        uint8_t         prefixLength;
    } ipv6_unicastAddress[MAX_UNICAST];
        // Optional
    ipv6Address_t   ipv6_multicastAddress[MAX_MULTICAST];

    // Link addresses
    ipv6Address_t   ipv6_dnsAddress; //[MAX_DNS]; // allow a primary & secondary DNS
    ipv6Address_t   ipv6_routerAddress;
    ipv6Address_t   ipv6_gatewayAddress;
    ipv6Address_t   ipv6_ntpAddress;
    ipv6Address_t   ipv6_tftpAddress;

    uint32_t        reachableTime;
    uint16_t        routerLifetime;             // duration, in seconds, that the router should be used as the default gateway
    uint8_t         retransTimer;
    uint8_t         routerHopLimit;
    mac48Address_t  routerMacAddress;
} ipv6_db_t;

extern ipv6_db_t ipv6_database_info;

#define ipv6db_setLinkLocalAddr(addr)                       ipv6db_setAddress(LINKLOCAL_ADDR, addr)
#define ipv6db_setGlobalUnicastAddr(index, addr)            ipv6db_setAddress(index, addr)
#define ipv6db_getLinkLocalAddr()                           ipv6db_getAddress(LINKLOCAL_ADDR)
#define ipv6db_getGlobalUnicastAddr(index)                  ipv6db_getAddress(index)
#define ipv6db_getLinkLocalAddrState()                      ipv6db_getAddressState(LINKLOCAL_ADDR)
#define ipv6db_getGlobalUnicastAddrState(index)             ipv6db_getAddressState(index)

void ipv6db_init(void);

/** Get functions */
ipv6Address_t* ipv6db_getAddress(uint8_t index);
ipv6Address_t* ipv6db_getMulticastAddress(uint8_t index);
addrState_t    ipv6db_getAddressState(uint8_t index);

ipv6Address_t* ipv6db_getDNS(void);
ipv6Address_t* ipv6db_getRouter(void);
mac48Address_t* ipv6db_getRouterMAC(void);
ipv6Address_t* ipv6db_getNTP(void);

/** Set functions */
void ipv6db_setAddress(uint8_t index, const ipv6Address_t *addr);
void ipv6db_setMulticastAddress(uint8_t index, const ipv6Address_t *addr);
void ipv6db_setAddressState(uint8_t index, addrState_t newState);

void ipv6db_setDNS(const ipv6Address_t *dnsAddr);
void ipv6db_setPrefix(uint8_t index, const ipv6Address_t *prefix, uint8_t prefixLength);
void ipv6db_setRouter(ipv6Address_t *routerAddr);
void ipv6db_setRouterMAC(mac48Address_t* macAddr);
void ipv6db_setGateway(const ipv6Address_t *gatewayAddr);
void ipv6db_setNTP(const ipv6Address_t* ntpAddr);
void ipv6db_Update(void);

#endif  /* IPV6_DATABASE_H */
