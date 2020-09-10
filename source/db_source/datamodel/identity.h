/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    identity.h

  Summary:
    This is the header file for the identity.c

  Description:
    This header file provides the API for the Identity Resource.

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
//  identity.h
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 9/23/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#ifndef __mchp_graph__identity__
#define __mchp_graph__identity__


#include "coapMethods.h"
#include "data_model.h"
#include "string_heap.h"

//typedef struct
//{
//    char uuid[8];//uuid
//    char entPhysicalName[15];
//    char entPhysicalClass[15];
//    //alternate key string has not been added
//}identity_t;

typedef struct
{
    string_index_t entPhysicalNameIdx;
    string_index_t entPhysicalClassIdx;
    string_index_t alternateKeyIdx;   
}identity_eeprom_t;

//extern char *idt_id[3];
//extern char *idt_id[10];//Mo

 
 typedef struct 
{
  const char *name;
  menu_items_t id;
} identity_lst;

extern restfulMethodStructure_t identityMethods;

int16_t initIdentityResource(void);
error_msg deleteIdentityResource(obj_index_t objIdx);

//extern bool queryEnamHandler(obj_index_t objIdx,uint32_t query_value);
//extern bool queryEclaHandler(obj_index_t objIdx,uint32_t query_value);
//extern bool queryAkeyHandler(obj_index_t objIdx,uint32_t query_value);

#endif /* defined(__mchp_graph__identity__) */
