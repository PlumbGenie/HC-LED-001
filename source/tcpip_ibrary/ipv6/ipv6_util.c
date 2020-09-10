/**
  IPv6 Utility Functions

  Company:
    Microchip Technology Inc.

  File Name:
    ipv6_util.c

  Summary:
    This is the implementation file for IPv6 utility functions

  Description:
    This source file provides the implementation of the IPv6 utility functions

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
#include <string.h>
#include <stdlib.h>

#include "ipv6.h"
#include "ipv6_database.h"
#include "ipv6_util.h"
#include "mac_address.h"

/**
  Section: Private function prototypes
*/
static bool ipv6IsEthMcSolicitedNode(mac48Address_t* macAddr);

/**
  Section: Public functions
*/
#if 0
error_msg makeStrToIpv6Address(const char *str, ipv6Address_t *addr)
{
#warning Work In Progress
    error_msg result = SUCCESS;
    char *pch = NULL;
    uint8_t x = 8;

    pch = strtok(str, "::");

    if( pch != NULL )
    {
        strtol(pch, NULL, 16);

        ((uint16_t *)&addr)[x]= atoi(pch);
        x--;
        pch = strtok (NULL,":");
    }

    return result;
}

/**
 * inet_ntop
 */

error_msg makeIpv6AddressToStr(const ipv6Address_t *addr, char *strAddr)
{
#warning Work In Progress
    error_msg result = SUCCESS;
    char hextet[5];
    uint8_t strLen = 0;
    uint8_t i = 0;
    uint8_t firstAllZeroHextet = 8;
    uint8_t lastAllZeroHextet = 8;

    while( i < 8 )
    {
        if( addr->s6_addr16[i] == 0 )
        {
            firstAllZeroHextet = i;
            lastAllZeroHextet = i;
            i++;
            break;
        }

        i++;
    }

    while( i < 8 )
    {
        if( addr->s6_addr16[i] != 0 )
        {
            lastAllZeroHextet = i;
            break;
        }
        i++;
    }

    i = 0;
    if( firstAllZeroHextet == 0 )
    {
        strcat(strAddr, ":");
    }
    else
    {
        while( i < firstAllZeroHextet )
        {
            sprintf(hextet, "%X:", addr->s6_addr16[i]);
            strcat(strAddr, hextet);
            i++;
        }
    }

    i = lastAllZeroHextet;
    if( lastAllZeroHextet == 8 )
    {
        strcat(strAddr, ":");
    }
    else
    {
        while( i < 8 )
        {
            sprintf(hextet, ":%X", addr->s6_addr16[i]);
            strcat(strAddr, hextet);
            i++;
        }
    }

    return result;
}
#endif

bool in6AreAddrEqual(ipv6Address_t* a, ipv6Address_t* b)
{
    for( uint8_t index = 0; index < sizeof(ipv6Address_t); index++ )
    {
        if( a->s6_addr[index] != b->s6_addr[index] )
        {
            return false;
        }
    }
    return true;
}

void in6CopyAddr(ipv6Address_t *dst, ipv6Address_t *src)
{
    for( uint8_t index = 0; index < sizeof(ipv6Address_t); index++ )
    {
        dst->s6_addr[index] = src->s6_addr[index];
    }
}

/*
 * djb2 hashing algorithm
 */
uint16_t makeIpv6ToHash(ipv6Address_t *addr)
{
    uint16_t hash = 5381;
    uint16_t c;
    uint8_t* ptr = (uint8_t*)addr;
    
    while(c = *ptr++)
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

bool ipv6FilterOnMulticastEth(mac48Address_t* macAddr)
{
    return( ipv6IsEthMcAllNodes(macAddr) || ipv6IsEthMcSolicitedNode(macAddr) );
}

/**
  Section: Private functions
*/

static bool ipv6IsEthMcSolicitedNode(mac48Address_t* macAddr)
{
    ipv6Address_t* localIPv6Addr = ipv6db_getLinkLocalAddr();
            
    return( ((((const uint16_t *) (macAddr))[0] == 0x3333)
         && (((const uint8_t *) (macAddr))[2] == 0xff)
         && (((const uint8_t *) (macAddr))[3] == localIPv6Addr->s6.s6_u8[13])
         && (((const uint16_t *) (macAddr))[2] == localIPv6Addr->s6.s6_u16[7]) ) );
}

uint8_t ipv6GenerateRandom(void)
{
    static bool seedInit = false;
    if( !seedInit ) 
    {
        mac48Address_t *addr = (mac48Address_t *)MAC_getAddress();
        uint16_t hash = 5381;
        uint16_t c;

        while(c = *(uint8_t*)addr++)
        {
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }

        srand( hash );
        seedInit != seedInit;
    }

    return rand();
}

bool isSolicitedNodeMulticastAddress(ipv6Address_t* dstAddr)
{
    // {0xFF02, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0xFF00, 0x0000} / 104
    
    return( dstAddr->s6_addr16[0] == 0xff02 &&
        dstAddr->s6_addr16[1] == 0x0000 &&
        dstAddr->s6_addr16[2] == 0x0000 &&
        dstAddr->s6_addr16[3] == 0x0000 &&
        dstAddr->s6_addr16[4] == 0x0000 &&
        dstAddr->s6_addr16[5] == 0x0001 &&
        dstAddr->s6_addr[13]  == 0xff);
}

bool isLinkLocal(ipv6Address_t* dstAddr)
{
    return( dstAddr->s6.s6_u16[0] == 0xfe80 && 
        dstAddr->s6.s6_u16[1] == 0x0000 &&
        dstAddr->s6.s6_u16[2] == 0x0000 && 
        dstAddr->s6.s6_u16[3] == 0x0000 );
}

//bool isLinkScope(ipv6Address_t* dstAddr)
//{
//    // link-scope multicast (ff00::/8) 
//    return false;
//}

bool ipv6IsAddrOnLink(ipv6Address_t* dstAddr)
{
//    if( isLinkScope(dstAddr) )
//    {
//        return true;
//    }
    
    if( isLinkLocal(dstAddr) ) 
    {
        return true;
    }
    
    if( isSolicitedNodeMulticastAddress(dstAddr) )
    {
        return true;
    }
    
    return false;
}

void ntohipv6(ipv6Address_t* hAddr, ipv6Address_t* nAddr)
{
    uint8_t tmpAddr[16];

    tmpAddr[0]  = nAddr->s6_addr[1];
    tmpAddr[1]  = nAddr->s6_addr[0];
    tmpAddr[2]  = nAddr->s6_addr[3];
    tmpAddr[3]  = nAddr->s6_addr[2];
    tmpAddr[4]  = nAddr->s6_addr[5];
    tmpAddr[5]  = nAddr->s6_addr[4];
    tmpAddr[6]  = nAddr->s6_addr[7];
    tmpAddr[7]  = nAddr->s6_addr[6];
    tmpAddr[8]  = nAddr->s6_addr[9];
    tmpAddr[9]  = nAddr->s6_addr[8];
    tmpAddr[10] = nAddr->s6_addr[11];
    tmpAddr[11] = nAddr->s6_addr[10];
    tmpAddr[12] = nAddr->s6_addr[13];
    tmpAddr[13] = nAddr->s6_addr[12];
    tmpAddr[14] = nAddr->s6_addr[15];
    tmpAddr[15] = nAddr->s6_addr[14];
    
    for( uint8_t index = 0; index < sizeof(ipv6Address_t); index++ )
    {
        hAddr->s6_addr[index] = tmpAddr[index];
    }

    return;
}