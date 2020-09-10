/*
 * File:   coap_ObserveCommon.c
 * Author: C16008
 *
 * Created on February 27, 2017, 1:26 PM
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "graph.h"
#include "coap_ObserveCommon.h"
#include "udpv4.h"
#include "coap_subjecter.h"
#include "coap_observer.h"
#include "../coap_options.h"
#include "../rtcc.h"
#include "coap_resources.h"
#include "coap.h"

typedef struct
{
    uint32_t observerIPAddress;
    char observerToken[COAP_TOKEN_SIZE];
    vertex_index_t observedVertexIndex;
    content_format_t observedAcceptFormat;
    uint16_t destPort;
    uint16_t srcPort;
    time_t observeAddedTime;
}ObserveServerParams_t;

typedef struct
{    
    coapTransmit_t observeClient;
    vertex_index_t resourceIDLinked;          
    time_t prevTimeOfNotification; //T1
    uint32_t prevObserveOptionValue;//V1    
}ObserveClientParams_t;

uint32_t observeOptionValue;
obj_index_t observer_index_object_index;
static coapResourceTypes_t resourceType;
static obj_index_t objIdx;
static vertex_index_t vIdx;

#define REGISTER    (0)
#define DEREGISTER  (1)
#define ARRAYSIZE(a)    (sizeof(a) / sizeof(*(a)))



static ObserveServerParams_t observeParams;
static ObserveClientParams_t registerPtr = {0};
static coapTransmit_t notifyPtr = {0};
bool isFreshestNotification = false;

/*****************************************Server APIs********************************************************/
static int16_t COAP_ProcessServerObserverOption(ObserveServerParams_t *options, uint32_t observeOptionValue);
static int16_t COAP_checkEndpointTokenPairAndGetObserverIDFromList(uint32_t endpointAddress, char *token);
static int16_t InitObservers(ObserveServerParams_t *options);
static int16_t COAP_AddOrUpdateToObservedResources(ObserveServerParams_t *options, int16_t obObjIdx);
static void COAP_RemoveFromObservedResources(vertex_index_t observedVertexIdx, int16_t observerIndex);
static int16_t COAP_checkResourceAndGetObserverIDFromList(vertex_index_t resourceIndex);
static int16_t addObserverLinkToResource(vertex_index_t resourceIdx, int16_t observerIdx);
static int16_t removeObserverLinkFromResource(vertex_index_t resourceIdx, int16_t observerIdx);
static error_msg COAP_SendNotifyPacket(ObserveServerParams_t *options,coapTypes msessageType);


/*********************************************Client APIs****************************************************/
static int16_t COAP_ProcessClientObserverOption(coapReceive_t *ptr, int16_t observingObjIdx);
static int16_t COAP_checkEndpointTokenPairAndGetObserveringIDFromList(uint32_t endpointAddress, char *token);
static int16_t InitObserveringResources(ObserveClientParams_t *params);
static int16_t COAP_AddOrUpdateToObservingResources(ObserveClientParams_t *params, int16_t ocpObjIdx);
static void COAP_RemoveFromObservingResources(vertex_index_t observingVertexIdx, int16_t observingObjIndex);

/*************************************************************************************************/
int16_t COAP_ProcessObserveOption(coapReceive_t *ptr)
{
    int16_t ret = -1;
    uint16_t recvdPortNumber;    
    int16_t observingObjIdx = -1;
    recvdPortNumber = UDP_GetSrcPort();    
    char recvdToken[COAP_TOKEN_SIZE];
    
    memset(recvdToken,0,COAP_TOKEN_SIZE);
    isFreshestNotification = false;
    
    
    if(recvdPortNumber == 5683 && coapObserveOptionRecvd == true)
    {  
        observeParams.observerIPAddress = ptr->destIP;
        observeParams.observedAcceptFormat = ptr->optlist.accept_format;
        observeParams.observedVertexIndex = ptr->optlist.uri_path[ptr->optlist.uripath_count-1];
        strncpy(observeParams.observerToken, ptr->token, COAP_TOKEN_SIZE);
        observeParams.destPort = UDP_GetDestPort();
        observeParams.srcPort = UDP_GetSrcPort();
        ret = COAP_ProcessServerObserverOption(&observeParams,observeOptionValue);
        ret = 1;
    }
    else
    {
        strncpy(recvdToken,ptr->token,COAP_TOKEN_SIZE);
        if((observingObjIdx = COAP_checkEndpointTokenPairAndGetObserveringIDFromList(ptr->destIP,ptr->token)) != -1)
        {
            ret = COAP_ProcessClientObserverOption(ptr,observingObjIdx);  
            if(coapObserveOptionRecvd == true)
            {
                if(COAP_IsFreshestNotification(observingObjIdx) == true)
                    isFreshestNotification = true;
            }
            if(ret != -1 && coapObserveOptionRecvd == false)
            {
                printf("Remove the register resource from the list \r\n");
                COAP_RemoveFromObservingResources(ret,observingObjIdx);
            }
        }        
    }
    
    
    return ret;
    
}

bool COAP_IsAddObserveOptionFlag()
{
    return coapObserveOptionRecvd;
}

void COAP_UpdateObserveOptionFlag(bool flag)
{
    coapObserveOptionRecvd = flag;
}

void COAP_makeNewObserveOtionValue()
{
    time_t localTime;
    
    time(&localTime);
    
    observeOptionValue =  localTime;
}


uint32_t COAP_getObserveOptionValue()
{       
    return observeOptionValue;
}

void COAP_setObserveOptionValue(char *value,uint16_t observeOptionLength)
{
    observeOptionValue = 0;
     while(observeOptionLength--)
    {
        ((uint8_t *)&observeOptionValue)[observeOptionLength] = *value++;        
    } 
}

/************************** Server Side APIs ********************/

static int16_t COAP_checkEndpointTokenPairAndGetObserverIDFromList(uint32_t endpointAddress, char *token)
{
    //Check Observer IP Address and Observer Token Pair
    observe_t o;
    ObserveServerParams_t op;
    uint16_t index = 0;
    
    for(index = 0; index <MAX_OBSERVERS; index++)
    {
        graph_getObserveAtIndex(&o, index);
        obj_getObjectAtIndex(o.objIdx,&op);
        
        if(op.observerIPAddress == endpointAddress && (strncmp(op.observerToken,token,COAP_TOKEN_SIZE) == 0))
        {            
            return o.objIdx;
        }
    }
    return -1;    
}

static int16_t COAP_ProcessServerObserverOption(ObserveServerParams_t *options, uint32_t observeOptionValue)
{
    int16_t observerIdx;
    time_t nowTime;
    
    observerIdx = COAP_checkEndpointTokenPairAndGetObserverIDFromList(options->observerIPAddress,options->observerToken);
    printf("ObserverIdx - %d \r\n",observerIdx);
    
    if(observeOptionValue == REGISTER)
    {
        options->observeAddedTime = time(&nowTime);
        COAP_AddOrUpdateToObservedResources(options,observerIdx);
    }
    else if(observeOptionValue == DEREGISTER)
    {
        coapObserveOptionRecvd = false;
        COAP_RemoveFromObservedResources(options->observedVertexIndex,observerIdx);
    }
    else
    {
        coapObserveOptionRecvd = 0;
    }
    
}

void COAP_RemoveObservedFromList(int16_t observedVertexIndex, uint32_t observerIPAddress)
{
    int16_t observerIdx = -1;
    ObserveServerParams_t op;
    observe_t o;
    int16_t index = 0;
    
    for(index = 0; index <MAX_OBSERVERS; index++)
    {
        graph_getObserveAtIndex(&o, index);
        obj_getObjectAtIndex(o.objIdx,&op);
        
        if(op.observerIPAddress == observerIPAddress)
        {            
            observerIdx =  o.objIdx;
            break;
        }
    }
    
    printf("remove ObserverIdx - %d \r\n",observerIdx);
    
    COAP_RemoveFromObservedResources(observedVertexIndex,observerIdx);
}

static int16_t InitObservers(ObserveServerParams_t *options)
{
    ObserveServerParams_t op;
    obj_index_t observer_index_object_index;
    
    memset(&op,0,sizeof(op));
    memset(&op.observerToken,0,sizeof(op.observerToken));
    
    observer_index_object_index = obj_malloc(sizeof(ObserveServerParams_t),"RSRC");
    
    memcpy(&op,options,sizeof(op));     
    
    obj_setObjectAtIndex(observer_index_object_index, (const void *)&op);
    
    return observer_index_object_index;    
}

static int16_t COAP_AddOrUpdateToObservedResources(ObserveServerParams_t *options, int16_t obObjIdx)
{    
    int16_t ret = false;  
    ObserveServerParams_t op;    
    
    memset(&op,0,sizeof(op));
    
    
    //Add New Observer
    if(obObjIdx == -1)
    {
        printf("Add new Observer \r\n");
        obObjIdx = InitObservers(options);
        ret = graph_add_observe(obObjIdx);   
        
        printf("Ret %d \r\n",ret);
        
        if(ret != -1)
        {        
            //Link observer to the resource   
            ret = addObserverLinkToResource(options->observedVertexIndex,ret);            
        }
    }
    //Update the existing Observer Data
    else
    {        
        memcpy(&op,options,sizeof(op));
        obj_setObjectAtIndex(obObjIdx, (const void *)&op);
    }
    
    if(ret != -1)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    
    COAP_UpdateObserveOptionFlag(ret);
    
    return ret;
}

static void COAP_RemoveFromObservedResources(vertex_index_t observedVertexIdx, int16_t observerObjIndex)
{  
    int16_t observerIndex;
    if((observerIndex = removeObserverLinkFromResource(observedVertexIdx,observerObjIndex)) != -1)
    {
        printf("Remove Observe \r\n");
        obj_free(observerObjIndex);         
        graph_remove_observe(observerIndex);           
    }
}

static int16_t addObserverLinkToResource(vertex_index_t resourceIdx, int16_t observerIdx)
{
    vertex_t resource;
    
    graph_getVertexAtIndex(&resource,resourceIdx);
    
    for(uint8_t i = 0; i < MAX_OBSERVERS; i++)
    {
        printf("Indices - %d \r\n",resource.observeIndices[i]);
        resource.isObserved = 1;
        if(resource.observeIndices[i] == -1)
        {
            resource.observeIndices[i] = observerIdx;
            graph_setVertexAtIndex(&resource,resourceIdx);
            printf("Adding observerIdx at for resource-  %d %d  %d\r\n",i, resource.observeIndices[i],resourceIdx);
            return 0;
        }
    }
    return -1;
}

static int16_t removeObserverLinkFromResource(vertex_index_t resourceIdx, int16_t observerObjIdx)
{
    vertex_t resource;
    observe_t observer;
    observe_index_t ret = -1;
    
    graph_getVertexAtIndex(&resource,resourceIdx);
    
    
    for(uint16_t i = 0; i < MAX_OBSERVERS; i++)
    {        
        graph_getObserveAtIndex(&observer,resource.observeIndices[i]);
        if(observer.objIdx == observerObjIdx)
        {   
            printf("Before Removing observerIdx at -  %d %d \r\n",i, resource.observeIndices[i]);
            ret = resource.observeIndices[i];
//            resource.isObserved = -1;
            resource.observeIndices[i] = -1;
            graph_setVertexAtIndex(&resource,resourceIdx);
            printf("After Removing observerIdx at -  %d %d \r\n",i, resource.observeIndices[i]);            
            break;
        }
    }
    if(ret == -1)
    {
         resource.isObserved = -1;
         graph_setVertexAtIndex(&resource,resourceIdx);
    }
    return ret;
}

void COAP_NotifyObservers(void)
{ 
    vertex_t ObservedResource;
    observe_t observer;
    ObserveServerParams_t op;
    time_t timeNow;
    error_msg ret = ERROR;
    uint8_t observeCount = 0;
    
    
    
    for(vertex_index_t i = 0; i <graph_get_vertex_count(); i++)
    {
        graph_getVertexAtIndex(&ObservedResource,i);
        if(ObservedResource.observeFlag == 1)
        {
            printf("\r\n Start - *******************\r\n");
            printf("Observed Resource Found %d \r\n",i);
            for(uint8_t j =0; j < MAX_OBSERVERS; j++)
            {               
                if(ObservedResource.observeIndices[j] != -1) //TODO - Don't send notification to all clients
                {
                    observeCount = observeCount + 1;
                    time(&timeNow);
                    printf("Observed Resource Object Found %d \r\n",j);
                    graph_getObserveAtIndex(&observer,ObservedResource.observeIndices[j]);                    
                    obj_getObjectAtIndex(observer.objIdx,&op); 
                    COAP_makeNewObserveOtionValue();
                    printf("Got Observe params %lu , %lu\r\n",op.observeAddedTime,timeNow);
                     if(timeNow >= op.observeAddedTime + 3) //24 hours to seconds - 86400
                     {  
                        printf("Send CON Notify Packet Started \r\n");
                        ret = COAP_SendNotifyPacket(&op,CON_TYPE);
                        op.observeAddedTime = time(&timeNow);
                        obj_setObjectAtIndex(observer.objIdx,&op); 
                     }
                     else
                     {
                         printf("Send NON Notify Packet Started \r\n");
                         ret = COAP_SendNotifyPacket(&op,NON_TYPE);    
                     }
                }
            }
            printf("Observe Count %d \r\n",observeCount);
            if(observeCount==0)
            {
                ObservedResource.isObserved = -1;
            }
            if(ret == SUCCESS || observeCount == 0)
            {
                printf("Reset Observe Flag \r\n");
                ObservedResource.observeFlag = -1;
                graph_setVertexAtIndex(&ObservedResource,i);
                printf("\r\n End - *******************\r\n");
            }
        }
    } 
}

void ObserveCoAPOptions()
{
    COAP_AddObserveOption(); 
}

const OptionTypeHandler_t OptionsTableCoAPOptions[] = 
{
    {COAP_AddObserveOption}
};

void observePayload(int16_t resourceIdx,content_format_t acceptFormat)
{
    vertex_t resource;
    ObserveServerParams_t op;
    
    printf("Inside Observe payload \r\n");
    printf("Resource Idx inside Observe payload %d \r\n",resourceIdx);
    
    graph_getVertexAtIndex(&resource,resourceIdx);
    
    COAP_AddContentFormatOption(acceptFormat);
  
    COAP_EndofOptions();
    COAP_GetResourceData(resource.type,resource.objIdx,resourceIdx,acceptFormat);
}


const ObservePayload_t observePayloadTable[] =
{
    {observePayload}  
};


static error_msg COAP_SendNotifyPacket(ObserveServerParams_t *options,coapTypes messageType)
{
    error_msg ret = ERROR;     
    
    
    notifyPtr.destIP = options->observerIPAddress;
    strncpy(notifyPtr.clientToken,options->observerToken,COAP_TOKEN_SIZE); 
    notifyPtr.destPort = options->destPort;
    notifyPtr.srcPort = 5683;
    notifyPtr.observeNotifiers.isObserveServerFlag = 1;
    notifyPtr.observeNotifiers.observedVertexIndex = options->observedVertexIndex;
    notifyPtr.observeNotifiers.observedAcceptFormat = options->observedAcceptFormat;
    notifyPtr.clientOption = OptionsTableCoAPOptions;
    notifyPtr.observePayload = observePayloadTable;  
    printf("Send - Observe Payload Function Address %p \r\n",notifyPtr.observePayload);
    if(messageType == CON_TYPE)
    {
        ret = COAP_Transmit(&notifyPtr,messageType,CONTENT,1);
    }
    else
    {
         ret = COAP_Transmit(&notifyPtr,messageType,CONTENT,0);
    }
    if(ret == SUCCESS)
    {
        COAP_Send();    
    }   
    
    return ret;
}

/**********************************************************************************************************/

bool COAP_IsFreshestNotification(int16_t observingObjIdx)
{   
    ObserveClientParams_t ocp;
    time_t   currentTimeOfNotification; //T2
    uint32_t currentObserveOptionValue; //V2
    
    time(&currentTimeOfNotification);
    currentObserveOptionValue = COAP_getObserveOptionValue();
    
    obj_getObjectAtIndex(observingObjIdx,&ocp);
    
    /**(V1 < V2 and V2 - V1 < 2^23) or
        (V1 > V2 and V1 - V2 > 2^23) or
        (T2 > T1 + 128 seconds)
     **/
    if(((ocp.prevObserveOptionValue < currentObserveOptionValue) && ((currentObserveOptionValue - ocp.prevObserveOptionValue) < 2^23)) ||
       ((ocp.prevObserveOptionValue > currentObserveOptionValue) && ((ocp.prevObserveOptionValue - currentObserveOptionValue) > 2^23)) ||
       (currentTimeOfNotification > (ocp.prevTimeOfNotification + 128)))
    {        
        ocp.prevObserveOptionValue = currentObserveOptionValue;
        ocp.prevTimeOfNotification = currentTimeOfNotification;
        obj_setObjectAtIndex(observingObjIdx,&ocp);
        
        return true;
    }
    
    return false;    
}

static int16_t COAP_ProcessClientObserverOption(coapReceive_t *ptr, int16_t observingObjIdx)
{    
    ObserveClientParams_t ocp; 
    int16_t ret = -1;
    
    obj_getObjectAtIndex(observingObjIdx,&ocp);
    ret = ocp.resourceIDLinked;
    return ret;
   
 
    
    /*Needs double check - 
     * ACK - 2.05 - Checks for message ID , removes from the client List and if not present - send error response (RST message)
     * NON - 2.05 - Checks for Token , IP address match and if not present - send error response (RST message)
     */
    
//    ret = COAP_IsEndpointMessageIDPairMatched(ptr->messageID);    
//        
//    
//    if((ret == false) && observingObjIdx == -1) 
//    {
//        return -1;
//    }
//    else
//    {
//        if(coapObserveOptionRecvd == true)
//        {
//            //DO Something
//            ret = observingIdx;
//        }
//        else
//        {
//            COAP_RemoveFromObservingResources(observingIdx,observingObjIdx);
//        }
//    }
 
}

static int16_t COAP_checkEndpointTokenPairAndGetObserveringIDFromList(uint32_t endpointAddress, char *token)
{    
    observe_t o;
    ObserveClientParams_t ocp;
    uint16_t index = 0;   
    
    for(index = 0; index <MAX_OBSERVERS; index++)
    {
        graph_getObserveAtIndex(&o, index);
        obj_getObjectAtIndex(o.objIdx,&ocp);
        
        if(ocp.observeClient.destIP == endpointAddress && (strncmp(ocp.observeClient.clientToken,token,COAP_TOKEN_SIZE) == 0))
        {            
            return o.objIdx;
        }
    }    
    return -1;  
}

error_msg COAP_RegisterResource(char *resourceAssoicatedWith,coapTransmit_t *ocp)
{
    error_msg ret = ERROR;   
    int16_t observingIdx;  
    
    observeOptionValue = REGISTER;
    
    registerPtr.resourceIDLinked = graph_get_vertex_idx_byName(resourceAssoicatedWith);    
    if(registerPtr.resourceIDLinked == -1)
        return ret;
    
    printf("Register \r\n");        
    
       NOP();
       NOP();
       NOP();
    ret = COAP_Transmit(ocp,NON_TYPE,GET,0);     
    
    if(ret==SUCCESS)
    {    
        memcpy(&registerPtr.observeClient,ocp,sizeof(coapTransmit_t));
        observingIdx = COAP_checkEndpointTokenPairAndGetObserveringIDFromList(ocp->destIP,ocp->clientToken);      
        printf("Register - observing Idx %d \r\n",observingIdx);
        COAP_EndofOptions();
        COAP_Send();
        COAP_AddOrUpdateToObservingResources(&registerPtr,observingIdx); 
        return ret;     
    }    
    ret = ERROR;    

    return ret;
}

error_msg COAP_DeRegisterResource(char *resourceAssoicatedWith,coapTransmit_t *ocp)
{
    error_msg ret = ERROR;   
    int16_t observingIdx;  
    
    observeOptionValue = DEREGISTER;
    
    registerPtr.resourceIDLinked = graph_get_vertex_idx_byName(resourceAssoicatedWith);    
    
    printf("DeRegister \r\n");  
    
    
    ret = COAP_Transmit(ocp,NON_TYPE,GET,0);     
    
    if(ret==SUCCESS)
    {    
        memcpy(&registerPtr.observeClient,ocp,sizeof(coapTransmit_t));
        observingIdx = COAP_checkEndpointTokenPairAndGetObserveringIDFromList(ocp->destIP,ocp->clientToken);      
        COAP_EndofOptions();
        COAP_Send();     
        return ret;
    }   
}

static int16_t InitObserveringResources(ObserveClientParams_t *params)
{
    ObserveClientParams_t op;
    obj_index_t observer_index_object_index;
    
    memset(&op,0,sizeof(op));
    memset(&op.observeClient.clientToken,0,sizeof(op.observeClient.clientToken));
    
    observer_index_object_index = obj_malloc(sizeof(ObserveClientParams_t),"RSRC");
    
    memcpy(&op,params,sizeof(op));     
    
    obj_setObjectAtIndex(observer_index_object_index, (const void *)&op);
    
    return observer_index_object_index;    
}

static int16_t COAP_AddOrUpdateToObservingResources(ObserveClientParams_t *params, int16_t ocpObjIdx)
{    
    int16_t ret = false;     
    ObserveClientParams_t ocp;    
    
    memset(&ocp,0,sizeof(ocp));
    
    //Add New Observing Resource
    if(ocpObjIdx == -1)
    {
        printf("Add New Observing Resource \r\n");
        ocpObjIdx = InitObserveringResources(params);
        ret = graph_add_observe(ocpObjIdx);   
        
        printf("Ret %d \r\n",ret);
        printf("Observe Object %d \r\n",ocpObjIdx);
        
        if(ret != -1)
        {        
            //Link observer to the resource   
//            ret = addObserverLinkToResource(options->observedVertexIndex,ret);            
        }
    }
    //Update the existing Observing Data
    else
    {        
        memcpy(&ocp,params,sizeof(ocp));
        obj_setObjectAtIndex(ocpObjIdx, (const void *)&ocp);
    }
    
    if(ret != -1)
    {
        ret = true;
    }
    else
    {
        ret = false;
    }  
    
    return ret;
}

static void COAP_RemoveFromObservingResources(vertex_index_t observingVertexIdx, int16_t observingObjIndex)
{  
    if(observingObjIndex != -1)
    {
        printf("Remove Observing Resource \r\n");
        obj_free(observingObjIndex);         
        graph_remove_observe(observingVertexIdx);           
    }
}

//static int16_t addObservingLinkToResource(vertex_index_t resourceIdx, int16_t observerIdx)
//{
//    vertex_t resource;
//    
//    graph_getVertexAtIndex(&resource,resourceIdx);
//    
//    for(uint8_t i = 0; i < MAX_OBSERVERS; i++)
//    {
//        printf("Indices - %d \r\n",resource.observeIndices[i]);
//        resource.isObserved = 1;
//        if(resource.observeIndices[i] == -1)
//        {
//            resource.observeIndices[i] = observerIdx;
//            graph_setVertexAtIndex(&resource,resourceIdx);
//            printf("Adding observerIdx at for resource-  %d %d  %d\r\n",i, resource.observeIndices[i],resourceIdx);
//            return 0;
//        }
//    }
//    return -1;
//}
//
//static int16_t removeObservingLinkFromResource(vertex_index_t resourceIdx, int16_t observerObjIdx)
//{
//    vertex_t resource;
//    observe_t observer;
//    observe_index_t ret = -1;
//    
//    graph_getVertexAtIndex(&resource,resourceIdx);
//    
//    
//    for(uint16_t i = 0; i < MAX_OBSERVERS; i++)
//    {        
//        graph_getObserveAtIndex(&observer,resource.observeIndices[i]);
//        if(observer.objIdx == observerObjIdx)
//        {   
//            printf("Before Removing observerIdx at -  %d %d \r\n",i, resource.observeIndices[i]);
//            ret = resource.observeIndices[i];
////            resource.isObserved = -1;
//            resource.observeIndices[i] = -1;
//            graph_setVertexAtIndex(&resource,resourceIdx);
//            printf("After Removing observerIdx at -  %d %d \r\n",i, resource.observeIndices[i]);            
//            break;
//        }
//    }
//    if(ret == -1)
//    {
//         resource.isObserved = -1;
//         graph_setVertexAtIndex(&resource,resourceIdx);
//    }
//    return ret;
//}
