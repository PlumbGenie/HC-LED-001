/**
  Neighbor Cache implementation

  Company:
    Microchip Technology Inc.

  File Name:
    ndp_cache.c

  Summary:
     This is the implementation of the Neighbor Cache, used by the ND protocol.

  Description:
    This source file provides the implementation of the API for the Neighbor Cache management

 */

/* ©  [2017] Microchip Technology Inc. and its subsidiaries.  You may use this software
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tcpip_types.h"
#include "tcpip_config.h"
#include "ipv6_util.h"
#include "ndp.h"
#include "icmpv6.h"

#ifdef ENABLE_NETWORK_DEBUG
#include "log.h"
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest) 
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

/**
  Section: Public attributes
*/
neighborCacheEntry_t neighborCache[NEIGHBOR_CACHE_SIZE];

/**
  Section: Private function prototypes
*/
neighborCacheEntry_t* findOldestEntry();
neighborCacheEntry_t* findEmptyEntry();
inline void to_incomplete(neighborCacheEntry_t* entry);
inline void to_delay(neighborCacheEntry_t* entry);
inline void to_probe(neighborCacheEntry_t* entry);
inline void to_reachable(neighborCacheEntry_t* entry);
inline void to_stale(neighborCacheEntry_t* entry);


void NeighborCache_Init()
{
    for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
    {
        neighborCache[i].reachState = NO_ENTRY_EXISTS;
    }
}

error_msg NeighborCache_Lookup(ipv6Address_t* ip, mac48Address_t* ethAddr)
{
    error_msg result = MAC_NOT_FOUND;

    if( in6IsAddrUnspecified(ip) )
    {
        ethAddr->s.byte1 = 0x33;
        ethAddr->s.byte2 = 0x33;
        ethAddr->s.byte3 = 0x0;
        ethAddr->s.byte4 = 0x0;
        ethAddr->s.byte5 = 0x0;
        ethAddr->s.byte6 = 0x0;
        return SUCCESS;
    }
    
    if( ip->s6_addr[1] == 0xff ) // this is a multicast message
    {
        /** Address Mapping -- Multicast
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |0 0 1 1 0 0 1 1|0 0 1 1 0 0 1 1|    DST[13]    |     DST[14]   |    DST[15]    |     DST[16]   |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
        ethAddrCopy(ethAddr, &multicastIPv6MAC);
        ethAddr->s.byte3 = ip->s6_addr[13];
        ethAddr->s.byte4 = ip->s6_addr[12];
        ethAddr->s.byte5 = ip->s6_addr[15];
        ethAddr->s.byte6 = ip->s6_addr[14];
        result = SUCCESS;
    }
    else
    {
        neighborCacheEntry_t* entryPointer = neighborCache;
        for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
        {
            if( neighborCache[i].reachState != NO_ENTRY_EXISTS && in6AreAddrEqual(ip, &neighborCache[i].ipAddress) )
            {
                ethAddrCopy(ethAddr, &entryPointer->macAddress);
                
                switch( entryPointer->reachState )
                {
                    case GLOBAL:
                        result = GLOBAL_DESTINATION;
                        break;

                    case STALE:
                        to_delay(entryPointer);
                        /* Fallthrough */

                    case REACHABLE:
                        result = SUCCESS;
                        break;

                    default:
                        result = ADDRESS_RESOLUTION;
                        break;
                }
                break;
            }
            entryPointer ++;
        }
    }
    
    return result;
}

/** ND Neighbor Cache Maintenance
 *  
 *
 */
void NeighborCache_Manage()
{
    for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
    {
        neighborCache[i].age++; // Update age regardless of state 
        if( (neighborCache[i].reachState != NO_ENTRY_EXISTS)
         && (neighborCache[i].reachState != GLOBAL)
         && (--neighborCache[i].timer == 0) )
        {
            switch(neighborCache[i].reachState)
            {
                case INCOMPLETE:
                    if(neighborCache[i].retries == 0)
                    {
                        // Delete entry
                        neighborCache[i].reachState = NO_ENTRY_EXISTS;
                    }
                    else 
                    {
                        ICMPv6_NeighborSolicitation(&neighborCache[i].ipAddress);
                        neighborCache[i].retries--;
                        neighborCache[i].timer = 1;
                    }
                    break;
                case PROBE:
                    if(neighborCache[i].retries == 0)
                    {
                        // Delete entry
                        neighborCache[i].reachState = NO_ENTRY_EXISTS;
                    }
                    else 
                    {
                        ICMPv6_NeighborSolicitation(&neighborCache[i].ipAddress);
                        neighborCache[i].retries--;
                        neighborCache[i].timer = 1;
                    }
                    break;
                case STALE:
                    // Entry should stay be STALE forever so don`t alter state
                    // here
                    break;
                case REACHABLE:
                    to_stale(&neighborCache[i]);
                    break;
                case DELAY:
                    to_probe(&neighborCache[i]);
                    break;
                default:
                    break;
            }
        }
    }
}

void NeighborCache_CreateIncomplete(ipv6Address_t* dstAddress)
{
    neighborCacheEntry_t* newEntry = 0;
    newEntry = findEmptyEntry();
    
    if(newEntry == 0)
        newEntry = findOldestEntry();
    
    newEntry->ipAddress = *dstAddress;
    to_incomplete(newEntry);
}

void NeighborCache_CreateGlobal(ipv6Address_t* dstAddress, mac48Address_t* macAddress)
{
    neighborCacheEntry_t* newEntry = 0;
    newEntry = findEmptyEntry();
    
    if(newEntry == 0)
    {
        newEntry = findOldestEntry();
    }
    
    newEntry->ipAddress = *dstAddress;
    newEntry->macAddress = *macAddress;
    newEntry->reachState = GLOBAL;
}

void NeighborCache_CreateStale(ipv6Address_t* dstAddress, mac48Address_t* src_lla)
{
    neighborCacheEntry_t* newEntry = 0;
    newEntry = findEmptyEntry();
    
    if(newEntry == 0)
    {
        newEntry = findOldestEntry();
    }
    
    newEntry->ipAddress  = *dstAddress;
    newEntry->macAddress = *src_lla;
    to_stale(newEntry);
}

void NeighborCache_RenewAddress(ipv6Address_t* addr)
{
    for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
    {
        if( in6AreAddrEqual(&addr, &neighborCache[i].ipAddress) )
        {
            to_reachable(&neighborCache[i]);
        }
    }
}

neighborCacheEntry_t* find(ipv6Address_t* ip) 
{
    for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
        if( in6AreAddrEqual(ip, &neighborCache[i].ipAddress) )
            return &neighborCache[i];
    return NULL;
}

neighborCacheEntry_t* findOldestEntry() 
{
    neighborCacheEntry_t* oldest = neighborCache;
    for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
        if( oldest->age < neighborCache[i].age )
            oldest = &neighborCache[i];
    return oldest;
}

neighborCacheEntry_t* findEmptyEntry() 
{
    for(uint8_t i = 0; i < NEIGHBOR_CACHE_SIZE; i++ )
        if( neighborCache[i].reachState == NO_ENTRY_EXISTS )
            return &neighborCache[i];
    return NULL;
}

void transition(cacheState_t to_state, neighborCacheEntry_t* entry, uint32_t timer, uint8_t retries) 
{
    entry->reachState = to_state;
    entry->timer = timer;
    entry->retries = retries;
}

inline void to_incomplete(neighborCacheEntry_t* entry)
{
    // When in INCOMPLETE send 3 (MC) NS messages at 1 second interval
    transition(INCOMPLETE, entry, 1, MAX_MULTICAST_SOLICIT);
}

inline void to_delay(neighborCacheEntry_t* entry)
{
    // When in DELAY wait 5 seconds for U or NA3
    transition(DELAY, entry, DELAY_FIRST_PROBE_TIME, 1);
}

inline void to_probe(neighborCacheEntry_t* entry)
{
    // When in PROBE send 3 (UC) NS messages at 1 second interval
    transition(PROBE, entry, 1, MAX_UNICAST_SOLICIT);
}

inline void to_reachable(neighborCacheEntry_t* entry) {
    transition(REACHABLE, entry, REACHABLE_TIME, 1);
    entry->age = 0;
}

inline void to_stale(neighborCacheEntry_t* entry)
{
    transition(STALE, entry, 0xFFFFFFFF, 255);
}
