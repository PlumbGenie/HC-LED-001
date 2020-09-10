/**
  Neighbor Discovery Protocol header file

  Company:
    Microchip Technology Inc.

  File Name:
    ndp.h

  Summary:
    This is the header file for ndp.c

  Description:
    This header file provides the API for the Neighbor Discovery protocol.

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


#ifndef NDP_H
#define NDP_H

/**
  Section: Included Files
*/
#include<time.h>
#include "tcpip_types.h"
#include "mac_address.h"
#include "icmpv6.h"

/**
  Section: Macro Declarations
*/
#define NDP_UPDATE_INTERVAL     (1)

/**
  Section: Data Types Definitions
*/

typedef enum
{
    NO_ENTRY_EXISTS  = 0,
    INCOMPLETE,             // address resolution in progress
    REACHABLE,                  // 
    STALE,                      // requires re-resolution
    DELAY,                      // resolution pending
    PROBE,                      // re-resolution in progress
    GLOBAL                      // this is a global address
} cacheState_t;

typedef struct
{
    uint16_t        ipv6AddrHash;
    mac48Address_t  macAddress;
    ipv6Address_t   ipAddress;
    cacheState_t    reachState;
    uint8_t         noAnswerProbeCnt;
    uint8_t         age; 
    uint8_t         timer;
    uint8_t         retries;
} neighborCacheEntry_t;

typedef struct
{
    uint8_t     prefixLength;
    union
    {
        uint8_t     flags;
        struct
        {
            uint8_t lFlag:1;
            uint8_t aFlag:1;
            uint8_t reserved1:6;
        };                
    };
    uint32_t        validLifetime;
    uint32_t        preferredLifetime;
    uint32_t        reserved2;
    ipv6Address_t   prefix;
} prefixInformation_t;

/**
  Section: Enumeration Definition
*/

/**
  Section: Public Interface Functions
 */

/** ND Initialization
 * This function will set up the Neighbor Discovery Protocol
 * the Neighbor Cache, the Routing Table, the prefix list
 * 
 */
void NDP_Init(void);

/** ND Duplicate Address Detection
 *
 * @param
 *      16-bit Hash function of the IPv6 address
 * @return
 *      Result of the DAD procedure
 */
void NDP_EnableDAD(bool state);

/** ND Management
 * This function performs the management of ND procedures
 *
 */
void NDP_Manage(void);

/** ND Is DAD ongoing
 * This function returns the state of the DAD procedure.
 *
 */
bool NDP_IsDAD(void);

void NDP_HandleNeighborSolicitation(ipv6Address_t* srcAddr, uint16_t msgLen);
void NDP_HandleNeighborAdvertisement(ipv6Address_t* srcAddr, ipv6Address_t* dstAddr, uint16_t msgLen);
void NDP_HandleRedirect(ipv6Address_t* srcAddr, uint16_t msgLen);
void NDP_HandleRouterAdvertisement(ipv6Address_t* srcAddr, uint16_t msgLen);

void NeighborCache_Init(void);
void NeighborCache_Manage(void);
error_msg NeighborCache_Lookup(ipv6Address_t* addr, mac48Address_t* ethAddr);
void NeighborCache_CreateIncomplete(ipv6Address_t* addr);
void NeighborCache_CreateStale(ipv6Address_t* addr, mac48Address_t* ethAddr);
void NeighborCache_CreateGlobal(ipv6Address_t* addr, mac48Address_t* ethAddr);
void NeighborCache_RenewAddress(ipv6Address_t* addr);

#endif  /* NDP_H */
