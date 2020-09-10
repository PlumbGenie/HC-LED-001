/**
 EEPROM object heap implementation

  Company:
    Microchip Technology Inc.

  File Name:
    object_heap.c

  Summary:
    EEPROM object heap implementation.

  Description:
    This file provides the EEPROM object heap implementation.

 */

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
//  object_heap.c
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 9/24/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object_heap.h"
#include "eeprom.h"
#include <limits.h>
#include <stdbool.h>

#define MAX_OBJECT_COUNT 512
//#define MAX_OBJECT_COUNT 1024 //MO
#define OBJINDEXSIZE (MAX_OBJECT_COUNT * sizeof(obj_heap_ptr_t))
//#define VERSIONID 32766
#define VERSIONSIZE (sizeof(int16_t))

typedef struct
{
    uint32_t freeSpace;
    uint32_t firstAvailableAddress;
    int16_t objectCount;
}heap_data_t;
    
heap_data_t heapData;

#define makeOBJAddress(a) (VERSIONSIZE + (a * sizeof(obj_heap_ptr_t)))

obj_heap_ptr_t *obj_getObjectPtrAtIndex(obj_index_t idx, obj_heap_ptr_t *objptr)
{
    static obj_heap_ptr_t obj;
    obj_heap_ptr_t *op;
    
    if(objptr) op = objptr;
    else op = &obj;
    
    storagePtr P = makeOBJAddress((uint16_t)idx); //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)P, op, sizeof(obj_heap_ptr_t)); //jira: CAE_MCU8-5647
    return op;
}

void obj_setObjectPtrAtIndex(obj_index_t idx, obj_heap_ptr_t *objPtr)
{
    storagePtr P = makeOBJAddress((uint16_t)idx); //jira: CAE_MCU8-5647
    storageWriteBlock((uint32_t)P, objPtr, sizeof(obj_heap_ptr_t)); //jira: CAE_MCU8-5647
}

void obj_getObjectAtIndex(obj_index_t idx, const void *data)
{
    obj_heap_ptr_t obj;
    obj_getObjectPtrAtIndex(idx, &obj);
    if(obj.obj_inUse==objINUSE)
    {
//        printf("objINUSE idx =%d\r\n",idx);
        storageReadBlock((uint32_t)obj.ptr, data, obj.size); //jira: CAE_MCU8-5647
    }
}

void obj_setObjectAtIndex(obj_index_t idx, const void *data)
{
    obj_heap_ptr_t obj;
    obj_getObjectPtrAtIndex(idx, &obj);
    if(obj.obj_inUse==objINUSE)
    {
        storageWriteBlock((uint32_t)obj.ptr, data, obj.size); //jira: CAE_MCU8-5647
    }
}

bool databaseChanged(int16_t version, uint8_t forceUpdate) //jira: M8TS-679
{
    int16_t v; //jira: M8TS-679
    storageReadBlock(VERSIONID, &v, sizeof(v)); //jira: M8TS-679
    
//    printf("found version %d : looking for version %d \r\n",v,version);
    
    if(version != v || forceUpdate==true)
    {
        storageDatabaseErase(); // destroy the entire memory 
        return true; //jira: M8TS-679
    }
    else return false; //jira: M8TS-679
}

void updateDatabaseVersion(int16_t version) //jira: M8TS-679
    {
    
//        printf ("Formatting the database\r\n");
        storageWriteBlock(VERSIONID, &version, sizeof(version)); //jira: M8TS-679
//        storageReadBlock(VERSIONID, &v, sizeof(v));
//        printf("saving version %d \r\n",v);
    }
    


bool obj_init(void) //jira: M8TS-679
{
    obj_heap_ptr_t obj; //jira: M8TS-679
    bool ret = false; //jira: M8TS-679
    
    heapData.objectCount = 0; //jira: M8TS-679
    heapData.freeSpace = STORAGE_FREE_SIZE; //jira: M8TS-679
    heapData.freeSpace -= OBJINDEXSIZE; //jira: M8TS-679
    heapData.firstAvailableAddress = OBJINDEXSIZE; // first byte after the index is available //jira: M8TS-679

    
    for(int16_t x=MAX_OBJECT_COUNT;x!=0;x--)
    {
        obj_getObjectPtrAtIndex(x, &obj);
        if(obj.obj_inUse==objINUSE)
        {
            heapData.objectCount ++;
            heapData.freeSpace -= obj.size;
            heapData.firstAvailableAddress += obj.size;
        }
    }
    return ret;
}

int16_t  obj_count(void)
{
    return heapData.objectCount;
}
    
int16_t obj_freeSpace(void)
{
    return heapData.freeSpace;
}

obj_index_t obj_malloc(obj_size_t sz, const char *objID)
{
    obj_index_t freeObj = -1;
    obj_index_t ret = -1;
    obj_heap_ptr_t obj;
    
    // is there space for the object?
    if(heapData.freeSpace > sz)
    {
    // I need bytes in the heap
    // and a free heap object pointer
    for(int16_t x=MAX_OBJECT_COUNT;x!=0;x--)
    {
        obj_getObjectPtrAtIndex(x,&obj);
        if(obj.obj_inUse != objINUSE)
        {
            freeObj = x;
        }
        else if (strncmp(obj.obj_id,"RSRC",4)==0)
        {
            //skip
//            printf("Object_id=%s and obj_in_Use=%d\r\n",obj.obj_id, obj.obj_inUse);
            ret=-1;
        }
        else // check the ID for a match
        {
                if( obj.obj_id[0] == objID[0] &&
                    obj.obj_id[1] == objID[1] &&
                    obj.obj_id[2] == objID[2] &&
                    obj.obj_id[3] == objID[3] )
                {
                    // object with this ID already exists
//                    printf("found : ");
                    ret = x;
                    break;
        }
    }
}
        if(ret == -1 && freeObj != -1)
        {
//            printf("allocating : ");
            // reserve the space.
            obj.obj_id[0] = objID[0];
            obj.obj_id[1] = objID[1];
            obj.obj_id[2] = objID[2];
            obj.obj_id[3] = objID[3];
            obj.ptr = heapData.firstAvailableAddress;
            obj.size = sz;
            obj.obj_inUse = objINUSE;
            heapData.firstAvailableAddress += sz;
            heapData.freeSpace -= sz;
            obj_setObjectPtrAtIndex(freeObj, &obj);
//            printf("obj_id=%d\r\n",freeObj);
            ret = freeObj;
        }
    }
//    printf("OBJ[%c%c%c%c] is idx %d : %d\r\n",obj.obj_id[0],obj.obj_id[1],obj.obj_id[2],obj.obj_id[3],ret,obj.ptr);
    return ret;
}

void obj_free(obj_index_t idx)
{
    int16_t objLastAddress;
    bool SomethingMoved = true;
    int16_t objSrcAddress;
    // mark this object unused.
    obj_heap_ptr_t obj;
    
    obj_getObjectPtrAtIndex(idx,&obj);
    obj.obj_inUse = ~objINUSE;
    obj_setObjectPtrAtIndex(idx, &obj);
    
    // update the heap data
    heapData.freeSpace += obj.size;
    heapData.firstAvailableAddress -= obj.size; // this may not be true yet.. but after the compaction happens, it will be true
    
    // COMPACTION!!
    // is there any used memory after this object?
    objSrcAddress = obj.ptr;
    objLastAddress = obj.ptr + obj.size;
    while(SomethingMoved)
    {
        for(int16_t x=0;x<MAX_OBJECT_COUNT;x++)
        {
            obj_getObjectPtrAtIndex(x,&obj);
            if(obj.obj_inUse == objINUSE && obj.ptr == objLastAddress)
            {
                // YES something comes later.
                // update the index.
//                printf("obj_free Yes something after this so compact it\r\n");
                obj.ptr = objSrcAddress;
                obj_setObjectPtrAtIndex(x, &obj);
                // move it forward
                for(int16_t y = obj.size;y!=0;y--)
                {
                    storageWrite((uint32_t)objSrcAddress++, storageRead((uint16_t)objLastAddress++)); //jira: CAE_MCU8-5647
                }
                // repeat
                SomethingMoved = true;
            }
            else SomethingMoved = false;
        }
    }
}

obj_index_t obj_findById(const char *objID)
{
    obj_heap_ptr_t obj;
    obj_index_t ret = -1;
    for(int16_t x=0;x<MAX_OBJECT_COUNT;x++)
    {
        obj_getObjectPtrAtIndex(x, &obj);
        if(obj.obj_inUse==objINUSE)
        {
            if(obj.obj_id[0] == objID[0] &&
               obj.obj_id[1] == objID[1] &&
               obj.obj_id[2] == objID[2] &&
               obj.obj_id[3] == objID[3] )
            {
                // object with this ID already exists
                ret = x;
                break;
            }
        }
    }
    return ret;
}

    
obj_index_t obj_saveObject(const void *data, obj_size_t sz, const char *objID)
{
    obj_index_t ret = -1;
    ret = obj_malloc(sz, objID);
    obj_setObjectAtIndex(ret, data);
    return ret;
}

void obj_loadObjectByID(const void *data, const char *objID)
{
    obj_index_t idx = obj_findById(objID);
    obj_getObjectAtIndex(idx, data);
}

