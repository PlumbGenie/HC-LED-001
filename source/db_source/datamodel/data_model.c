/**
  Data model implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    data_model.c

  Summary:
     This is the implementation of the data model.

  Description:
    This source file provides the implementation of the API for the data model.

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

/**
 Section: Included Files
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coapMethods.h"
#include "data_model.h"
#include "object_heap.h"
#include "context.h"
#include "actuator.h"
#include "act_sen.h"
#include "sensor.h"
#include "identity.h"
#include "network_resource.h"
#include "location.h"
#include "inventory.h"
#include "dfd.h"
#include "dfu.h"
#include "uuid.h"
#include "graph.h"
#include "ethernet_driver.h"
#include "contentFormat.h"
#include "udpv4.h"
#include "cbor.h"

bool impo_query;
bool domn_query;
bool role_query;
bool keyw_query;

bool pp_query;
bool pi_query;
bool or_query;
bool at_query;
bool ar_query;
bool ok_query;

bool n_query;
bool u_query;
bool v_query;
bool t_query;
bool cl_query;
bool m_query;

//jira: CAE_MCU8-5647 Move from data_model.h to here
const dataModelList_t dataModelItemsList[2] = {
  { "e", E },
  { "uuid", UUID }
};
//bool enam_query; //jira: CAE_MCU8-5647
//bool ecla_query; //jira: CAE_MCU8-5647
//bool akey_query; //jira: CAE_MCU8-5647

bool payl_query;

bool hwrv_query;
bool swrv_query;
bool snum_query;
bool fwrv_query;
bool manu_query;
bool modl_query;

uuid_eeprom_t uuid_eepr;
obj_index_t uuid_index_object_index;
 
act_sen_count_t act_sen_count;
obj_index_t act_sen_count_object_index;
//obj_heap_ptr_t act_sen_count_object;  //jira: CAE_MCU8-5647
obj_index_t vertexStrLen_object_index;
    
//uint8_t subResourceCount;   //jira: CAE_MCU8-5647
    
vertex_index_t iRoot,iRootDirectory;
static uint8_t numOfEachResourceTypes = 0;
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))
uint16_t readPtr, statVec;
static uint8_t retrievePayloadLen;
static bool uuid_ret = true;
bool ifUuidMatch(char * read_ptr);
void updateVertexList(uint8_t idx);
static uint8_t CBORProcessPayloadOverHead(uint8_t receivedPayloadLength);
static error_msg TEXT_PLAINProcessPayloadOverHead(uint8_t receivedPayloadLength);

int16_t addNetworkResource(void);  //jira: CAE_MCU8-5647
int16_t addIdentityResource(void); //jira: CAE_MCU8-5647

const restfulMethodStructure_t *restfulHandler[11] =
{
    NULL,
    &dfdMethods,          // DFD_TYPE
    &dfuMethods,           //DFU_TYPE
    &contextMethods,     //  CONTEXT_TYPE
    &actuatorMethods,    //  ACTUATOR_TYPE,
    &sensorMethods,      //  SENSOR_TYPE
    &identityMethods,    //  IDENTITY_TYPE
    &networkMethods,     //  NETWORK_TYPE
    &locationMethods,    //  LOCATION_TYPE
    &inventoryMethods,    // INVENTORY_TYPE
};


const QueryHandle_t queryHandler[23] = 
{
    /**Context***/
    {0, queryDomnHandler},
    {1, queryImpoHandler},
    {2, queryRoleHandler},
    {3, queryKeywHandler},
 /**Actuator***/   
    {4, queryPpHandler},
    {5, queryPiHandler},   
    {6, queryOrHandler}, 
    {7, queryAtHandler},
    {8, queryArHandler}, 
    {9, queryOkHandler}, 
/**Actuator and sensor***/    
    {10,queryNHandler},
 /**Sensor***/   
    {11,queryUHandler},
    {12,queryVHandler},   
    {13,queryTHandler},
    {14,queryClHandler},
    {15,queryMHandler},    
  /**Inventory***/  
    {16,queryHwrvHandler},    
    {17,queryFwrvHandler},
    {18,querySwrvHandler},
    {19,querySnumHandler},
    {20,queryManuHandler},    
    {21,queryModlHandler},
/**Location***/    
    {22,queryPaylHandler},   

};

void Query_Reset()
{
    impo_query = false;
    domn_query = false;
    role_query = false;
    keyw_query = false;
    
    pp_query = false;
    pi_query = false;
    or_query = false;
    at_query = false;
    ar_query = false;  
    ok_query = false;
  
    
    n_query = false;
    u_query = false;
    v_query = false;    
    t_query = false;
    cl_query = false;
    m_query = false;    
    
    
    payl_query = false;
    
    hwrv_query = false;
    swrv_query = false;
    snum_query = false;
    fwrv_query = false;
    manu_query = false;
    modl_query = false;
}

//const char *uuid = "UUID"; //jira: CAE_MCU8-5647
const char *scnt_id   = "SCNT";

char rsrc_uuid[UUID_STR_SIZE];
char uuid_str[UUID_STR_SIZE];

error_msg ramInit(uint8_t rsrcType, uint16_t rsrcCount, vertex_index_t idx);
obj_index_t includeMyParentResource(uint16_t existingCount,uint8_t rsrcType);
obj_index_t AddNewParentResource(uint8_t rsrcType, const char* parentResource,const char* parentOfParent);
obj_index_t initializeChildResource(uint8_t resourceType);
void updateResourceCount(uint16_t rsrcCount,uint8_t rsrcType);

void  createUuid(const char *uuid)
{
    uuid_index_object_index = obj_malloc(sizeof(uuid_eeprom_t),uuid);
    uuid_eepr.uuid = string_insertWord("1700",uuid_eepr.uuid);   
    obj_setObjectAtIndex(uuid_index_object_index, (const void *)&uuid_eepr);
}



char *getResourceUuid(int16_t idx)
{
    char rsrc_uuid_subbuff1[UUID_SUBSTR1_SIZE+1];
    char rsrc_uuid_subbuff2[UUID_SUBSTR2_SIZE+1];
    volatile uint32_t modified_bytes;
    
    memset(uuid_str,0,sizeof(uuid_str));
    memset(rsrc_uuid,0,sizeof(rsrc_uuid));
    memset(rsrc_uuid_subbuff1,0,sizeof(rsrc_uuid_subbuff1));
    memset(rsrc_uuid_subbuff2,0,sizeof(rsrc_uuid_subbuff2));
    
    uuid_eeprm_Read(uuid_str);
    memcpy(rsrc_uuid_subbuff1, &uuid_str[0],(UUID_SUBSTR1_SIZE));
    memcpy(rsrc_uuid_subbuff2, &uuid_str[UUID_SUBSTR1_SIZE], (UUID_SUBSTR2_SIZE));
    
    modified_bytes = xtoi(rsrc_uuid_subbuff2) + (uint16_t)idx; //jira: CAE_MCU8-5647
    sprintf(rsrc_uuid_subbuff2, "%lx", modified_bytes);
    
    sprintf(rsrc_uuid, "%s%s", rsrc_uuid_subbuff1, rsrc_uuid_subbuff2);
//    sprintf(rsrc_uuid,"%02x%02x%02x%02x%s",((char*)&first_str)[3],((char*)&first_str)[2],((char*)&first_str)[1],((char*)&first_str)[0],&uuid_str[8]);
    
    return rsrc_uuid;
}

const char *asct = "ASCT";// actuator sensor count ID

void initActSenCount(const char *asct)
{
   act_sen_count_object_index = obj_malloc(sizeof(act_sen_count_t),asct);
   act_sen_count.actuatorCount=0;
   act_sen_count.sensorCount =0;
   obj_setObjectAtIndex(act_sen_count_object_index, (const void *)&act_sen_count);
}

void initDiscoveryLength(void)
{
    uint32_t num =0;

    vertexStrLen_object_index = obj_malloc(sizeof(num),scnt_id);    
    obj_setObjectAtIndex(vertexStrLen_object_index, (const void *)&num);
}

void setActCount(uint8_t num)
{
    act_sen_count_object_index=  obj_findById((const char*)asct);
    act_sen_count.sensorCount = getSenCount();
    act_sen_count.actuatorCount = num;
    obj_setObjectAtIndex(act_sen_count_object_index,(const void *)&act_sen_count);
    
}

void setSenCount(uint8_t num)
{
    act_sen_count_object_index=  obj_findById((const char*)asct);
    act_sen_count.actuatorCount = getActCount();
    act_sen_count.sensorCount = num;
    obj_setObjectAtIndex(act_sen_count_object_index,(const void *)&act_sen_count);
}

void setDiscoveryLength(uint32_t num)
{
    vertexStrLen_object_index=  obj_findById((const char*)scnt_id);
   
    obj_setObjectAtIndex(vertexStrLen_object_index,(const void *)&num);
}

uint8_t getActCount(void)
{
    act_sen_count_object_index = obj_findById(asct);
    obj_getObjectAtIndex(act_sen_count_object_index,(const void *)&act_sen_count);

    return (act_sen_count.actuatorCount);
}

uint8_t getSenCount(void)
{
    act_sen_count_object_index = obj_findById(asct);
    obj_getObjectAtIndex(act_sen_count_object_index,(const void *)&act_sen_count);
    return (act_sen_count.sensorCount);
}

uint16_t getSubActSenResourceCount(void)
{
    act_sen_count_object_index = obj_findById(asct);
    obj_getObjectAtIndex(act_sen_count_object_index,(const void *)&act_sen_count);
    return (uint16_t)(act_sen_count.sensorCount+act_sen_count.actuatorCount); //jira: CAE_MCU8-5647
}


uint32_t getDiscoveryLength(void)
{
    uint32_t num;
    
    vertexStrLen_object_index = obj_findById(scnt_id);
    obj_getObjectAtIndex(vertexStrLen_object_index,(const void *)&num);
    
    return num;
}



int16_t createDatabase(void) //jira: M8TS-679
{    
    int16_t ret; //jira: M8TS-679
    initActSenCount(asct); 
    initDiscoveryLength();
    graph_create();    

    /********************************************** Add vertices **************************************************************/
    
    iRoot = graph_add_vertex( "root", NONRESOURCE_TYPE,0);    
    iRootDirectory = graph_add_vertex(ROOT_DIRECTORY, NONRESOURCE_TYPE,0);
    
        //Add edge
    graph_add_edge(iRoot, iRootDirectory); 

    //QUERIES
    graph_add_query_list("domn",CONTEXT_TYPE,-2,STR);
    graph_add_query_list("impo",CONTEXT_TYPE,-2,STR);
    graph_add_query_list("role",CONTEXT_TYPE,-2,STR);
    graph_add_query_list("keyw",CONTEXT_TYPE,-2,STR);
    
    graph_add_query_list("pp",ACTUATOR_TYPE,-2,INT);
    graph_add_query_list("pi",ACTUATOR_TYPE,-2,INT);
    graph_add_query_list("or",ACTUATOR_TYPE,-2,STR);
    graph_add_query_list("at",ACTUATOR_TYPE,-2,INT);
    graph_add_query_list("ar",ACTUATOR_TYPE,-2,INT);
    graph_add_query_list("ok",ACTUATOR_TYPE,-2,INT);   
  
    
    graph_add_query_list("n",ACTUATOR_TYPE,SENSOR_TYPE,STR);
    
    graph_add_query_list("u",SENSOR_TYPE,-2,STR);
    graph_add_query_list("v",SENSOR_TYPE,-2,INT);    
    graph_add_query_list("t",SENSOR_TYPE,-2,INT);
    graph_add_query_list("cl",SENSOR_TYPE,-2,STR);
    graph_add_query_list("m",SENSOR_TYPE,-2,INT);     
    
    graph_add_query_list("hwrv",INVENTORY_TYPE,-2,STR);
    graph_add_query_list("fwrv",INVENTORY_TYPE,-2,STR);
    graph_add_query_list("swrv",INVENTORY_TYPE,-2,STR);
    graph_add_query_list("snum",INVENTORY_TYPE,-2,STR);
    graph_add_query_list("manu",INVENTORY_TYPE,-2,STR);
    graph_add_query_list("modl",INVENTORY_TYPE,-2,STR);
    
    graph_add_query_list("payl",LOCATION_TYPE,-2,STR);
    
    
    Query_Reset(); 
    setDiscoveryLength(getDiscoveryTotalLength());

    ret =addNetworkResource(); //jira: M8TS-679
    if (ret!=-1) //jira: M8TS-679
        ret = addIdentityResource(); //jira: M8TS-679
    if (ret!=-1) //jira: M8TS-679
    printf("Database created\r\n"); //jira: M8TS-679
    else //jira: M8TS-679
        printf("Database creation Failure\r\n"); //jira: M8TS-679
    
    return ret; //jira: M8TS-679
    
}


void initResourceUUID(void)
{
    uuidValidate(uuid_str);
}

//Parent Constructor
int16_t addActOrSenResource(char *rsrcName, uint8_t rsrcType)
{
    error_msg ret=ERROR;
    uint16_t rsrcCount;
    int16_t parentOfRsrcVIdx,rsrcVIdx,contextVIdx,locationVIdx,inventoryVIdx;
    obj_index_t rsrcidx,locidx,invidx,conidx;
//    vertex_t act;    //jira: CAE_MCU8-5647
 
    
    
//    for(uint16_t i=0; i<MAXVERTEXCOUNT;i++)
//    {
    if ((rsrcVIdx=graph_get_vertex_idx_byName(rsrcName)) != -1)
    {        
        if(graph_get_vertex_idx_verified_byType(rsrcType,rsrcVIdx) != -1)
        {
            return -1;
        }
    }

    if(rsrcType == ACTUATOR_TYPE)
    {
        rsrcCount=getActCount();
        if(rsrcCount >= MAX_ACTUATORS_COUNT)
        return -1;
    }
    else if(rsrcType == SENSOR_TYPE)
    {
        rsrcCount=getSenCount();
        if(rsrcCount >= MAX_SENSORS_COUNT)
        return -1;
    }

    parentOfRsrcVIdx = includeMyParentResource(rsrcCount,rsrcType);


    //get obj_index
    rsrcidx =initializeChildResource(rsrcType);   
    conidx  =initializeChildResource(CONTEXT_TYPE);
    invidx  =initializeChildResource(INVENTORY_TYPE); 
    locidx  =initializeChildResource(LOCATION_TYPE);
   
    
    //add vertices
    rsrcVIdx        = graph_add_vertex(rsrcName,rsrcType,rsrcidx);             
    contextVIdx     = graph_add_vertex(CONTEXT, CONTEXT_TYPE,conidx);       
    locationVIdx    = graph_add_vertex(LOCATION, LOCATION_TYPE,locidx);     
    inventoryVIdx   = graph_add_vertex(INVENTORY, INVENTORY_TYPE,invidx);
    

    //add edges
    graph_add_edge(parentOfRsrcVIdx,rsrcVIdx);   
    graph_add_edge(rsrcVIdx, contextVIdx);    
    graph_add_edge(rsrcVIdx, locationVIdx); 
    graph_add_edge(rsrcVIdx, inventoryVIdx);
    


    //ram init for Sensor and Actuator
    ret= ramInit(rsrcType,rsrcCount,rsrcidx);
    if(ret!=SUCCESS)
        return -1;
    
    rsrcCount +=1;
    updateResourceCount(rsrcCount,rsrcType);
    setDiscoveryLength(getDiscoveryTotalLength());
    
    return rsrcVIdx;
}

void localResourceInit(void)
{
    localActuatorsInit();
    localSensorsInit();
}


//Resource Destructor
error_msg removeThisResource(char *resourceName, uint8_t rsrcType)
{ 
    vertex_index_t indexOfTheResourceToBeDeleted, indexOfTheParent, indexOfTheChild,indexOfRoot;
    vertex_t resourceToBeDeleted,childResourceToBeDeleted;
    uint16_t childCount;
    graph_iterator gi, iteratorToFindIndexForName;
    uint16_t num=0;
    bool found=false;
    error_msg ret = ERROR;

    //Find the resource to be deleted and return the index
    graph_makeNameIterator(&iteratorToFindIndexForName,string_findIndexOfWord(resourceName));
    while((indexOfTheResourceToBeDeleted = graph_nextVertex(&iteratorToFindIndexForName))!= -1)
    {
        if(graph_get_vertex_idx_verified_byType(rsrcType,indexOfTheResourceToBeDeleted)!= -1)
        {
            found=true;
            break;
        }
    }


    //Is the Vertex to be deleted a valid vertex?
    if(found==true)
    {
        graph_getVertexAtIndex(&resourceToBeDeleted,indexOfTheResourceToBeDeleted);
        //FindParentCount
        indexOfRoot = graph_get_vertex_idx_byName(ROOT_DIRECTORY);
        //Find the child count
        childCount = graph_get_child_count(resourceToBeDeleted.idx);
        if(childCount == 0) ret=SUCCESS;
        
        
        graph_makeChildIterator(&gi,resourceToBeDeleted.idx);  
        //Remove all the children
        for(uint8_t i =0; i< childCount; i++)
        {
        
           indexOfTheChild = graph_nextVertex(&gi);

            graph_getVertexAtIndex(&childResourceToBeDeleted,indexOfTheChild);
            if (restfulHandler[(uint8_t)childResourceToBeDeleted.type]->deleter != NULL) //jira: CAE_MCU8-5647
            {
                ret = restfulHandler[(uint8_t)childResourceToBeDeleted.type]->deleter(childResourceToBeDeleted.objIdx,NULL,(uint8_t)indexOfTheChild); //jira: CAE_MCU8-5647
            }
            obj_free(childResourceToBeDeleted.objIdx);
            
            graph_remove_edge(indexOfTheResourceToBeDeleted, indexOfTheChild);
            graph_remove_vertex(indexOfTheChild);
              
        }
        
        if(ret == SUCCESS)
        {
            
            //Remove The Resource to be deleted 
            indexOfTheParent=resourceToBeDeleted.myParent;

            graph_makeChildIterator(&gi,indexOfTheParent);        
            
            if(restfulHandler[(uint8_t)resourceToBeDeleted.type]->deleter != NULL)
            {
                ret = restfulHandler[(uint8_t)resourceToBeDeleted.type]->deleter(resourceToBeDeleted.objIdx,NULL,(uint8_t)indexOfTheResourceToBeDeleted);
            }
            obj_free(resourceToBeDeleted.objIdx);
            
            graph_remove_edge(indexOfTheParent,indexOfTheResourceToBeDeleted);// check and see if we can add the hash update here
            graph_remove_vertex(resourceToBeDeleted.idx);
        }

        if(ret==SUCCESS)
        {
            //Update the local resource information (sensors and actuators)        
            switch(rsrcType)
            {
                case (ACTUATOR_TYPE):
                    num=getActCount();
                    setActCount(--num);
                    break;
                case (SENSOR_TYPE):
                    num=getSenCount();
                    setSenCount(--num);
                    break;
                default:
                    break;
            }
        

           //Remove the parent resource with no children left if it's not the root and remove it's edge to it's parent as well 
            if(num == 0 && indexOfRoot != indexOfTheParent) 
            {
                vertex_t parentResource;
                vertex_index_t parentOfParentIdx;
                graph_getVertexAtIndex(&parentResource,indexOfTheParent);
                parentOfParentIdx = parentResource.myParent; 
                graph_remove_edge(parentOfParentIdx,indexOfTheParent); 
                graph_remove_vertex(indexOfTheParent);

            }
            } 
        //Update Hash IDs
        graph_updateHashIDs();


        //Update local resources
        localResourceInit();
        string_removeWord(resourceToBeDeleted.nameIdx);
        found=false;
    }
    
    setDiscoveryLength(getDiscoveryTotalLength());
    
    
    return ret;
    
}
    

int16_t addDFDResource     (void)
{   
    obj_index_t dfdidx;
    vertex_index_t dfdVIdx;
    error_msg ret=ERROR;
    
    int16_t rsrcVIdx;
    
    if ((rsrcVIdx=graph_get_vertex_idx_byName(DFD)) != -1)
    {        
        if(graph_get_vertex_idx_verified_byType(DFD_TYPE,rsrcVIdx) != -1)
        {
            return -1;
        }
    }
    
    dfdidx =initializeChildResource(DFD_TYPE);
    iRootDirectory=graph_get_vertex_idx_byName(ROOT_DIRECTORY);
    dfdVIdx=graph_add_vertex(DFD, DFD_TYPE,dfdidx);
    graph_add_edge(iRootDirectory, dfdVIdx); 
    setDiscoveryLength(getDiscoveryTotalLength());
    
    return dfdVIdx;
    
}

int16_t addNetworkResource(void) //jira: CAE_MCU8-5647
{
    obj_index_t netidx;
    vertex_index_t netVIdx;    
    
    
    int16_t rsrcVIdx;
    
    if ((rsrcVIdx=graph_get_vertex_idx_byName(NETWORK)) != -1)
    {        
        if(graph_get_vertex_idx_verified_byType(NETWORK_TYPE,rsrcVIdx) != -1)
        {
            return -1;
        }
    }
    
    netidx=initializeChildResource(NETWORK_TYPE);  
    iRootDirectory=graph_get_vertex_idx_byName(ROOT_DIRECTORY);
    netVIdx=graph_add_vertex(NETWORK,NETWORK_TYPE,netidx);
    graph_add_edge(iRootDirectory, netVIdx);
    setDiscoveryLength(getDiscoveryTotalLength());
    
    return netVIdx;
 
}

int16_t addIdentityResource(void) //jira: CAE_MCU8-5647
{
    obj_index_t idtidx;
    vertex_index_t idtVIdx;    
    
    
    int16_t rsrcVIdx;
    
    if ((rsrcVIdx=graph_get_vertex_idx_byName(IDENTITY)) != -1)
    {        
        if(graph_get_vertex_idx_verified_byType(IDENTITY_TYPE,rsrcVIdx) != -1)
        {
            return -1;
        }
    }
    
    idtidx=initializeChildResource(IDENTITY_TYPE);  
    iRootDirectory=graph_get_vertex_idx_byName(ROOT_DIRECTORY);
    idtVIdx=graph_add_vertex(IDENTITY,IDENTITY_TYPE,idtidx);
    graph_add_edge(iRootDirectory, idtVIdx);
    setDiscoveryLength(getDiscoveryTotalLength());
    
    return idtVIdx;
 
}

int16_t addDFUResource     (void)
{
    obj_index_t dfuidx;
    vertex_index_t dfuVIdx;
    
    error_msg ret=ERROR;
    
    int16_t rsrcVIdx;
    
    if ((rsrcVIdx=graph_get_vertex_idx_byName(DFU)) != -1)
    {        
        if(graph_get_vertex_idx_verified_byType(DFU_TYPE,rsrcVIdx) != -1)
        {
            return -1;
        }
    }
    
    dfuidx=initializeChildResource(DFU_TYPE);
    iRootDirectory=graph_get_vertex_idx_byName(ROOT_DIRECTORY);
    dfuVIdx=graph_add_vertex(DFU, DFU_TYPE,dfuidx);
    graph_add_edge(iRootDirectory, dfuVIdx);
    setDiscoveryLength(getDiscoveryTotalLength());
    
    return dfuVIdx;
 
}


obj_index_t includeMyParentResource(uint16_t existingCount,uint8_t rsrcType)
{
    obj_index_t parentIndex;
        switch(rsrcType)
        {
            case ACTUATOR_TYPE:
                if(existingCount==0)
                {
                    parentIndex = AddNewParentResource(ACTUATOR_TYPE,ACTUATORS,ROOT_DIRECTORY);
                }
                else
                {
                    parentIndex=graph_get_vertex_idx_byName(ACTUATORS);
                }
                break;
            case SENSOR_TYPE:
                 if(existingCount==0)
                {
                    parentIndex = AddNewParentResource(SENSOR_TYPE,SENSORS,ROOT_DIRECTORY);
                }
                else
                {
                    parentIndex=graph_get_vertex_idx_byName(SENSORS);
                }
                break;
            default:
                break;
        }

    return parentIndex; 
}

obj_index_t AddNewParentResource(uint8_t rsrcType, const char* parentResource,const char* parentOfParent)
{
    obj_index_t parentIndex;

    parentIndex = graph_add_vertex(parentResource, rsrcType,0);
    iRootDirectory=graph_get_vertex_idx_byName(parentOfParent);
    graph_add_edge(iRootDirectory, parentIndex); 

    return parentIndex;
}

obj_index_t initializeChildResource(uint8_t resourceType)
{
    obj_index_t childIndex;
    childIndex = restfulHandler[resourceType]->initializer();
    return childIndex;
}

error_msg ramInit(uint8_t rsrcType, uint16_t rsrcCount, vertex_index_t idx)
{
    error_msg ret=ERROR;
    if (rsrcType==ACTUATOR_TYPE)
    {
        ret = restfulHandler[ACTUATOR_TYPE]->ramInitializer((void*)&ar[rsrcCount],idx);
    }
    else if (rsrcType==SENSOR_TYPE)
        {
        ret = restfulHandler[SENSOR_TYPE]->ramInitializer((void*)&sen[rsrcCount],idx);
    }
    return ret;
}

void updateResourceCount(uint16_t rsrcCount,uint8_t rsrcType)
{
    if(rsrcType==ACTUATOR_TYPE)
            setActCount(rsrcCount);
    else if(rsrcType==SENSOR_TYPE)
            setSenCount(rsrcCount);
}

void setResourceUpdateFlags(obj_index_t objIdx)
{
    vertex_index_t resourceID;
    vertex_t resource;
    
    resourceID = graph_get_vertex_idx_byObjID(objIdx);
    graph_getVertexAtIndex(&resource,resourceID);
    
    if(resource.isObserved == 1)
    {
        resource.observeFlag = 1;

        graph_setVertexAtIndex(&resource,resourceID);
    }
    
}

void setNumberOfResourceCount(uint8_t count)
{
    numOfEachResourceTypes = count;
}

uint8_t getNumberOfResourceCount(void)
{
    return numOfEachResourceTypes;
}

void addPayloadOverHead(obj_index_t objIdx, uint8_t numOfResources)
{
    uint8_t digitalBuildingMapElements = 1;
    
    writeAuxData(&digitalBuildingMapElements,MAP);
    Get_ToEthernet((char *) "e",TEXT_STRING);    
    if(objIdx==0)
    {
        writeAuxData(&numOfResources,ARRAY);        
    }
    
}


int16_t processPayloadOverHead( uint8_t receivedPayloadLength,content_format_t cfType)
{
    switch(cfType)
    {
        case CBOR:
            receivedPayloadLength = CBORProcessPayloadOverHead(receivedPayloadLength);
            break;
        case TEXT_PLAIN:
            receivedPayloadLength = (uint8_t)TEXT_PLAINProcessPayloadOverHead(receivedPayloadLength); //jira: CAE_MCU8-5647
            break;           
        default:
            break;
    }
    
    return receivedPayloadLength;    
}

bool ifUuidMatch(char * receviedData)
{
    bool ret = false;    
    vertex_t v;    
    
     for(uint8_t i=0; i < strlen((const char *)resourceList); i++)
     {
         graph_getVertexAtIndex(&v,resourceList[i]);
         memset(rsrc_uuid,0,sizeof(rsrc_uuid));
         strcpy((char *)rsrc_uuid, getResourceUuid(v.idx));
         if(strncmp((const char *)rsrc_uuid,receviedData,sizeof(rsrc_uuid)) ==0)
         {
             ret = true;
             updateVertexList(i); // What's the purpose of the vertex list?
             break;
         }
         else
             ret = false;      
     }    
    return ret;
}

void updateVertexList(uint8_t idx)
{
    uint8_t temp =0;
    
    temp = resourceList[idx];
    resourceList[idx] = resourceList[0];
    resourceList[0] = temp;
}


void COAP_CL_nameSpace(uint8_t elementCount)
{    
    UDP_Write8(CBOR_MAJOR_MAP|1);
    CBOR_Encode_Text_String((char*)"e");
    UDP_Write8(CBOR_MAJOR_MAP|elementCount);
    
}

static uint8_t CBORProcessPayloadOverHead(uint8_t receivedPayloadLength)
{
    uint8_t len =0;
    uint8_t payload_length;  
    char receviedData[MAX_WORD_SIZE];     
    const dataModelList_t *dataModelChoice = NULL;
    dataModelChoice = dataModelItemsList;
    dataModelEnumItems mid;
    bool ret = false;
     
    uuid_ret = true;
    payload_length = receivedPayloadLength;
    if (firstPutEntry==false && payload_length==0)
    {
        payload_length = retrievePayloadLen;
        ETH_SetReadPtr(readPtr);
        ETH_SetStatusVectorByteCount(statVec);
        ret = false;     
    }    
    while(payload_length)
    {
        if(ret == false)
        {
            len =0;
            if(firstPutEntry==true)
            {
                readPtr=ETH_GetReadPtr();
                statVec=ETH_GetStatusVectorByteCount();
                retrievePayloadLen= payload_length;
                firstPutEntry=false;                
                ret = false;                 
            }
            len += parseData((char *)receviedData,sizeof(receviedData));
             dataModelChoice = NULL;
            for(uint8_t i = 0; i < ARRAY_SIZE(dataModelItemsList); i++)
            {                
                if (strncmp((const char *)receviedData, dataModelItemsList[i].name, strlen((const char *)receviedData)) == 0)
                {
                    dataModelChoice = dataModelItemsList + i;
                    break;
                }
            }           
                  
            mid = dataModelChoice ? dataModelChoice->id : NOT_DEFINED;

            switch(mid)
            {
//                memset(receviedData,0,sizeof(receviedData));    //jira: CAE_MCU8-5647
                case E:                      
                    ret = true;
                    len += parseData((char *)receviedData,sizeof(receviedData));                   
                    break;                
                case UUID:
                    uuid_ret = false;
                    len += parseData((char *)receviedData,sizeof(receviedData));                 
                    CfElements.array_map_elements--;
                    break;
                default:  
                    ret = false;
                    break;
            }
          
            payload_length -= len;
             if(ret == true && uuid_ret==true)
             {
                 receivedPayloadLength = payload_length;
                 payload_length = 0;
             }
         }
     }
    
    return receivedPayloadLength;
}

static error_msg TEXT_PLAINProcessPayloadOverHead(uint8_t receivedPayloadLength)
{     
    error_msg ret = ERROR;
    char str[MAX_WORD_SIZE];    
    uint8_t len =0;
    uint8_t payload_length;
    
    memset(str,0,sizeof(str));
    payload_length = receivedPayloadLength;
    
     if (firstPutEntry==false && payload_length==0)
     {
         payload_length = retrievePayloadLen;
         ETH_SetReadPtr(readPtr);
         ETH_SetStatusVectorByteCount(statVec);
         ret = false;
     }
     
     while(payload_length)
     {
         if(firstPutEntry==true)
         {
             readPtr=ETH_GetReadPtr();
             statVec=ETH_GetStatusVectorByteCount();
             retrievePayloadLen= payload_length;
             firstPutEntry=false;                
             ret = false;                 
         }
         len =0;
         len += parseData(str,sizeof(str));
         if(ret == ERROR)
         {
             len += parseData(str,sizeof(str));
                 CfElements.array_map_elements = (uint8_t)atoi((const char *)str);     //jira: CAE_MCU8-5647            
                 ret = true;
            
         }        
         payload_length -= len;
         if(ret == true)
         {            
            receivedPayloadLength = payload_length;
            payload_length = 0;
         }
     }   
     return receivedPayloadLength;
}



