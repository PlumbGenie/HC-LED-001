/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    inventory.h

  Summary:
    This is the header file for the inventory.c

  Description:
    This header file provides the API for the Inventory Resource.

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

/* 
 * File:   inventory.h
 * Author: c14210
 *
 * Created on September 29, 2015, 2:56 PM
 */

#ifndef INVENTORY_H
#define	INVENTORY_H


#include "coapMethods.h"
#include "data_model.h"
#include "string_heap.h"


typedef struct
{
    string_index_t hardwareRevision;//hwrv
    string_index_t firmwareRevision;//fwrv
    string_index_t softwareRevision;//swrv
    string_index_t serialNumber;//snum
    string_index_t manufacturer;//manu
    string_index_t model;//modl
    
}inventory_eeprom_t;

extern restfulMethodStructure_t inventoryMethods;


//extern char *inv_id[3];

int16_t initInventoryResource(void);
error_msg deleteInventoryResource(obj_index_t objIdx);
 
 typedef struct 
{
  const char *name;
  menu_items_t id;
}inventory_lst;



extern bool queryHwrvHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryFwrvHandler(obj_index_t objIdx,uint32_t query_value);
extern bool querySwrvHandler(obj_index_t objIdx,uint32_t query_value);
extern bool querySnumHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryManuHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryModlHandler(obj_index_t objIdx,uint32_t query_value);

char* getMyManufacturerName(obj_index_t objIdx);
void updateFirmwareRev(void);// 

#endif	/* INVENTORY_H */

