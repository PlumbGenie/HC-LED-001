/**
  IPv6 Database implementation

  Company:
    Microchip Technology Inc.

  File Name:
    ipv6_database.c

  Summary:
    This is the implementation file for ipv6_database.c

  Description:
    This source file provides the implementation of the IPv6 database.

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

#include <stdint.h>
#include <stdio.h>

#include "network.h"
#include "tcpip_types.h"
#include "tcpip_config.h"
#include "ethernet_driver.h"
#include "ipv6_database.h"
#include "ipv6.h"
#include "ndp.h"

typedef struct ipv6_dbEntry {
    ipv6Address_t       addr;                       // IPv6 address
    uint16_t            addrHash;                   // djb2 hash value of the addr
    mac48Address_t      nextHopAddr;                // MAC Address of next-hop address
    addrState_t         state;
} ipv6_dbEntry_t;

ipv6_db_t ipv6_database_info;

/* NOTE: A link-local address has an infinite preferred and valid lifetime; it
is never timed out. */

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

void ipv6db_init(void)
{
    uint8_t* ptr = (uint8_t*)&ipv6_database_info;
    
    for( uint8_t index = 0; index < sizeof(ipv6_database_info); index++ )
    {
        ptr[index] = 0;
    }
    
    for(uint8_t index = 0; index < MAX_UNICAST; index++)
    {
        ipv6db_setAddressState(index, INVALID);
    }
    ipv6_database_info.routerHopLimit = IPv6_UNICAST_HOP_LIMIT;
}

ipv6Address_t* ipv6db_getAddress(uint8_t index)
{
#ifdef VALIDATE_SIZE
    if( index < MAX_UNICAST )
    {
        return &ipv6_database_info.ipv6_unicastAddress[index].addr;
    }
    else
    {
        return NULL;
    }
#else
    return &ipv6_database_info.ipv6_unicastAddress[index].addr;
#endif
}

ipv6Address_t* ipv6db_getMulticastAddress(uint8_t index)
{
#ifdef VALIDATE_SIZE
    if( index < MAX_MULTICAST )
    {
        return &ipv6_database_info.ipv6_multicastAddress[index];
    }
    else
    {
        return NULL;
    }
#else
    return &ipv6_database_info.ipv6_multicastAddress[index];
#endif
}

addrState_t ipv6db_getAddressState(uint8_t index)
{
#ifdef VALIDATE_SIZE
    if( index < MAX_UNICAST )
    {
        return ipv6_database_info.ipv6_unicastAddress[index].state;
    }
    else
    {
        return INVALID;
    }
#else
    return ipv6_database_info.ipv6_unicastAddress[index].state;
#endif
}

uint8_t ipv6db_getAddressIndex(void)
{
#define entryState ipv6_database_info.ipv6_unicastAddress[index].state
    for( uint8_t index = GLOBAL_ADDRESS_1; index < MAX_UNICAST; index++)
    {
        if( entryState & VALID_MASK )
        {
            return index;
        }
    }
#undef entryState
}

mac48Address_t* ipv6db_getRouterMAC(void)
{
    if( ipv6_database_info.routerLifetime > 0)
    {
        return &ipv6_database_info.routerMacAddress;
    }
    else
    {
        return NULL;
    }
}

ipv6Address_t* ipv6db_getDNS(void) { return &ipv6_database_info.ipv6_dnsAddress; }
ipv6Address_t* ipv6db_getRouter(void) { return &ipv6_database_info.ipv6_routerAddress; }
ipv6Address_t* ipv6db_getNTP(void) { return &ipv6_database_info.ipv6_ntpAddress; }
ipv6Address_t* ipv6db_getTFTP(void) { return &ipv6_database_info.ipv6_tftpAddress; }

void ipv6db_setAddress(uint8_t index, const ipv6Address_t *addr)
{
#ifdef VALIDATE_SIZE
    if( index < MAX_UNICAST )
#endif
    {
        ipv6_database_info.ipv6_unicastAddress[index].addr.s6_addr32[0] = addr->s6_addr32[0];
        ipv6_database_info.ipv6_unicastAddress[index].addr.s6_addr32[1] = addr->s6_addr32[1];
        ipv6_database_info.ipv6_unicastAddress[index].addr.s6_addr32[2] = addr->s6_addr32[2];
        ipv6_database_info.ipv6_unicastAddress[index].addr.s6_addr32[3] = addr->s6_addr32[3];
        ipv6_database_info.ipv6_unicastAddress[index].state = TENTATIVE;
    }
}

void ipv6db_setAddressState(uint8_t index, addrState_t newState)
{
#ifdef VALIDATE_SIZE
    if( index < MAX_UNICAST )
#endif
    {
        ipv6_database_info.ipv6_unicastAddress[index].state = newState;
    }
}

void ipv6db_setMulticastAddress(uint8_t index, const ipv6Address_t* addr)
{
#ifdef VALIDATE_SIZE
    if( index < MAX_MULTICAST )
#endif
    {
        ipv6_database_info.ipv6_multicastAddress[index].s6_addr32[0] = (addr->s6_addr32[0]);
        ipv6_database_info.ipv6_multicastAddress[index].s6_addr32[1] = (addr->s6_addr32[1]);
        ipv6_database_info.ipv6_multicastAddress[index].s6_addr32[2] = (addr->s6_addr32[2]);
        ipv6_database_info.ipv6_multicastAddress[index].s6_addr32[3] = (addr->s6_addr32[3]);
    }
}

void ipv6db_setDNS(const ipv6Address_t* addr)
{
    ipv6_database_info.ipv6_dnsAddress.s6_addr32[0] = (addr->s6_addr32[0]);
    ipv6_database_info.ipv6_dnsAddress.s6_addr32[1] = (addr->s6_addr32[1]);
    ipv6_database_info.ipv6_dnsAddress.s6_addr32[2] = (addr->s6_addr32[2]);
    ipv6_database_info.ipv6_dnsAddress.s6_addr32[3] = (addr->s6_addr32[3]);
}

void ipv6db_setPrefix(uint8_t index, const ipv6Address_t *prefix, uint8_t prefixLength)
{
    for(uint8_t i = 0; i < prefixLength; i++ )
    {
        ipv6_database_info.ipv6_unicastAddress[index].addr.s6_addr[i] = prefix->s6_addr[i];
    }
}

void ipv6db_setRouter(ipv6Address_t* addr)
{
    ipv6_database_info.ipv6_routerAddress.s6_addr32[0] = (addr->s6_addr32[0]);
    ipv6_database_info.ipv6_routerAddress.s6_addr32[1] = (addr->s6_addr32[1]);
    ipv6_database_info.ipv6_routerAddress.s6_addr32[2] = (addr->s6_addr32[2]);
    ipv6_database_info.ipv6_routerAddress.s6_addr32[3] = (addr->s6_addr32[3]);
}

void ipv6db_setRouterMAC(mac48Address_t* macAddr)
{
    ipv6_database_info.routerMacAddress.mac_array[0] = macAddr->mac_array[0];
    ipv6_database_info.routerMacAddress.mac_array[1] = macAddr->mac_array[1];
    ipv6_database_info.routerMacAddress.mac_array[2] = macAddr->mac_array[2];
    ipv6_database_info.routerMacAddress.mac_array[3] = macAddr->mac_array[3];
    ipv6_database_info.routerMacAddress.mac_array[4] = macAddr->mac_array[4];
    ipv6_database_info.routerMacAddress.mac_array[5] = macAddr->mac_array[5];
}

void ipv6db_setGateway(const ipv6Address_t* addr)
{
    ipv6_database_info.ipv6_gatewayAddress.s6_addr32[0] = (addr->s6_addr32[0]);
    ipv6_database_info.ipv6_gatewayAddress.s6_addr32[1] = (addr->s6_addr32[1]);
    ipv6_database_info.ipv6_gatewayAddress.s6_addr32[2] = (addr->s6_addr32[2]);
    ipv6_database_info.ipv6_gatewayAddress.s6_addr32[3] = (addr->s6_addr32[3]);
}

void ipv6db_setNTP(const ipv6Address_t* addr)
{
    ipv6_database_info.ipv6_ntpAddress.s6_addr32[0] = (addr->s6_addr32[0]);
    ipv6_database_info.ipv6_ntpAddress.s6_addr32[1] = (addr->s6_addr32[1]);
    ipv6_database_info.ipv6_ntpAddress.s6_addr32[2] = (addr->s6_addr32[2]);
    ipv6_database_info.ipv6_ntpAddress.s6_addr32[3] = (addr->s6_addr32[3]);
}

void ipv6db_Update(void)
{
    if( ipv6_database_info.routerLifetime > 0)
    {
        ipv6_database_info.routerLifetime--;
    }
    
#define entryLifetime ipv6_database_info.ipv6_unicastAddress[index].validLifetime
    for( uint8_t index = GLOBAL_ADDRESS_1; index < MAX_UNICAST; index++)
    {
        if( (entryLifetime > 0) && (entryLifetime != 0xFFFFFFFF) )
        {
            entryLifetime--;
            if(entryLifetime == 0)
            {
                ipv6_database_info.ipv6_unicastAddress[index].state = INVALID;
            }
        }
    }
#undef entryLifetime
}
