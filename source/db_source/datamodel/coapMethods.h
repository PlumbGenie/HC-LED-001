//
//  dataModels.h
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 9/23/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#ifndef mchp_graph_dataModels_h
#define mchp_graph_dataModels_h

#include <stdint.h>
#include "coap.h"
#include "contentFormat.h"


// CoAP restful resource handlers

typedef error_msg (*restfulFunctionPointer)(obj_index_t , int16_t, uint8_t);
typedef error_msg (*restfulFuntionInitRamPointer)(void *, obj_index_t);// double check  //jira: CAE_MCU8-5647
typedef obj_index_t (*restfulFunctionInitPointer)(void);
typedef bool (*restfulSupportCFFunctionPointer)(content_format_t);
typedef struct
{
    restfulFuntionInitRamPointer ramInitializer;
    restfulFunctionPointer getter;
    restfulFunctionPointer putter;
    restfulFunctionPointer patcher;
    restfulFunctionPointer deleter;
    restfulFunctionPointer poster;
//    restfulFunctionPointer content;
    restfulFunctionInitPointer initializer;  
}restfulMethodStructure_t;



typedef bool (*queryFunctionPointer)(obj_index_t,uint32_t);
typedef struct
{
    uint8_t query_num;
    queryFunctionPointer queryHandle;
}QueryHandle_t;

typedef error_msg (*responseGetPutFunctionPointer)(obj_index_t, coapReceive_t*,int16_t );

typedef struct
{
    content_format_t contentFormatType;
    responseGetPutFunctionPointer GetbyFormat;
    responseGetPutFunctionPointer PutbyFormat;

}get_put_byformat_handler_t;

extern const restfulMethodStructure_t *restfulHandler[11];
extern const QueryHandle_t queryHandler[23];


#endif
