/*

�  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software
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
//  object_heap.h
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 9/24/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#ifndef __mchp_graph__object_heap__
#define __mchp_graph__object_heap__

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "eeprom.h"

typedef int16_t obj_index_t;
typedef uint16_t obj_size_t;
typedef enum {objINUSE = 0xAA} objInUse_t;

typedef struct
{
    char obj_id[5];
    storagePtr ptr;
    objInUse_t obj_inUse;
    obj_size_t size;
}obj_heap_ptr_t;
 
bool obj_init(void); //jira: M8TS-679
int16_t  obj_count(void);
obj_index_t obj_malloc(obj_size_t sz, const char *objID);
void updateDatabaseVersion(int16_t); //jira: M8TS-679
bool databaseChanged(int16_t, uint8_t); //jira: M8TS-679
void obj_free(obj_index_t idx); 
obj_index_t obj_findById(const char *objID);
obj_heap_ptr_t *obj_getObjectPtrAtIndex(obj_index_t idx, obj_heap_ptr_t *objptr);
void obj_setObjectPtrAtIndex(obj_index_t idx, obj_heap_ptr_t *objPtr);
void obj_getObjectAtIndex(obj_index_t idx, const void *data);
void obj_setObjectAtIndex(obj_index_t idx, const void *data);
obj_index_t obj_saveObject(const void *data, obj_size_t sz, const char *objID);
void obj_loadObjectByID(const void *data, const char *objID);


#endif /* defined(__mchp_graph__object_heap__) */
