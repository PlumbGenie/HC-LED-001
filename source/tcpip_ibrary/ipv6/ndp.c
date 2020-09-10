/**
  Neighbor Discovery Protocol implementation

  Company:
    Microchip Technology Inc.

  File Name:
    ndp.c

  Summary:
     This is the implementation of ND protocol.

  Description:
    This source file provides the implementation of the API for the Neighbor Discovery protocol

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
#include <stdlib.h>
#include <time.h>

#include "ethernet_driver.h"
#include "mac_address.h"
#include "tcpip_config.h"
#include "tcpip_types.h"
#include "ipv6_database.h"
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
  Section: Private macro definitions
*/

/* Neighbor Cache entry state machine */
#define NDP_CACHE_NO_ENTRY_EXISTS           0
#define NDP_CACHE_INCOMPLETE                1
#define NDP_CACHE_REACHABLE                 2
#define NDP_CACHE_STALE                     3
#define NDP_CACHE_DELAY                     4
#define NDP_CACHE_PROBE                     5

#define ND_NEIGHBOR_MSG_SIZE               (4 + 4 + 16)
#define ND_REDIRECT_MSG_SIZE               (4 + 4 + 16 + 16)
#define ND_ROUTER_ADV_SIZE                 (4 + 4 + 4 + 4)

#define R_FLAG_MASK 0x80000000
#define S_FLAG_MASK 0x40000000
#define O_FLAG_MASK 0x20000000
/**
  Section: Private types
*/
typedef enum
{
/* NDP state machine */
    NDP_INIT_STATE                  = 0,
    NDP_IDLE_STATE                  = 1,

    NDP_DAD_START                   = 10,
    NDP_DAD_WAIT_NA,
    NDP_DAD_LINKLOCAL,
    NDP_DAD_SEND,
    NDP_DAD_FAIL,

    NDP_RS_START                    = 20,
    NDP_RS_WAIT_RA,
    NDP_RS_GLOBAL_ADDR,
    NDP_RS_FAIL,

} ndpState_t;

typedef struct
{
    union
    {
        uint16_t typeLength;
        struct
        {
            uint8_t type;
            uint8_t length;
        };
    };
} icmpv6Option_t;

#define defaultPreferredLifetime    (604800uL)
#define defaultValidLifetime        (2592000uL)

/**
  Section: Private attributes
*/
extern mac48Address_t hostMacAddress;
static bool ndpDadEnable;

/* Neighbor-Discovery-related variables */
static uint8_t linkMtu;
static uint32_t baseReachableTime;
static uint8_t dupAddrDetectTransmits = 1;

ndpState_t ndpState;
uint8_t dad_retries, dad_index, rs_retries;


/**
  Section: External private attributes
*/
extern neighborCacheEntry_t neighborCache[NEIGHBOR_CACHE_SIZE];

/**
  Section: External private function prototypes
*/
extern neighborCacheEntry_t* find(ipv6Address_t* ip);
extern inline void to_reachable(neighborCacheEntry_t* entry);
extern inline void to_stale(neighborCacheEntry_t* entry);

/**
  Section: Private function prototypes
*/
void ND_Manage(void);
void ND_AddrPrefLifetimeExpireHandle(void);
void ND_AddrValidLifetimeExpireHandle(void);
void ND_CreateLinkLocalTentativeAddr(bool eui64);
void ND_PrefixManageList(prefixInformation_t* newEntry);
bool ND_IsPrefixKnown(ipv6Address_t* globalAddr, ipv6Address_t* prefixReceived, uint8_t prefixLength);

inline void ND_DAD(uint8_t index);
inline void ND_RouterDiscovery(void);
inline void ND_CalculateReachableTime(void);
inline void ND_DADFailed(void);
inline void ND_RouterDiscovered();
bool isSolicited(neighborAdvMsg_t* neighborAdvMsg);
bool isOverride(neighborAdvMsg_t* neighborAdvMsg);

/**
  Section: Public functions
*/
/**
 * ND initialization
 * @param
 * @return
 */
void NDP_Init(void)
{
    NeighborCache_Init();
    ETH_GetMAC((char*)&hostMacAddress);

    ipv6_database_info.retransTimer = RETRANS_TIMER;
    baseReachableTime = REACHABLE_TIME;
    ndpDadEnable = true;

    ND_CalculateReachableTime();

    return;
}

/**
 * ND manage
 * @param N/A
 * @return N/A
 */
void NDP_Manage(void)
{
    ipv6db_Update();
    ND_Manage();
    NeighborCache_Manage();
}

/** ND Duplicate Address Detection
 *
 * @param
 *      16-bit Hash function of the IPv6 address
 * @return
 *      Result of the DAD procedure
 */
void NDP_EnableDAD(bool state)
{
    ndpDadEnable = state;
    return;
}

/** ND Duplicate Address Detection
 *  
 * @param   Index of address on which DAD will be performed
 * @return 
 */
inline void ND_DAD(uint8_t index)
{
    ndpState = NDP_DAD_START;
    dad_retries = dupAddrDetectTransmits;
    dad_index = index;    
}

void NDP_HandleNeighborSolicitation(ipv6Address_t* srcAddr, uint16_t msgLen)
{
    uint16_t readLen = ND_NEIGHBOR_MSG_SIZE;
    ipv6Address_t* peerAddr;
    neighborSolMsg_t neighborSolMsg;
    icmpv6Option_t option;
    mac48Address_t src_lla;
    bool flag = in6IsAddrUnspecified(srcAddr);

    logMsg("ICMPv6 RX: Neighbor Solicitation", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));

    /* Destination address is FF02::1 if NS src is  :: */
    if( flag )
    {
        if( !isSolicitedNodeMulticastAddress(srcAddr) )
        {
            return;
        }
        peerAddr = (ipv6Address_t*)&mcAllNodes;
    }
    else
    {
        peerAddr = srcAddr;
    }
    
    ETH_ReadBlock( &neighborSolMsg, sizeof(neighborSolMsg_t) );

    // Validate Message
    if( in6IsAddrMulticast((ipv6Address_t*) &neighborSolMsg.targetAddr) )
    {
        return;
    }

    do
    {
        readLen += sizeof(uint16_t);
        option.typeLength = ETH_Read16();
        option.typeLength = ntohs(option.typeLength);

        if(option.length == 0)
        {
            // Nodes MUST silently discard an ND packet that contains an
            // option with length zero
            return;
        }

        switch(option.type )
        {
            case ICMPV6_SOURCE_LL_ADDR_OPT:
                // If the IP source address is the unspecified address, there is no
                // source link-layer address option in the message. - RFC 4861
                if( flag )
                {
                    return;
                }
                readLen += sizeof(mac48Address_t);
                ETH_ReadBlock( &src_lla, sizeof(mac48Address_t));

                neighborCacheEntry_t* entry = find(srcAddr);
                if(entry)
                {
                    if( !ipv6AreEthAddrEqual(&entry->macAddress, &src_lla) )
                    {
                        entry->macAddress = src_lla;
                        entry->reachState = STALE;
                    }
                }
                else
                {
                    NeighborCache_CreateStale(srcAddr, &src_lla);
                }
                break;

            default:
                // Read length * 8 - 2 bytes because length is in units of 8 bytes
                // including type (1B) and length (1B) fields that have already been read.
                readLen += option.length << 3 - 2;
                ETH_Dump(option.length << 3 - 2);
                break;
        }
    }while( readLen < msgLen );

    ICMPv6_NeighborAdvertisement(peerAddr);
}


void NDP_HandleNeighborAdvertisement(ipv6Address_t* srcAddr, ipv6Address_t* dstAddr, uint16_t msgLen)
{
    uint16_t readLen = ND_NEIGHBOR_MSG_SIZE;
    neighborAdvMsg_t neighborAdvMsg;
    icmpv6Option_t option;
    mac48Address_t targetLla;
    bool llaOptions = false;

    logMsg("ICMPv6 RX: Neighbor Advertisement", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    // Check if this message is answer to a DAD NS
    if( in6AreAddrEqual(srcAddr, ipv6db_getLinkLocalAddr()) )
    {
        ND_DADFailed();
        return;
    }
    
    ETH_ReadBlock( &neighborAdvMsg, sizeof(neighborAdvMsg_t) );
    neighborAdvMsg.flags = ntohl(neighborAdvMsg.flags);
    
    // Validate Message
    if( in6IsAddrMulticast((ipv6Address_t*) &neighborAdvMsg.targetAddr) 
     || ( in6IsAddrMulticast(dstAddr) && isSolicited(&neighborAdvMsg)))
    {
        return;
    }
    
    do
    {
        readLen += sizeof(uint16_t);
        option.typeLength = ETH_Read16();
        option.typeLength = ntohs(option.typeLength);
        
        if(option.length == 0)
        {
            // Nodes MUST silently discard an ND packet that contains an
            // option with length zero
            return;
        }

        switch(option.type )
        {
            case ICMPV6_TARGET_LL_ADDR_OPT:
                readLen += sizeof(mac48Address_t);
                ETH_ReadBlock( &targetLla, sizeof(mac48Address_t));
                llaOptions = true;
                break;

            default:
                // Read length * 8 - 2 bytes because length is in units of 8 bytes
                // including type (1B) and length (1B) fields that have already been read.
                readLen += option.length * 8 - 2;
                ETH_Dump(option.length * 8 - 2);
                break;
        }
    } while( readLen < msgLen );
    
    if( llaOptions )
    {
        neighborCacheEntry_t* entry = find(srcAddr);
        if(entry)
        {
            switch(entry->reachState)
            {
                case INCOMPLETE:
                    entry->macAddress = targetLla;
                    if( isSolicited(&neighborAdvMsg) )
                    {
                        to_reachable(entry);
                    } 
                    else
                    {
                        to_stale(entry);
                    }
                    break;
                case REACHABLE:
                    if( !ipv6AreEthAddrEqual(&entry->macAddress, &targetLla) )
                    {
                        if(isSolicited(&neighborAdvMsg) ^ isOverride(&neighborAdvMsg)) // S = 0 and O = 1 OR S = 1 and O = 0
                        {
                            to_stale(entry);
                        }
                    }
                    break;
                case STALE:
                    if( isSolicited(&neighborAdvMsg) && 
                        (isOverride(&neighborAdvMsg)
                            ||
                        (!isOverride(&neighborAdvMsg) && ipv6AreEthAddrEqual(&entry->macAddress, &targetLla) )
                        )
                      )
                    {
                        to_reachable(entry);
                    }
                    to_reachable(entry);
                    break;
                case DELAY:
                    if( isSolicited(&neighborAdvMsg) && 
                        (isOverride(&neighborAdvMsg)
                            ||
                        (!isOverride(&neighborAdvMsg) && ipv6AreEthAddrEqual(&entry->macAddress, &targetLla) )
                        )
                      )
                    {
                        to_reachable(entry);
                    }
                    
                    if( !isSolicited(&neighborAdvMsg) &&
                        isOverride(&neighborAdvMsg) &&
                        ipv6AreEthAddrEqual(&entry->macAddress, &targetLla)
                      )
                    {
                        to_stale(entry);
                    }
                    break;
                case PROBE:
                    if( isSolicited(&neighborAdvMsg) && 
                        (isOverride(&neighborAdvMsg)
                            ||
                        (!isOverride(&neighborAdvMsg) && ipv6AreEthAddrEqual(&entry->macAddress, &targetLla) )
                        )
                      )
                    {
                        to_reachable(entry);
                    }
                    
                    if( !isSolicited(&neighborAdvMsg) &&
                        isOverride(&neighborAdvMsg) &&
                        ipv6AreEthAddrEqual(&entry->macAddress, &targetLla)
                      )
                    {
                        to_stale(entry);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void NDP_HandleRedirect(ipv6Address_t* srcAddr, uint16_t msgLen)
{
    uint16_t readLen = ND_REDIRECT_MSG_SIZE;
    redirectMsg_t redirectMsg;
    icmpv6Option_t option;
    mac48Address_t targetLla;
    bool llaOptions = false;

    logMsg("ICMPv6 RX: Redirect", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    ETH_ReadBlock( &redirectMsg, sizeof(redirectMsg_t) );
    
    //Validate message
    if( isLinkLocal(srcAddr) 
     && !in6IsAddrMulticast((ipv6Address_t*) &redirectMsg.destinationAddr)
     && ( isLinkLocal((ipv6Address_t*) &redirectMsg.targetAddr) //when redirected to a router
       || in6AreAddrEqual((ipv6Address_t*) &redirectMsg.destinationAddr, (ipv6Address_t*) &redirectMsg.targetAddr) ) )//when redirected to the on-link destination
    {
        do
        {
            readLen += sizeof(uint16_t);
            option.typeLength = ETH_Read16();
            option.typeLength = ntohs(option.typeLength);

            if(option.length == 0)
            {
                // Nodes MUST silently discard an ND packet that contains an
                // option with length zero
                return;
            }

            switch(option.type )
            {
                case ICMPV6_TARGET_LL_ADDR_OPT:
                    readLen += sizeof(mac48Address_t);
                    ETH_ReadBlock( &targetLla, sizeof(mac48Address_t));
                    llaOptions = true;
                    break;

                default:
                    // Read length * 8 - 2 bytes because length is in units of 8 bytes
                    // including type (1B) and length (1B) fields that have already been read.
                    readLen += option.length << 3 - 2;
                    ETH_Dump(option.length << 3 - 2);
                    break;
            }
        }while( readLen < msgLen );
        }
    else
    {
        return;
    }

    if( llaOptions )
    {
        neighborCacheEntry_t* destinationEntry = find(&redirectMsg.destinationAddr);
        destinationEntry->macAddress = targetLla;
    } 
    else 
    {
        neighborCacheEntry_t* targetEntry = find(&redirectMsg.targetAddr);
        neighborCacheEntry_t* destinationEntry = find(&redirectMsg.destinationAddr);
        if( targetEntry )
        {
            if( targetEntry->reachState != INCOMPLETE
                && destinationEntry && destinationEntry->reachState == GLOBAL)
            {
                destinationEntry->macAddress = targetEntry->macAddress;
            }
        }
        else
        {
            NeighborCache_CreateIncomplete(&redirectMsg.targetAddr);
        }
    }
}

void NDP_HandleRouterAdvertisement(ipv6Address_t* srcAddr, uint16_t msgLen)
{
    uint16_t readLen = ND_ROUTER_ADV_SIZE;
    icmpv6Option_t option;
    routerAdvMsg_t routerAdvMsg;

    logMsg("ICMPv6 RX: Router Advertisement", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    ETH_ReadBlock( &routerAdvMsg, sizeof(routerAdvMsg_t) );
    
    //Validation of Router Advertisement Messages
    if( isLinkLocal(srcAddr) )
    {
        ND_RouterDiscovered();
        do
        {
            mac48Address_t ethAddr;
            prefixInformation_t prefInfo;
            uint32_t mtu = 0;

            readLen += sizeof(uint16_t);
            option.typeLength = ETH_Read16();
            option.typeLength = ntohs(option.typeLength);
            
            if(option.length == 0)
            {
                return;
                /* Nodes MUST silently discard an ND packet that contains an
                    option with length zero */
            }
            switch(option.type )
            {
                case ICMPV6_SOURCE_LL_ADDR_OPT:
                    readLen += sizeof(mac48Address_t);
                    ETH_ReadBlock( &ethAddr, sizeof(mac48Address_t) );
                    ipv6db_setRouterMAC( &ethAddr );
                    NeighborCache_CreateStale(srcAddr, &ethAddr);
                    break;
                
                case ICMPV6_PREFIX_INFO_OPT:
                    readLen += sizeof(prefixInformation_t);
                    ETH_ReadBlock( &prefInfo, sizeof(prefixInformation_t) );
                    
                    if( (prefInfo.lFlag == 1)
                        && (!isLinkLocal(&prefInfo.prefix) )
                        && (prefInfo.preferredLifetime < prefInfo.validLifetime) )
                    {
                        ND_PrefixManageList(&prefInfo);
                    }
                    break;
                
                case ICMPV6_MTU_OPT:
                    readLen += ICMPV6_MTU_OPT_SIZE;
                    ETH_Read16();       //reserved
                    mtu = ETH_Read32();
                    if( mtu >= MIN_MTU && mtu <= MAX_MTU)
                    {
                        linkMtu = mtu;
                    }
                    break;

                default:
                    break;
            }
            
        }while( readLen < msgLen );
        
        if( !in6AreAddrEqual(ipv6db_getRouter(), srcAddr) )
        {
            if( ipv6_database_info.routerLifetime == 0 )
            {
                ipv6db_setRouter((ipv6Address_t*)srcAddr);
            }
            else
            {
                return;
            }
        }

        ipv6_database_info.routerLifetime = routerAdvMsg.routerLifetime;

        if(routerAdvMsg.currentHopLimit != 0)
        {
            ipv6_database_info.routerHopLimit = routerAdvMsg.currentHopLimit;
        }
        if(routerAdvMsg.reachableTime != 0)
        {
            baseReachableTime = routerAdvMsg.reachableTime;
            if(routerAdvMsg.reachableTime != ipv6_database_info.reachableTime)
            {
                ND_CalculateReachableTime();        
            }   
        }
        if(routerAdvMsg.retransTime != 0)
        {
            ipv6_database_info.retransTimer = routerAdvMsg.retransTime;
        }
    }
}

bool ND_IsPrefixKnown(ipv6Address_t* globalAddr, ipv6Address_t* prefixReceived, uint8_t prefixLength)
{ 
    uint8_t bitGlobal, bitPrefix;
    for( uint8_t index = 0; index < prefixLength; index++)
    {
        bitGlobal = globalAddr->s6.s6_u8[index%8] >> (7 - index%8);
        bitPrefix = prefixReceived->s6.s6_u8[index%8] >> (7 - index%8);
        if( bitGlobal != bitPrefix )
        {
            return false;
        }
    }
    
    return true;
}

/**
  Section: Private functions
*/
void ND_Manage(void) 
{
    // This function MUST be called one a second
    switch( ndpState )
    {
        case NDP_INIT_STATE:
            if(in6IsAddrUnspecified(ipv6db_getLinkLocalAddr()))
            {
                ND_CreateLinkLocalTentativeAddr(true);
                ND_DAD(LINKLOCAL_ADDR);
                break;
            }
            
            ndpState = NDP_IDLE_STATE;
            break;

        case NDP_IDLE_STATE:
            for( uint8_t i = LINKLOCAL_ADDR; i < MAX_UNICAST; i++ )
            {
                if(ipv6db_getAddressState(i) == TENTATIVE)
                {
                    ND_DAD(i);
                    break;
                }
            }
            break;
            
        case NDP_DAD_START:
            if(dad_retries == 0) 
            {
                ipv6db_setAddressState(dad_index, PREFERRED);
                if( (ipv6db_getRouterMAC() == NULL) && (dad_index == LINKLOCAL_ADDR) /* && !DHCPv6 */ )
                {
                    ND_RouterDiscovery();
                }
                else 
                {
                    ndpState = NDP_IDLE_STATE;
                }
                break;
            }
            
            ndpState = NDP_DAD_SEND;
            
            // Fall throw
            
        case NDP_DAD_SEND:
            ICMPv6_NeighborSolicitation(ipv6db_getAddress(dad_index));
            dad_retries--;
            ndpState = NDP_DAD_START;
            break;

        case NDP_DAD_FAIL: 
            ipv6db_setAddressState(dad_index, INVALID);
            break;

        case NDP_RS_START:
            if(rs_retries == 0) 
            {
                ndpState = NDP_IDLE_STATE;
            }
            else
            {
                ICMPv6_RouterSolicitation();
                rs_retries--;
            }
            break;

        default:
            break;
    }
}

void ND_PrefixManageList(prefixInformation_t* newEntry)
{    
    ipv6Address_t* globalAddr;
    uint8_t index = 0;
    
    //check addresses;
    for( uint8_t i = GLOBAL_ADDRESS_1; i < MAX_UNICAST; i++ )
    {
        globalAddr = ipv6db_getGlobalUnicastAddr(i);
        if(ipv6db_getAddressState(i) != INVALID)
        {
            if(ND_IsPrefixKnown(globalAddr, &newEntry->prefix, newEntry->prefixLength))
            {
                ipv6_database_info.ipv6_unicastAddress[i].validLifetime = newEntry->validLifetime;
                return;
            }
        }
        else
        {
            index = i;
        }
    }

    //create new Global Address
    if( index != 0 )
    {
        globalAddr = ipv6db_getLinkLocalAddr();
        globalAddr->s6.s6_u32[0] = newEntry->prefix.s6.s6_u32[0];
        globalAddr->s6.s6_u32[1] = newEntry->prefix.s6.s6_u32[1];
        ipv6db_setAddress(index, globalAddr);
        ipv6_database_info.ipv6_unicastAddress[index].validLifetime = newEntry->validLifetime;
    }
}

void ND_CreateLinkLocalTentativeAddr(bool eui64)
{
    // Initialize the local variable with the link-local prefix
    ipv6Address_t addr = {0xfe80, 0, 0, 0, 0, 0, 0, 0};
    if( eui64 )
    {
        // Extended Unique Identifier (EUI), as per RFC2373, allows a host to assign
        // itself a unique 64-Bit IP Version 6 interface identifier (EUI-64).
        addr.s6_addr[9]  = hostMacAddress.s.byte1;
        addr.s6_addr[8]  = hostMacAddress.s.byte2 ^ IPV6_UNIVERSAL_LOCAL_FLAG;
        addr.s6_addr[11] = hostMacAddress.s.byte3;
        addr.s6_addr[10] = 0xFF;
        addr.s6_addr[13] = 0xFE;
        addr.s6_addr[12] = hostMacAddress.s.byte4;
        addr.s6_addr[15] = hostMacAddress.s.byte5;
        addr.s6_addr[14] = hostMacAddress.s.byte6;
    }
    else
    {
        for(uint8_t i = 8; i < 15; i++)
        {
            addr.s6_addr[i] = ipv6GenerateRandom();
        }        
    }

    ipv6db_setLinkLocalAddr(&addr);
}

inline void ND_RouterDiscovery(void)
{
    ndpState = NDP_RS_START;
    rs_retries = MAX_RTR_SOLICITATIONS;
}

bool NDP_IsDAD(void)
{
    return (ndpState == NDP_DAD_SEND);
}

inline void ND_DADFailed(void) {
    if( ndpState == NDP_DAD_START )
        ndpState = NDP_DAD_FAIL;
}

inline void ND_RouterDiscovered() {
    if(ndpState == NDP_RS_START)
        ndpState = NDP_IDLE_STATE;
}

bool isSolicited(neighborAdvMsg_t* neighborAdvMsg) 
{
    return ( (neighborAdvMsg->flags & S_FLAG_MASK) == S_FLAG_MASK);
}

bool isOverride(neighborAdvMsg_t* neighborAdvMsg) 
{
    return ( (neighborAdvMsg->flags & O_FLAG_MASK) == O_FLAG_MASK);
}

/* a uniformly distributed
random value between MIN_RANDOM_FACTOR and
MAX_RANDOM_FACTOR times BaseReachableTime
milliseconds. A new random value should be
calculated when BaseReachableTime changes (due to
Router Advertisements) or at least every few
hours even if no Router Advertisements are
received. */
inline void ND_CalculateReachableTime(void)
{
    ipv6_database_info.reachableTime = MIN_RANDOM_FACTOR(baseReachableTime) + (baseReachableTime * ipv6GenerateRandom() ) / UINT8_MAX;
}