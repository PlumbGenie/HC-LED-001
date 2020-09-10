/**
  CoAP client implementation

  Company:
    Microchip Technology Inc.

  File Name:
    coap_resources.c

  Summary:
     This is the implementation of CoAP Resources.

  Description:
    This source file provides the implementation of the CoAP Resource Handler APIs.

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
#include <string.h>
#include "coap.h"
#include "udpv4.h"
#include "coap_options.h"
#include "coap_resources.h"
#include "data_model.h"
#include "coapMethods.h"
#include "object_heap.h"
#include "strnlen.h"
#include "coap_blockwise.h"
//#include "hardware_8b.h"

/***************************Local Macro Definitions****************************/
#define COAP_RESOURCE_START '<'
#define COAP_RESOURCE_END '>'
#define COAP_RESOURCE_NEXT ','
#define COAP_SLASH '/'
#define ARRAYSIZE(a) (sizeof(a)/sizeof(*a))

/***************************Extern Variables***********************************/
uint8_t resourceList[MAXVERTEXCOUNT];
bool firstPutEntry= true;

/******************************Static Variables********************************/
static int8_t queryName[MAX_URI_QUERY_COUNT];  
static uint8_t queryBuffer[MAXQUERYCOUNT]; 
static uint8_t vertexCount =0;
static uint8_t validQueryCount =0;
static bool isQueryInList(uint8_t idx);
static bool COAP_StartGetResponse(coapReceive_t *ptr);
static bool COAP_ProcessPutResponse(coapReceive_t *ptr);
/*****************************Local API Prototypes*****************************/
static bool COAP_QueryProcessHandler(vertex_t *resource,vertex_index_t resourceVertexIndex, coapReceive_t *ptr);
static void addToVertexList(vertex_index_t idx);
static void parseVertexList();
/******************************************************************************/

/***************************Resource Handlers**********************************/

bool coapwellknownHandler(coapReceive_t *ptr)
{
    bool ret = false;    
    
    graph_iterator leafIterator,pathIterator;
    vertex_index_t leafResourceIndex,pathResourceIndex; 
    char nameOfTheResource[MAX_WORD_SIZE];
    vertex_t pathResource;

    switch(ptr->code)
    {
        case GET:
            ptr->code = CONTENT;       
            ptr->type = ACK_TYPE;
            ret = COAP_MessageResponse(ptr);
            COAP_AddContentFormatOption(LINK_FORMAT);
            if(ret == SUCCESS)
            {
                if(isBlockOptionRequired())
                {
                    COAP_DiscoveryBlockTransmit(ptr);
                }
                else
                {
                    UDP_Write8(0xFF);                
                    graph_makeLeafIterator(&leafIterator);
                    while((leafResourceIndex = graph_nextVertex(&leafIterator))!=-1)
                    {               
                        // here is a child.... print the path
                        graph_makePathIterator(&pathIterator, leafResourceIndex);
                        UDP_Write8(COAP_RESOURCE_START);

                        while((pathResourceIndex = graph_nextVertex(&pathIterator))!=-1)
                        {                            
                            graph_getVertexAtIndex(&pathResource, pathResourceIndex);                      
                            strcpy(nameOfTheResource,string_getWordAtIndex(pathResource.nameIdx));
                            UDP_WriteBlock(nameOfTheResource,strlen(nameOfTheResource));
                            UDP_Write8(COAP_SLASH);                        
                        }
                        UDP_Write8(COAP_RESOURCE_END);
                        UDP_Write8(COAP_RESOURCE_NEXT);
                    }
                }
                COAP_Send();
            }
            break;        
        default:
            if(ptr->type !=NON_TYPE)
            {
                ptr->type = ACK_TYPE;               
                ptr->code = METHOD_NOT_ALLOWED;                        
                ret = COAP_MessageResponse(ptr);
                if(ret == SUCCESS)
                {
                    COAP_Send();
                }
            }
            ret = true;
            break;
    }    
    return ret;
}

bool COAP_ResourceHandler(vertex_t *resource,vertex_index_t resourceVertexIndex, coapReceive_t *ptr) 
{
//    bool ret = false, IsQueryMatched = true,IsContentFormatMatched = false;  
    bool ret = false,IsQueryMatched = true; //jira: CAE_MCU8-5647
    graph_iterator childIterator;
    vertex_index_t childVertexIndex;
    vertex_t theResourceVertex;         
    uint16_t resourceChildCount =0;
    int16_t remainingPayloadLength;   
   
    vertexCount = 0;
    validQueryCount=0;         
    
   
    if(resource->type!= NONRESOURCE_TYPE)
    {
        memset(resourceList,0,sizeof(resourceList));
        if(ptr->optlist.uriquery_count!=0)
        {
            IsQueryMatched = COAP_QueryProcessHandler(resource,resourceVertexIndex,ptr);
        }
        if(IsQueryMatched)
        {
            if(vertexCount == 0)
            {
                if(resource->objIdx==0 && resource->childCount !=0)
                {   
                    resourceChildCount = graph_get_child_count(resourceVertexIndex);
                    graph_makeChildIterator(&childIterator,resourceVertexIndex);  
                    for(uint8_t i=0; i< resourceChildCount; i++)
                    {
                        childVertexIndex = graph_nextVertex(&childIterator);
                        resourceList[vertexCount] = (uint8_t)childVertexIndex;  //jira: CAE_MCU8-5647
                        vertexCount++;
                    }                    
                }
                else
                {
                    resourceList[vertexCount] = (uint8_t)resourceVertexIndex;   //jira: CAE_MCU8-5647
                    vertexCount++;
                }
            }
            
            switch(ptr->code) 
            {
                case GET:
                    ptr->code = CONTENT;  
                    ptr->type = ACK_TYPE;
                    
                    if(iscfMatched(ptr->optlist.accept_format))
                    {                      
                        ret = COAP_StartGetResponse(ptr);                             
                        if(ret==true)
                        {
                            addPayloadOverHead(resource->objIdx, vertexCount);                                
                        }                        
                    }                 
                    else
                    {
                        ret = false;
                        ptr->retCode = UNSUPPORTED_CONTENT_FORMAT;
                        
                        return ret;
                    }
                    if(ret == true)
                    {
                        for(uint8_t resourceIdx = 0; resourceIdx < vertexCount; resourceIdx++)
                        {
                            graph_getVertexAtIndex(&theResourceVertex,resourceList[resourceIdx]);
                            if((restfulHandler[(uint8_t)resource->type]->getter) != NULL)  //jira: CAE_MCU8-5647
                            {                                
                                ret =(unsigned char)restfulHandler[(uint8_t)resource->type]->getter(theResourceVertex.objIdx,(uint16_t)theResourceVertex.idx,(uint8_t)ptr->payload_length);  //jira: CAE_MCU8-5647
                            }
                            else
                            {
                                ptr->retCode = METHOD_NOT_ALLOWED;
                                ret = false;  
                            }
                        }
                        if(ret == true)
                        {
                            COAP_Send();
                        }
                        else 
                        {
                            UDP_FlushTXPackets();
                            ptr->retCode = METHOD_NOT_ALLOWED;
                            ret = false;                                
                        }
                    }
                    else
                    {
                        ptr->retCode = METHOD_NOT_ALLOWED;
                        
                        ret = false;  
                    }
                    break;
                case PUT: 
                    ptr->code = CHANGED;   
                    ptr->type = ACK_TYPE;
                    ret = false;                                                                   
                    if(iscfMatched(ptr->optlist.content_format))
                    {
                        
//                        IsContentFormatMatched = true;    //jira: CAE_MCU8-5647

                        for(uint8_t resourceIdx = 0; resourceIdx < vertexCount; resourceIdx++)
                        {
                            if((remainingPayloadLength = processPayloadOverHead((uint8_t)ptr->payload_length,ptr->optlist.content_format)) != 0)    //jira: CAE_MCU8-5647
                            {                                    
                                graph_getVertexAtIndex(&theResourceVertex,resourceList[resourceIdx]);
                                ret =(unsigned char) restfulHandler[(uint8_t)resource->type]->putter(theResourceVertex.objIdx,(uint16_t)theResourceVertex.idx,(uint8_t)remainingPayloadLength);      //jira: CAE_MCU8-5647
                                ptr->payload_length = 0;                                    
                            }
                        }
                        if(ret==true)
                        {
                            firstPutEntry=true;
                            ret = COAP_MessageResponse(ptr);
                            if(ret==true)
                            {
                                COAP_Send();
                            }
                        }                                                               
                    }                       
                    else
                    {
                        ret = false;
                        ptr->retCode = UNSUPPORTED_CONTENT_FORMAT;
                        
                        return ret;
                    }
                    if (ret != true)
                    {
                        ptr->retCode = METHOD_NOT_ALLOWED;
                    }
                    break;
                case PATCH:
                    ptr->code = CHANGED;                
                    ret = false;
                    break;
                case POST:            
                    ptr->code = CREATED; 
                    ret = false;
                    break;
                case DELETE:
                    ptr->code = DELETED; 
                    ret = false;
                    break;                        
                default:
                    ptr->retCode = METHOD_NOT_ALLOWED; 
                    ptr->type = ACK_TYPE;
                    ret = false;
                    break; 
            }        
        }
        else
        {
            ret = false;
            ptr->type = ACK_TYPE; 
            ptr->retCode = METHOD_NOT_ALLOWED;
        }  
    }
    
    return ret;
}

/*****************************Query Handlers***********************************/

static bool COAP_QueryProcessHandler(vertex_t *resource,vertex_index_t resourceVertexIndex, coapReceive_t *ptr) 
{
    bool ret = true;
    uint8_t queryIndex =0;
    uint8_t uriQueryCount =0;   
    bool queryRetValue = true;
    query_t resourceQuery;
    query_index_t resourceQueryIndex;    
    vertex_index_t resourceOfResorceTypeIndex;    
    vertex_t resourceOfResorceType;
    graph_iterator queryIterator,resourceTypeIterator;        
    uint32_t queryValue;
    const QueryHandle_t *hptr;
    
    hptr = queryHandler;
    uriQueryCount = ptr->optlist.uriquery_count;
    memset(queryBuffer,1,sizeof(queryBuffer));
    memset(queryName,-1,sizeof(queryName));  
    
    while(queryIndex < uriQueryCount)
    {     
        graph_makeQueryIterator(&queryIterator,ptr->optlist.queryList[queryIndex].name);
        resourceQueryIndex = graph_nextQuery(&queryIterator);
        graph_getQueryAtIndex(&resourceQuery,resourceQueryIndex);
        if(hptr->query_num == resourceQueryIndex)
        {
            graph_makeTypeIterator(&resourceTypeIterator,resourceVertexIndex,resourceQuery.type1,resourceQuery.type2);
            while((resourceOfResorceTypeIndex = graph_nextVertex(&resourceTypeIterator))!=-1)
            {
                graph_getVertexAtIndex(&resourceOfResorceType,resourceOfResorceTypeIndex);
                if(resourceOfResorceType.objIdx!=0) // No Queries at Master Resources
                {       
                    queryValue =0;
                    queryValue = ptr->optlist.queryList[queryIndex].value;
                    if(resource->objIdx==0)    // Master Resources
                    {
                        queryRetValue = hptr->queryHandle(resourceOfResorceType.objIdx,queryValue);                                           
                        Query_Reset();
                        if(queryRetValue==true)
                        {
                            queryBuffer[hptr->query_num] = queryRetValue;
                            queryName[validQueryCount++] = hptr->query_num;                                                        
                            if(resourceOfResorceType.childCount == 0)
                            {
                                addToVertexList(resourceOfResorceType.myParent);                                
                            }
                            else
                            {
                                addToVertexList(resourceOfResorceType.idx);
                            }
                        }
                        else 
                        {
                            if(isQueryInList(hptr->query_num)==false)
                            {
                                queryBuffer[hptr->query_num] = queryRetValue;
                            }
                        }                      
                    }
                    else
                    {
                        queryBuffer[hptr->query_num]= hptr->queryHandle(resourceOfResorceType.objIdx,queryValue);
                    }
                }
            }  
            hptr = queryHandler;
            queryIndex++;
        }       
        else
        {
            hptr++;
        }
    }
    for(queryIndex = 0;queryIndex < MAXQUERYCOUNT; queryIndex++)
    {       
        if(queryBuffer[queryIndex] ==0)
        {
            ret = false;
            break;
        }
        ret &= queryBuffer[queryIndex];
    }
    Query_Reset();  
    
    return ret;    
}

/************** Query and Resource Handler Helper APIs *************************/

static void addToVertexList(vertex_index_t idx)
{
    for(uint8_t i=0; i < vertexCount; i++)
    {
        if(resourceList[i]==idx)
            return;
    }
    resourceList[vertexCount++] = (uint8_t)idx;  //jira: CAE_MCU8-5647
}

static bool isQueryInList(uint8_t idx)
{
    for(uint8_t i=0; i < validQueryCount; i++)
    {
        if(queryName[i]==idx)
        {
            return true;
        }
    }
    return false;
}

/************************Observe Notifier Payload******************************/

bool COAP_GetResourceData(coapResourceTypes_t type, obj_index_t objIdx, vertex_index_t vIdx, content_format_t accept_format)
{
    bool ret = false;  
    
    
    if(iscfMatched(accept_format))
    {       
        ret =(unsigned char) restfulHandler[(uint8_t)type]->getter(objIdx,(uint16_t)vIdx,0);          //jira: CAE_MCU8-5647
    }
       
    return ret;
}
  
static bool COAP_StartGetResponse(coapReceive_t *ptr)
{
    error_msg ret = ERROR;
    
    ret = COAP_MessageResponse(ptr);
    if(ret == SUCCESS)
    {        
        COAP_AddContentFormatOption(ptr->optlist.accept_format);        
        UDP_Write8(0xFF);  
    }
    
    return ret;
}



