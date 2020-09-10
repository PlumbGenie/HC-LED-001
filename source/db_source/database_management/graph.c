/**
 EEPROM Graph implementation

  Company:
    Microchip Technology Inc.

  File Name:
    graph.c

  Summary:
    EEPROM Graph implementation.

  Description:
    This file provides the EEPROM Graph implementation.

 */

/*

?  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software
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
//  graph.c
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 8/10/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//
#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "graph.h"
#include "strnlen.h"
#include "string_heap.h"
#include "eeprom.h"
#include "data_model.h"
#include "object_heap.h"

 const char *vindex_id = "VIDX";
 const char *eindex_id = "EIDX";
 const char *qindex_id = "QIDX";
 const char *oindex_id = "OIDX";

 
const char *vcnt_id   = "VCNT";
const char *ecnt_id   = "ECNT";
const char *qcnt_id   = "QCNT";
const char *ocnt_id   = "OCNT";



obj_heap_ptr_t vertex_index_object;
obj_heap_ptr_t edge_index_object;
obj_heap_ptr_t query_index_object;
obj_heap_ptr_t observe_index_object;


obj_index_t vertex_index_object_index;
obj_index_t edge_index_object_index;
obj_index_t query_index_object_index;
obj_index_t observe_index_object_index;


obj_heap_ptr_t vertexCount_object;
obj_heap_ptr_t edgeCount_object;
obj_heap_ptr_t queryCount_object;
obj_heap_ptr_t observeCount_object;


obj_index_t vertexCount_object_index;
obj_index_t edgeCount_object_index;
obj_index_t queryCount_object_index;
obj_index_t observeCount_object_index;





#define makeVertexAddress(idx)      (vertex_index_object.ptr + (idx * sizeof(vertex_t)))
#define makeEdgeAddress(idx)        (edge_index_object.ptr  + (idx * sizeof(edge_t)))
#define makeQueryAddress(idx)       (query_index_object.ptr  + (idx * sizeof(query_t)))
#define makeObserveAddress(idx)      (observe_index_object.ptr  + (idx * sizeof(observe_t)))

#define makeVertexOffsetAddress(idx,offset) (vertex_index_object.ptr + (idx * sizeof(vertex_t)) + offset)


void graph_setVertexAtIndex(vertex_t *v, vertex_index_t idx)
{
    storagePtr p = makeVertexAddress((uint16_t)idx);   //jira: CAE_MCU8-5647
    storageWriteBlock((uint32_t)p,v,sizeof(vertex_t)); //jira: CAE_MCU8-5647
}

void graph_getVertexAtIndex(vertex_t *v, vertex_index_t idx)
{
    storagePtr p = makeVertexAddress((uint16_t)idx);   //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)p,v,sizeof(vertex_t));  //jira: CAE_MCU8-5647
}

void graph_getVertexOffsetAtIndex(const void *v, vertex_index_t idx,uint8_t offset, size_t size)
{
    storagePtr p = makeVertexOffsetAddress((uint16_t)idx,offset); //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)p,v,size);  //jira: CAE_MCU8-5647
}

void graph_setQueryAtIndex(query_t *q, query_index_t idx)
{
    storagePtr p = makeQueryAddress((uint16_t)idx);   //jira: CAE_MCU8-5647
    storageWriteBlock((uint32_t)p,q,sizeof(query_t)); //jira: CAE_MCU8-5647
}

void graph_getQueryAtIndex(query_t *q, query_index_t idx)
{
    storagePtr p = makeQueryAddress((uint16_t)idx);  //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)p,q,sizeof(query_t)); //jira: CAE_MCU8-5647
}

void graph_setObserveAtIndex(observe_t *q, observe_index_t idx)
{
    storagePtr p = makeObserveAddress((uint16_t)idx);   //jira: CAE_MCU8-5647
    storageWriteBlock((uint32_t)p,q,sizeof(observe_t)); //jira: CAE_MCU8-5647
}

void graph_getObserveAtIndex(observe_t *q, observe_index_t idx)
{
    storagePtr p = makeObserveAddress((uint16_t)idx);   //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)p,q,sizeof(observe_t));  //jira: CAE_MCU8-5647
}

void graph_deleteObserveAtIndex(observe_index_t idx)
{
    storagePtr p = makeEdgeAddress((uint16_t)idx);  //jira: CAE_MCU8-5647
    edge_t e = {G_INVALID,-1,-1};
    storageWriteBlock((uint32_t)p,&e,sizeof(observe_index_t)); //jira: CAE_MCU8-5647
}

void graph_setEdgeAtIndex(edge_t *e, edge_index_t idx)
{
    storagePtr p = makeEdgeAddress((uint16_t)idx);   //jira: CAE_MCU8-5647
    storageWriteBlock((uint32_t)p,e,sizeof(edge_t)); //jira: CAE_MCU8-5647
}

void graph_getEdgeAtIndex(edge_t *e, edge_index_t idx)
{
    storagePtr p = makeEdgeAddress((uint16_t)idx);  //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)p,e,sizeof(edge_t)); //jira: CAE_MCU8-5647
}

void graph_deleteEdgeAtIndex(edge_index_t idx)
{
    storagePtr p = makeEdgeAddress((uint16_t)idx);  //jira: CAE_MCU8-5647
    edge_t e = {G_INVALID,-1,-1};
    storageWriteBlock((uint32_t)p,&e,sizeof(edge_t)); //jira: CAE_MCU8-5647
}

// the vertex count should not be used for a loop terminator.
// the vertices are added in the first free space.
// when vertices are removed they are not compacted.
// this makes it possible to have holes in the vertex list.
uint16_t graph_get_vertex_count(void)
{
    uint16_t x;
    storageReadBlock((uint32_t)vertexCount_object.ptr ,&x,sizeof(x)); //jira: CAE_MCU8-5647
    return x;
}

void graph_set_vertex_count(uint16_t c)
{
    storageWriteBlock((uint32_t)vertexCount_object.ptr,&c,sizeof(c)); //jira: CAE_MCU8-5647
}

uint16_t graph_get_query_count(void)
{
    uint16_t x;
    storageReadBlock((uint32_t)queryCount_object.ptr ,&x,sizeof(x)); //jira: CAE_MCU8-5647
    return x;
}

void graph_set_query_count(uint16_t c)
{
    storageWriteBlock((uint32_t)queryCount_object.ptr,&c,sizeof(c)); //jira: CAE_MCU8-5647
}

uint16_t graph_get_observe_count(void)
{
    uint16_t x;
    storageReadBlock((uint32_t)observeCount_object.ptr ,&x,sizeof(x)); //jira: CAE_MCU8-5647
    return x;
}

void graph_set_observe_count(uint16_t c)
{
    storageWriteBlock((uint32_t)observeCount_object.ptr,&c,sizeof(c)); //jira: CAE_MCU8-5647
}


// the vertex count should not be used for a loop terminator.
// the vertexes are added in the first free splace.
// when vertexes are removed they are not compacted.
// this makes it possible to have holes in the vertex list.
uint16_t graph_get_edge_count(void)
{
    uint16_t x;
    storageReadBlock((uint32_t)edgeCount_object.ptr,&x,sizeof(x));  //jira: CAE_MCU8-5647
    return x;
}

void graph_set_edge_count(uint16_t c)
{
    storageWriteBlock((uint32_t)edgeCount_object.ptr,&c,sizeof(c)); //jira: CAE_MCU8-5647
}

void graph_init(void)
{
    vertex_index_object_index = obj_malloc(VERTEXSIZE,vindex_id);
    edge_index_object_index = obj_malloc(EDGESIZE,eindex_id);
    query_index_object_index = obj_malloc(QUERYSIZE,qindex_id);
    
    
    obj_getObjectPtrAtIndex(vertex_index_object_index, &vertex_index_object);
    obj_getObjectPtrAtIndex(edge_index_object_index, &edge_index_object);     
    obj_getObjectPtrAtIndex(query_index_object_index, &query_index_object);
    obj_getObjectPtrAtIndex(observe_index_object_index, &observe_index_object);
    
    
    vertexCount_object_index = obj_malloc(sizeof(uint16_t),vcnt_id);
    obj_getObjectPtrAtIndex(vertexCount_object_index,&vertexCount_object);
    
    edgeCount_object_index = obj_malloc(sizeof(uint16_t),ecnt_id);
    obj_getObjectPtrAtIndex(edgeCount_object_index, &edgeCount_object);
    
    queryCount_object_index = obj_malloc(sizeof(uint16_t),qcnt_id);
    obj_getObjectPtrAtIndex(queryCount_object_index,&queryCount_object);
    
    observeCount_object_index = obj_malloc(sizeof(uint16_t),ocnt_id);
    obj_getObjectPtrAtIndex(observeCount_object_index,&observeCount_object);   
    
}
    
void graph_create(void)
{
    edge_t e;
    vertex_t v;
    query_t q;
    observe_t o;
    int16_t x;
    
    graph_set_vertex_count(0);
    v.childCount = 0;
    v.id_hash = 0;
    v.nameIdx = -1;
    v.valid = G_INVALID;
    v.isObserved = -1;
    v.observeFlag = -1;    
    for(x = 0;x<MAXVERTEXCOUNT;x++)
    {
        graph_setVertexAtIndex(&v, x);
    }
    graph_set_edge_count(0);
    e.v1 = -1;
    e.v2 = -1;
    for(x=0;x<MAXEDGES;x++)
    {
        graph_setEdgeAtIndex(&e, x);
    }
    
    graph_set_query_count(0);
    q.nameIdx = -1;    
    q.valid = G_INVALID;
    for(x=0;x<MAXQUERYCOUNT;x++)
    {
        graph_setQueryAtIndex(&q,x);
    }
    
    graph_set_observe_count(0);
    o.valid = G_INVALID;  
    o.isFlag = -1;
    o.objIdx = -1;  
    
}

void graph_delete(void)
{
    graph_set_vertex_count(0);
    graph_set_edge_count(0);
}

// vertex rules for this graph.
// 1) Each vertex can have only 1 parent. (the edge list only has each vertex on the right side once)
// 2) There can only be 16 vertexes in a single generation.
// 3) The hash is 16 * parent_key + my_key.

vertex_index_t graph_add_vertex(const char *name, coapResourceTypes_t vertexType, obj_index_t objIdx)
{
    int16_t idx = 0;
    int16_t firstEmptyIdx = -1;
    vertex_t v;
 
    if(graph_get_vertex_count())
    {
        for(idx = 0; idx <MAXVERTEXCOUNT;idx ++)
        {
            graph_getVertexAtIndex(&v, idx);
            if(v.valid != G_VALID)
            {
                firstEmptyIdx = idx;
                break;
            }
        }
    }
    else
    {
        v.childCount = 0;
        v.id_hash = 0;
        v.nameIdx = -1;
        v.valid = G_INVALID;
         v.isObserved = -1;
        v.observeFlag = -1;       
        firstEmptyIdx = 0;
        //printf("firstEmptyIdx = 0\r\n");
    }
    if(firstEmptyIdx != -1)
    {
        // If the name is unique & there is an empty place, insert the data and return a pointer.
        //printf("node name:%s\r\n",name);
        v.nameIdx = string_insertWord(name,v.nameIdx);
        v.vertexStrLength = strlen(name);
        if(v.nameIdx != -1)
        {
            v.id_hash = 0;
            v.childCount = 0; // no children yet.
            v.myParent = -1; // no parents yet
            v.generation = 0;
            v.valid = G_VALID;
            v.type = vertexType;
            v.idx = firstEmptyIdx;
            v.objIdx = objIdx;
            v.isObserved = -1;
            v.observeFlag = -1;            
            graph_setVertexAtIndex(&v,firstEmptyIdx);
            graph_set_vertex_count(graph_get_vertex_count()+1);   
            
            return firstEmptyIdx;
        }
    }
    
    return -1;
}

// , this function finds the FIRST vertex with this name.
vertex_index_t graph_get_vertex_idx_byName(const char *name)
{
    vertex_t v;
    int16_t ret = -1;
    int16_t idx, nidx;

    // if the name is in the word list...
    nidx = string_findIndexOfWord(name);
    if(nidx > -1)
    {
        // find it in the vertex list.
        for(idx = 0; idx < MAXVERTEXCOUNT;idx ++)
        {
            graph_getVertexAtIndex(&v,idx);
            if(v.valid== G_VALID)
            {
                if( v.nameIdx == nidx )
                {
                    ret = idx;
                    break;
                }
            }
        }
    }
    return ret;
}

// this function finds the vertex by the type per index
vertex_index_t graph_get_vertex_idx_verified_byType(coapResourceTypes_t type, vertex_index_t idx)
{
    vertex_t v;
    int16_t ret = -1;
        graph_getVertexAtIndex(&v,idx);
        if(v.valid== G_VALID)
        {
            if( v.type == type )
            {
                ret = idx;
//                //printf("Vertex_Index=%d\r\n",idx);
            }
        }
    return ret;
}


// returns an index to the element removed from the list
// because this index was just removed... we can insert an new vertex here.
vertex_index_t graph_remove_vertex(vertex_index_t idx)
{
    vertex_t v;
    bool nameUsed = false;
    vertex_index_t i;
        
    graph_getVertexAtIndex(&v, idx);
    // Is the name used by other index's?
    i = v.nameIdx;
    //later this should be replaced by the usage count decrement
    for(int16_t x=0;x<MAXVERTEXCOUNT;x++)
    {
        graph_getVertexAtIndex(&v, x);
        if(v.nameIdx == i)
        {
            nameUsed = true;
            break;
        }
    }
    // remove the vertex
    if(!nameUsed)
        string_removeWord(v.nameIdx);
    v.nameIdx = -1;
//    v.workerIndex = 0;
    v.valid = G_INVALID;
     v.isObserved = -1;
    v.observeFlag = -1;   
    
    //Delete Vertex at Index
    graph_setVertexAtIndex(&v,idx);
    // update the vertex count
    graph_set_vertex_count(graph_get_vertex_count() - 1);
    // all done
//    //printf("VertexCount =%d\r\n",graph_get_vertex_count());
    return idx;
}

void graph_add_edge(vertex_index_t vertex1, vertex_index_t vertex2)
{
    bool haveAParent = false;
    edge_index_t firstEmpty = -1;
    edge_t e;
    vertex_t v1,v2;
    int16_t hash;
    if(vertex1>-1 && vertex2>-1) // ensure the vertices are not valid
    {
        for(int16_t idx = 0; idx < MAXEDGES;idx ++)
        {
            graph_getEdgeAtIndex(&e, idx);
            if(e.valid != G_VALID && firstEmpty == -1)
            {
                firstEmpty = idx;
            }
            if(vertex2 == e.v2)
            {
                // vertex 2 already has a parent....
                //printf("Have a parent!!! ALARM -- vertex1=%d  vertex2=%d\r\n",vertex1,vertex2);
                haveAParent = true;
                break;
            }
        }
        if(!haveAParent && firstEmpty>-1) // does not exist in the list and there is an empty slot...
        {
            e.v1 = vertex1;
            e.v2 = vertex2;
            e.valid = G_VALID;
            graph_setEdgeAtIndex(&e,firstEmpty);
            graph_set_edge_count(graph_get_edge_count()+1);
            
            graph_getVertexAtIndex(&v1, vertex1);
//            //printf("v1.childCount=%d\r\n",v1.childCount);
            v1.childCount ++;
            hash = v1.id_hash * 16 + v1.childCount;
            graph_setVertexAtIndex(&v1, vertex1);
//            //printf("Parent_Vertex=%d  ParentchildCount=%d child_Vertex=%d\r\n", v1.idx, v1.childCount,vertex2);
            
            graph_getVertexAtIndex(&v2, vertex2);
            v2.myParent = vertex1;
            v2.id_hash = (uint32_t)hash;   //jira: CAE_MCU8-5647
            graph_setVertexAtIndex(&v2, vertex2);     
            
        }
    }
}

void  graph_remove_edge(vertex_index_t vertex1, vertex_index_t vertex2)
{
    edge_t e;
    vertex_t v;
    for(int16_t idx = 0; idx < MAXEDGES;idx ++)
    {
        graph_getEdgeAtIndex(&e, idx);
//        //printf("Parent_Vertex=%d, e.v1=%d, e.v2=%d,vertex1.idx=%d,vertex2.idx=%d\r\n",vertex1,e.v1,e.v2,vertex1,vertex2);
        if( (vertex1 == e.v1) && (vertex2 == e.v2))
        {
            // found an existing edge...
            
            graph_getVertexAtIndex(&v, vertex1);
//            //printf("v.childCount=%d\r\n",v.childCount);
            v.childCount --;
//            //printf("FOUND - Parent_Vertex=%d, vertex1.idx=%d, v1.childcount=%d, vertex2.idx=%d\r\n",vertex1,e.v1,v.childCount,e.v2);
            graph_setVertexAtIndex(&v, vertex1);
            graph_deleteEdgeAtIndex(idx);
            graph_set_edge_count(graph_get_edge_count()-1);
            break;
        }
    }
    
}


void graph_updateHashIDs(void)
{  
    graph_iterator childIterator;
    vertex_index_t childVertexIdx;
    vertex_t parentVertex,childVertex;
    uint16_t cc =1;
    
    for(uint8_t parentVertexIdx=0; parentVertexIdx < MAXVERTEXCOUNT; parentVertexIdx ++)
    {
        cc =1;
        graph_getVertexAtIndex(&parentVertex,parentVertexIdx);

        graph_makeChildIterator(&childIterator,parentVertexIdx);   
        while((childVertexIdx = graph_nextVertex(&childIterator) )!= -1  && cc <= parentVertex.childCount)
        {  
            graph_getVertexAtIndex(&childVertex, childVertexIdx);
            childVertex.id_hash = parentVertex.id_hash * 16 + cc;
            graph_setVertexAtIndex(&childVertex, childVertexIdx);                 
            cc++;
        }
    }
    
}

vertex_index_t graph_findFirstCommonAncestor(vertex_index_t vertex1, vertex_index_t vertex2)
{
    uint32_t v1Hash = 0;
    uint32_t v2Hash = 0;
    vertex_t v;
    int16_t idx;
    vertex_index_t ret= -1;
    char generationsOfV1;
    char generationsOfV2;

    if(vertex1>-1 && vertex2>-1 && vertex1 != vertex2)
    {
        graph_getVertexAtIndex(&v, vertex1);
        v1Hash = v.id_hash;
        graph_getVertexAtIndex(&v, vertex2);
        v2Hash = v.id_hash;

        for(generationsOfV1 = 0; generationsOfV1 < 32;generationsOfV1 +=4)
        {
            if((v1Hash >> (generationsOfV1)) == 0) break;
        }
        for(generationsOfV2 = 0; generationsOfV2 < 32;generationsOfV2 +=4)
        {
            if((v2Hash >> (generationsOfV2)) == 0) break;
        }
        if(generationsOfV1 > generationsOfV2) // V1 could be a descendant of V2
        {
            v1Hash >>= (generationsOfV1 - generationsOfV2);
        }
        else if(generationsOfV1 < generationsOfV2) // V2 could be a descendant of V1
        {
            v2Hash >>= (generationsOfV2 - generationsOfV1);
        }
        else // they could be siblings
        {
            v1Hash >>= 4;
            v2Hash >>= 4;
        }
        if(v1Hash)
        {
            if(v1Hash == v2Hash)
            {
                // now find a vertex with a key == v1Hash in the vertex list because this is the ancestor
                for(idx = 0; idx < MAXVERTEXCOUNT;idx++)
                {
                    graph_getVertexAtIndex(&v, idx);
                    if(v.id_hash == v1Hash)
                    {
                        ret = idx;
                        break;
                    }
                }
            }
        }
    }
    return ret;
}

bool graph_isAChild(vertex_index_t child, vertex_index_t parent)
{
    bool ret = false;
    edge_t e;
    if(parent != -1 && child != -1)
    {
        for(int16_t idx = 0; idx < MAXVERTEXCOUNT;idx ++)
        {
            graph_getEdgeAtIndex(&e, idx);
            if(e.valid == G_VALID)
            {
                if( (parent == e.v1) && (child == e.v2) )
                {
                    ret = true;
                    break;
                }
            }
        }
    }
    return ret;
}

bool graph_isRelated(vertex_index_t vertex1, vertex_index_t vertex2)
{
    bool ret = false;
//    vertex_t v1,v2,v3;   
    uint32_t v1_idHash,v2_idHash,v3_idHash;
    if(vertex1 != -1 && vertex2 != -1)
    {          
//        graph_getVertexAtIndex(&v1, vertex1);
//        graph_getVertexAtIndex(&v2, vertex2);
        graph_getVertexOffsetAtIndex((void *)&v1_idHash,vertex1,offsetof(vertex_t,id_hash),sizeof(v1_idHash));
        graph_getVertexOffsetAtIndex((void *)&v2_idHash,vertex2,offsetof(vertex_t,id_hash),sizeof(v2_idHash));
        if(v1_idHash > v2_idHash)
        {
            v3_idHash = v1_idHash;
            v1_idHash = v2_idHash;
            v2_idHash = v3_idHash;
        }
        for(int16_t x=0; x< MAXVERTEXCOUNT ; x += 4)
        {
            if(v2_idHash >> x == v1_idHash)
            {
                ret = true;
                break;
            }
        }               
    }
    return ret;
}

void graph_sort_edges_onFrom(void)
{
    uint16_t index;  //jira: CAE_MCU8-5647
    edge_t e1,e2;
    bool moveHappened = false;
    
    // Bubble sort the list.
    do
    {
        moveHappened = false;
        for(index = 0;index < graph_get_edge_count(); index ++)
        {
            graph_getEdgeAtIndex(&e1, index);
            graph_getEdgeAtIndex(&e2, index+1);
            if(e1.v1 > e2.v1 && e1.v1 != -1) // swap positions
            {
                graph_setEdgeAtIndex(&e1, index+1);
                graph_setEdgeAtIndex(&e2, index);
                moveHappened = true;
            }
        }
        
    }while(moveHappened);
}

void graph_sort_edges_onTo(void)
{
    uint16_t index;  //jira: CAE_MCU8-5647
    edge_t e1,e2;
    bool moveHappened = false;
    
    // Bubble sort the list.
    do
    {
        moveHappened = false;
        for(index = 0;index < graph_get_edge_count(); index ++)
        {
            graph_getEdgeAtIndex(&e1, index);
            graph_getEdgeAtIndex(&e2, index+1);
            if(e1.v2 > e2.v2 && e1.v2 != -1) // swap positions
            {
                graph_setEdgeAtIndex(&e1, index+1);
                graph_setEdgeAtIndex(&e2, index);
                moveHappened = true;
            }
        }
        
    }while(moveHappened);
}

uint16_t graph_get_child_count(vertex_index_t vertex)
{
//    vertex_t v;
    uint16_t v_childCount;
//    graph_getVertexAtIndex(&v_ChildCount, vertex);
    graph_getVertexOffsetAtIndex((void *)&v_childCount, vertex,offsetof(vertex_t,childCount),sizeof(v_childCount));
    return v_childCount;
}

uint16_t graph_get_child_countRaw(vertex_index_t vertex)
{
    // return the number of neighbours for a vertex in the graph.
    // Neighbours are downstream of the current node.
    uint16_t ret = 0;
    edge_t e;
    for(int16_t idy = 0; idy < MAXEDGES;idy++)
    {
        graph_getEdgeAtIndex(&e, idy);
        if(e.v1 == vertex) ret ++;
    }
    return ret;
}

uint16_t graph_get_parent_count(vertex_index_t vertex)
{
    // return the number of neighbours for a vertex in the graph.
    // Neighbours are downstream of the current node.
    uint16_t ret = 0;
    edge_t e;
    for(int16_t idy = 0; idy < MAXEDGES;idy++)
    {
        graph_getEdgeAtIndex(&e, idy);
        if(e.v2 == vertex) ret ++;
    }
    return ret;
}


vertex_index_t graph_get_parent_idx(vertex_index_t vertex)
{
    //The rule is that only one parent per vertex. Once you find the parent you are done
    vertex_index_t ret = -1;
    edge_t e;
    for(int16_t idy = 0; idy < MAXEDGES;idy++)
    {
        graph_getEdgeAtIndex(&e, idy);
//        //printf("e.vi=%d, e.v2=%d, vertex=%d\r\n",e.v1,e.v2,vertex);
        if(e.v2 == vertex) //Does this vertex live at the child edge of a parent
        {
            ret=idy;
            break;
        
        }
    }
    return ret;
}


bool iteratorNormalHandler(graph_iterator *i)
{
    return true;
}

bool iteratorParentHandler(graph_iterator *i)
{
    edge_t e;
    vertex_t v;
    uint8_t x;
    // I am a parent if my child has NO children
    // Return -1 if my child has children
    for(x=0;x<MAXEDGES;x++) // scan ALL the edges looking for a relationship.
    {
        graph_getEdgeAtIndex(&e, x); // open this edge.
        if(e.v1 == i->Vindex) // make sure this index is the parent.
        {
            graph_getVertexAtIndex(&v, e.v2); // open the child vertex..
            if(v.childCount == 0) // this child has NO children...
                return true;      // this index is good
        }
    }
    // we made it through the loop.... and failed the child count every time....
    return false;
}

bool iteratorChildHandler(graph_iterator *i)
{
    return graph_isAChild(i->Vindex, i->VPIndex);
}

bool iteratorPathHandler(graph_iterator *i)
{
//    uint32_t v_hashPath;
    vertex_t v;
    graph_getVertexAtIndex(&v, i->Vindex);
//    graph_getVertexOffsetAtIndex((void *)&v_hashPath, i->Vindex, offsetof(vertex_t,id_hash),sizeof(v_hashPath));
//    //printf("i->Vindex=%d  v_hashPath=%d  i->hashPath >> i->hashShiftCnt=%d\r\n ",i->Vindex,v_hashPath,i->hashPath >> i->hashShiftCnt);
    if(v.id_hash == i->hashPath >> i->hashShiftCnt)
    {
        i->hashShiftCnt -= 4;
        return true;
    }
    else
        return false;
}

bool iteratorLeafHandler(graph_iterator *i)
{
    uint16_t v_childCount;
    vertex_t v;
    graph_getVertexAtIndex(&v, i->Vindex);
//     graph_getVertexOffsetAtIndex((void *)&v_childCount, i->Vindex,offsetof(vertex_t,childCount),sizeof(v_childCount));
//     //printf("LeafHandler : v_childCount=%d, i->Vindex=%d\r\n",v_childCount,i->Vindex);
    return (unsigned char)(v.childCount?false:true);  //jira: CAE_MCU8-5647
}

bool iteratorNameHandler(graph_iterator *i)
{
    vertex_t v;
    graph_getVertexAtIndex(&v, i->Vindex);
    return (unsigned char)(v.nameIdx == i->nameToFind?true:false);
}

bool iteratorQueryNameHandler(graph_iterator *i)
{
    query_t q;
    graph_getQueryAtIndex(&q, i->Qindex);
    return (unsigned char)(q.nameIdx == i->nameToFind?true:false);
}

bool iteratorQueryVertexHandler(graph_iterator *i)
{
    query_t q;
    graph_getQueryAtIndex(&q, i->Qindex);
    return (unsigned char)(q.nameIdx == i->nameToFind?true:false);
}

//bool iteratorResourceHandler(graph_iterator *i)
//{
//    vertex_t v;  
//
//   
//    graph_getVertexAtIndex(&v, i->Vindex);
//    if(graph_isAChild(v.idx,i->VPIndex))
//    {  
//        if((v.type==i->typeToFind))
//        {
//            return true;
//        }
//        else
//        {          
//            return false;
//        }        
//    }
//    else
//        return false;
//}

bool iteratorTypeHandler(graph_iterator *i)
{
    vertex_t v1,v2;
    graph_getVertexAtIndex(&v1, i->Vindex);
    graph_getVertexAtIndex(&v2,i->VPIndex);
    if(v1.type == i->type1ToFind || v1.type == i->type2ToFind)
    {
        if(graph_isRelated(i->VPIndex,v1.idx)) 
            return true;
        else if(v2.childCount==0 && (v1.myParent == v2.myParent))
            return true;
    }
    return false;   
}

bool iteratorNameIndexHandler(graph_iterator *i)
{
    int16_t v_nameIdx;  
    vertex_t v;


    graph_getVertexAtIndex(&v, i->Vindex);
    
    graph_getVertexOffsetAtIndex((void *)&v_nameIdx,i->Vindex,offsetof(vertex_t,nameIdx),sizeof(v_nameIdx));
//    if(v.nameIdx ==i->nameToFind) 
    if(v_nameIdx ==i->nameToFind) 
    {
//        //printf("******Match\r\n");
//            //printf("i->nameToFind=%d   i->Vindex=%d  i->VPIndex=%d ",i->nameToFind, i->Vindex,i->VPIndex);
        return graph_isAChild(i->Vindex, i->VPIndex);
    }
    else
    {
        return false;
    }
}

void graph_makeIterator(graph_iterator *i)
{
    if(i)
    {
        i->handler = iteratorNormalHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = -1;
        i->hashPath = 0;
        i->hashShiftCnt = 0;
    }
}

void graph_makeChildIterator(graph_iterator *i, vertex_index_t parent)
{
    if(i)
    {
        i->handler = iteratorChildHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = parent;
        i->hashPath = 0;
        i->hashShiftCnt = 0;
    }
}

void graph_makePathIterator(graph_iterator *i, vertex_index_t p)
{
    vertex_t v;
    if(i)
    {
        i->handler = iteratorPathHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = p;
        graph_getVertexAtIndex(&v, p);
        i->hashPath = v.id_hash;
        for(uint16_t x=0; x<32 ; x += 4)  //jira: CAE_MCU8-5647
        {
            if(i->hashPath >> x == 0)
            {
                i->hashShiftCnt = x - 4;
                break;
            }
        }
    }
}

void graph_makeLeafIterator(graph_iterator *i)
{
    if(i)
    {
        i->handler = iteratorLeafHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = 0;
        i->hashPath = 0;
        i->hashShiftCnt = 0;
    }
}

void graph_makeNameIterator(graph_iterator *i,string_index_t name)
{
    if(i)
    {
        i->handler = iteratorNameHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = 0;
        i->hashPath = 0;
        i->nameToFind = name;
        i->hashShiftCnt = 0;
    }
}

void graph_makeQueryIterator(graph_iterator *i,query_index_t name)
{
    if(i)
    {
        i->handler = iteratorQueryNameHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = 0;        
        i->Qindex =0;
        i->hashPath = 0;
        i->nameToFind = name;
        i->hashShiftCnt = 0;          
    }
}

//void graph_makeResourceIterator(graph_iterator *i,coapResourceTypes_t type,vertex_index_t idx)
//{
//    if(i)
//    {
//        i->handler = iteratorResourceHandler;
//        i->Eindex = 0;
//        i->Vindex = 0;
//        i->VPIndex = idx;        
//        i->Qindex =0;
//        i->hashPath = 0;
//        i->typeToFind = type;
//        i->hashShiftCnt = 0;          
//    }
//}

void graph_makeNameIndexIterator(graph_iterator *i,string_index_t parent, int16_t idx)
{
    if(i)
    {
        i->handler = iteratorNameIndexHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = parent;        
        i->hashPath = 0;
        i->nameToFind = idx;
        i->hashShiftCnt = 0;          
    }
}

void graph_makeTypeIterator(graph_iterator *i, vertex_index_t p, coapResourceTypes_t type1, coapResourceTypes_t type2)
{
    if(i)
    {
        i->handler = iteratorTypeHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = p;        
        i->hashPath = 0;
        i->type1ToFind = type1; 
        i->type2ToFind = type2;
    }
}

void graph_makeObserveIterator(graph_iterator *i, vertex_index_t p, observe_index_t o)
{
    if(i)
    {
        i->handler = iteratorTypeHandler;
        i->Eindex = 0;
        i->Vindex = 0;
        i->VPIndex = p;   
        i->Oindex = o;
        i->hashPath = 0;
        i->type1ToFind = 0; 
        i->type2ToFind = 0;
    }
}

vertex_index_t graph_nextVertex(graph_iterator *i)
{
    vertex_index_t ret = -1;
    vertex_t v;
//    valid_t v_valid;  //jira: CAE_MCU8-5647

    if(i)
    {
//        //printf("inside graph_nxtVertex()\r\n");
            for(;i->Vindex < MAXVERTEXCOUNT;i->Vindex++)
            {
                graph_getVertexAtIndex(&v,i->Vindex);
               
            
//                graph_getVertexOffsetAtIndex((void *)&v_valid,i->Vindex,offsetof(vertex_t,valid),sizeof(v_valid));
                if(v.valid ==G_VALID)
                {
//                    //printf("valid index i->Vindex=%d \r\n", i->Vindex);
                    
                    if(i->handler(i))
                    {
//                        //printf("Found ChildVindex=%d\r\n",i->Vindex);
                        ret = i->Vindex ++;
                        
                        break;
                    }
                }

            }

    }
//                //printf("ret=%d\r\n",ret);
    return ret;
}

query_index_t graph_nextQuery(graph_iterator *i)
{
    query_index_t ret = -1;
    query_t q;
    if(i)
    {
        for(;(uint16_t)i->Qindex < graph_get_query_count();i->Qindex++)  //jira: CAE_MCU8-5647
        {
            graph_getQueryAtIndex(&q,i->Qindex);
            if(i->handler(i))
            {
                ret = i->Qindex ++;
                break;
            }
        }
    }    
    return ret;
}

observe_index_t graph_nextObserve(graph_iterator *i)
{
    observe_index_t ret = -1;
    observe_t o;
    if(i)
    {
        for(;(uint16_t)i->Oindex < graph_get_observe_count();i->Oindex++)  //jira: CAE_MCU8-5647
        {
            graph_getObserveAtIndex(&o,i->Oindex);  //jira: CAE_MCU8-5647
            if(i->handler(i))
            {
                ret = i->Oindex ++;
                break;
            }
        }
    }    
    return ret;
}


edge_index_t graph_nextEdge(graph_iterator *i)
{
    edge_index_t ret = -1;
    edge_t e;
    if(i)
    {
        while(i->Eindex < MAXEDGES)
        {
            graph_getEdgeAtIndex(&e, i->Eindex);
            if(e.valid==G_VALID)
            {
                ret =  i->Eindex;
            }
            i->Eindex++;
        }
    }
    return ret;
}

query_index_t graph_add_query_list(const char *name, coapResourceTypes_t queryType1, coapResourceTypes_t queryType2, Query_DataType data_type)
{
    int16_t idx = 0;
    int16_t firstEmptyIdx = -1;
    query_t q;
    if(graph_get_query_count())
    {
        for(idx = 0; idx < MAXQUERYCOUNT;idx ++)
        {
            graph_getQueryAtIndex(&q, idx);
            if(q.valid != G_VALID)
            {
                firstEmptyIdx = idx;               
                break;
            }
        }
    }
    else
    {
        q.nameIdx = -1;    
        q.valid = G_INVALID;
        firstEmptyIdx = 0;
    }
    if(firstEmptyIdx != -1)
    {
//        //printf("firstEmptyIdx %d \r\n",firstEmptyIdx);
        // If the name is unique & there is an empty place, insert the data and return a pointer.
        q.nameIdx = string_insertWord(name,q.nameIdx);        
        q.valid = G_VALID;
        q.type1 = queryType1;
        q.type2 = queryType2;
        q.data_type = data_type;
        graph_setQueryAtIndex(&q,firstEmptyIdx);
        graph_set_query_count(graph_get_query_count()+1);
//        //printf("Name Index: %d \r\n",q.nameIdx);
    }
    return firstEmptyIdx;
}

Query_DataType graph_queryDataType(string_index_t idx)
{
    graph_iterator gi;
    query_index_t q;
    query_t query;
    
    graph_makeQueryIterator(&gi,idx);
    q = graph_nextQuery(&gi);
    
    graph_getQueryAtIndex(&query,q);
    
    return query.data_type;
}

uint32_t getDiscoveryTotalLength(void)
{
    graph_iterator leafIterator;
    graph_iterator pathIterator;
    vertex_index_t leafVertexIndex,pathVertexIndex;
    vertex_t pathVertex;
    uint32_t len=0;
    
    graph_makeLeafIterator(&leafIterator);
    while((leafVertexIndex = graph_nextVertex(&leafIterator))!=-1)
    {
        graph_makePathIterator(&pathIterator, leafVertexIndex);
        len++;
        while((pathVertexIndex = graph_nextVertex(&pathIterator))!=-1)
        {            
            graph_getVertexAtIndex(&pathVertex, pathVertexIndex);                      
            len += pathVertex.vertexStrLength + 1u;                       //jira: CAE_MCU8-5647   
        }
        len +=2;
    } 
    return len;
}

//void graph_dotDump(char *filename)
//{
//    vertex_t v;
//    edge_t e;
//    int16_t x;
//    FILE *f = fopen(filename,"w+");
//    if(f)
//    {
//        fprintf(f,"digraph test {\n");
//        // fix up the labels
//        for(x=0;x<MAXVERTEXCOUNT;x++)
//        {
//            graph_getVertexAtIndex(&v, x);
//            if(v.valid==G_VALID)
//                fprintf(f,"\t%d [label=\"%s\\n0x%08x\"]\n",x,string_getWordAtIndex(v.nameIdx),v.id_hash);
//        }
//        // print the edges
//        for(x=0;x<MAXEDGES;x++)
//        {
//            graph_getEdgeAtIndex(&e, x);
//            if(e.valid == G_VALID)
//                fprintf(f,"\t%d -> %d\n",e.v1,e.v2);
//        }
//        fprintf(f,"}\n");
//        fclose(f);
//    }
//    else
//    {
//        //printf("No file opened\n");
//    }
//}



vertex_index_t graph_get_vertex_idx_byObjID(obj_index_t objID)
{
    vertex_t v;
    int16_t ret = -1;
    uint16_t idx;    //jira: CAE_MCU8-5647
    
    if(objID > -1)
    {
        // find it in the vertex list.
        for(idx = 0; idx < graph_get_vertex_count();idx ++)
        {
            graph_getVertexAtIndex(&v,idx);
            if(v.valid== G_VALID)
            {
                if( v.objIdx == objID )
                {
                    ret = idx;
                    break;
                }
            }
        }
    }
    return ret;
}

