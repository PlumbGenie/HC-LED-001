/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    network_resource.h

  Summary:
    This is the header file for the network_resource.c

  Description:
    This header file provides the API for the Network_resource.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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


//
//  network.h
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 9/23/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#ifndef __mchp_graph__network__
#define __mchp_graph__network__

#include "coapMethods.h"
#include "data_model.h"
#include "string_heap.h"

typedef struct
{  
    string_index_t mgmtAddressType;//madt  
    string_index_t mgmtAddress;//madr
    string_index_t mgmtDNSName;//mdns
    string_index_t endptMacAddress; // emac
    string_index_t endptAddressType; // eadt
    string_index_t endptAddress; // eadr
    string_index_t endptDNSName; // edns
    string_index_t endptDNSServer;// dns
    string_index_t endptNTPServer;// ntp    
}network_eeprom_t;

 typedef struct 
{
  const char name[SIZE_OF_QUERY_NAME_ENUM];
  menu_items_t id;
} net_lst;



extern restfulMethodStructure_t networkMethods;
obj_index_t initNetworkResource(void);

#endif /* defined(__mchp_graph__network__) */
