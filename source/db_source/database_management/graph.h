
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
//  graph.h
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 8/10/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#ifndef __mchp_graph__graph__
#define __mchp_graph__graph__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "data_model.h"
#include "string_heap.h"
#include "object_heap.h"

typedef enum
{
    emptyType,
    type1,
    type2
}element_type;

typedef int16_t vertex_index_t;
typedef int16_t edge_index_t;
typedef int16_t query_index_t;
typedef int16_t observe_index_t;

typedef int16_t actSenCount_index_t;

typedef struct 
{
    void *getter; // worker function for getting the value
    void *setter; // worker function for setting the value
} worker_t;

typedef enum
{
    G_VALID = 0xAA,
    G_INVALID = 0xFF
} valid_t;
#define G_VALID 0xAA

typedef struct
{
    valid_t             valid;
    int16_t             nameIdx;
    uint32_t            id_hash;
    uint16_t            childCount;
    uint8_t             generation;
    obj_index_t         objIdx;
    coapResourceTypes_t type;
    vertex_index_t      myParent;
    vertex_index_t      idx;
    uint8_t            vertexStrLength;
    int8_t              isObserved;
    int8_t              observeFlag;    
}vertex_t;

typedef struct
{
    valid_t valid;
    vertex_index_t v1;
    vertex_index_t v2;
}edge_t;

typedef enum
{
    INT =0,
            STR
}Query_DataType;

typedef struct
{  
     valid_t  valid;
    int16_t  nameIdx; 
    coapResourceTypes_t type1;
    coapResourceTypes_t type2;
    Query_DataType data_type;
//     void     *data; //pointer to the structs to data models
}query_t;


typedef struct
{  
    valid_t  valid;   
    int8_t isFlag;
    obj_index_t objIdx;
//    vertex_index_t vIdx;
}observe_t;

typedef enum
{
    i_NORMAL,
    i_PARENT,
    i_CHILD,
    i_PATH
} i_mode_t;


typedef struct graph_s
{
    vertex_index_t Vindex;
    edge_index_t Eindex;
    query_index_t Qindex;
    observe_index_t Oindex;
    vertex_index_t VPIndex;   
    vertex_index_t VCIndex; 
    string_index_t nameToFind;
    uint32_t hashPath;
    uint8_t hashShiftCnt;
    coapResourceTypes_t type1ToFind;
    coapResourceTypes_t type2ToFind;
    char name[32];
    bool (*handler)(struct graph_s *i);  
} graph_iterator;

typedef bool (*iteratorHandler)(graph_iterator *i);


#define VCOUNTSIZE (sizeof(int16_t))
#define ECOUNTSIZE (sizeof(int16_t))
#define QCOUNTSIZE (sizeof(int16_t))
#define VERTEXSIZE (MAXVERTEXCOUNT * sizeof(vertex_t))
#define EDGESIZE   (MAXEDGES       * sizeof(edge_t))
#define QUERYSIZE  (MAXQUERYCOUNT * sizeof(query_t))
#define OBSERVESIZE (MAX_OBSERVERS * sizeof(observe_t))
#define GRAPHSIZE  (VCOUNTSIZE + ECOUNTSIZE + QCOUNTSIZE + VERTEXSIZE + EDGESIZE + QUERYSIZE)

#define VCOUNTOFFSET 0
#define ECOUNTOFFSET (VCOUNTOFFSET+VCOUNTSIZE)
#define VERTEXOFFSET (ECOUNTOFFSET+ECOUNTSIZE)
#define EDGEOFFSET   (VERTEXOFFSET+VERTEXSIZE)

void graph_init(void);
void graph_create(void);
void graph_delete(void);

vertex_index_t graph_add_vertex(const char *name, coapResourceTypes_t vertexType, obj_index_t objIdx);
vertex_index_t graph_get_vertex_idx_byName(const char *name);
vertex_index_t graph_get_vertex_idx_verified_byType(coapResourceTypes_t type, vertex_index_t idx);
// returns a pointer to the element removed from the list
vertex_index_t graph_remove_vertex(vertex_index_t vertex);
void graph_add_edge(vertex_index_t vertex1, vertex_index_t vertex2);
void graph_remove_edge(vertex_index_t vertex1, vertex_index_t vertex2);
bool graph_isAdjacent(vertex_index_t vertex1, vertex_index_t vertex2);
bool graph_isAChild(vertex_index_t child, vertex_index_t parent);
bool graph_isRelated(vertex_index_t vertex1, vertex_index_t vertex2);
void graph_sort_edges_onFrom(void);
void graph_sort_edges_onTo(void);
uint16_t graph_get_child_count(vertex_index_t vertex);
uint16_t graph_get_parent_count(vertex_index_t vertex);
vertex_index_t graph_get_parent_idx(vertex_index_t vertex);
uint16_t graph_get_edge_count(void);
uint16_t graph_get_vertex_count(void);
vertex_index_t graph_findFirstCommonAncestor(vertex_index_t vertex1, vertex_index_t vertex2);
void graph_setVertexAtIndex(vertex_t *v, vertex_index_t idx);
void graph_getVertexAtIndex(vertex_t *v, vertex_index_t idx);
void graph_setEdgeAtIndex(edge_t *e, edge_index_t idx);
void graph_getEdgeAtIndex(edge_t *e, edge_index_t idx);
void graph_getQueryAtIndex(query_t *q, query_index_t idx);
void graph_setQueryAtIndex(query_t *q, query_index_t idx);
void graph_setObserveAtIndex(observe_t *q, observe_index_t idx);
void graph_getObserveAtIndex(observe_t *q, observe_index_t idx);
uint16_t graph_get_vertex_count(void);
void graph_set_vertex_count(uint16_t c);
uint16_t graph_get_edge_count(void);
void graph_set_edge_count(uint16_t c);
uint16_t graph_get_query_count(void);
void graph_set_query_count(uint16_t c);

void graph_dotDump(char *filename);
void graph_getVertexOffsetAtIndex(const void *v, vertex_index_t idx,uint8_t offset, size_t size);
// iterator functions
void graph_makeIterator(graph_iterator *i);
void graph_makeChildIterator(graph_iterator *i, vertex_index_t parent);
void graph_makePathIterator(graph_iterator *i, vertex_index_t p);
void graph_makeLeafIterator(graph_iterator *i);
void graph_makeNameIterator(graph_iterator *i,string_index_t name);
void graph_makeNameIndexIterator(graph_iterator *i,string_index_t parent, int16_t idx);
void graph_makeTypeIterator(graph_iterator *i, vertex_index_t p, coapResourceTypes_t type1, coapResourceTypes_t type2);
void graph_makeQueryIterator(graph_iterator *i,query_index_t name);
void graph_makeResourceIterator(graph_iterator *i,coapResourceTypes_t type,vertex_index_t idx);


vertex_index_t graph_nextVertex(graph_iterator *i);
edge_index_t graph_nextEdge(graph_iterator *i);
query_index_t graph_nextQuery(graph_iterator *i);

query_index_t graph_add_query_list(const char *name, coapResourceTypes_t queryType1, coapResourceTypes_t queryType2, Query_DataType data_type);
Query_DataType graph_queryDataType(string_index_t idx);
uint32_t getDiscoveryTotalLength(void);


void graph_updateHashIDs(void);
vertex_index_t graph_get_vertex_idx_byObjID(obj_index_t objID);

#endif /* defined(__mchp_graph__graph__) */
