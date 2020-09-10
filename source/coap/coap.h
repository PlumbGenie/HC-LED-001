/**
  CoAP Protocol Header file

  Company:
    Microchip Technology Inc.

  File Name:
    coap.h

  Summary:
    Header file for CoAP protocol implementation.

  Description:
    This header file provides the API for the CoAP protocol.

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

#ifndef TCPIP_COAP_H
#define TCPIP_COAP_H



#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "tcpip_types.h"
#include "data_model.h"
#include "contentFormat.h"



//#define ERROR                           (0)  //jira: CAE_MCU8-5647
//#define SUCCESS                         (1)  //jira: CAE_MCU8-5647
#define COAP_TOKEN_SIZE                 (8u)  //jira: CAE_MCU8-5647

// Seconds
#define EXCHANGE_LIFETIME               (247)
#define ACK_TIMEOUT                     (2u)  //jira: CAE_MCU8-5647
#define ACK_RANDOM_FACTOR               (1.5)
#define MAX_RETRANSMIT                  (4)
#define NSTART                          (1)
#define DEFAULT_LEISURE                 (5)
#define MAX_TRANSMIT_SPAN               (45)
#define NON_LIFETIME                    (145)
// byte/second
#define PROBING_RATE                    (1)

#define NUMBER_OF_SUPPORTED_CLIENTS     (2)
#define COAP_SERVER_SRC_PORT            (5683)
#define COAP_CLIENT_SRC_PORT            (65520)
#define COAP_CLIENT_DEST_PORT           COAP_SERVER_SRC_PORT

typedef void (*OptionHandler_fuction_ptr)(void);
typedef void (*payloadHandler_fuction_ptr)(void);
typedef void (*observePayloadHandler_fuction_ptr)(int16_t vIdx,content_format_t acceptFormat);

extern uint8_t resourceList[MAXVERTEXCOUNT];
typedef struct
{
    OptionHandler_fuction_ptr addOptions;
}OptionTypeHandler_t;

typedef struct
{    
    payloadHandler_fuction_ptr addPayload;
}payload_t;

typedef struct
{    
    observePayloadHandler_fuction_ptr addObservePayload;
}ObservePayload_t;

typedef enum
{
    CON_TYPE =0,
    NON_TYPE =1,
    ACK_TYPE =2,
    RST_TYPE =3    
}coapTypes;

typedef enum
{
    EMPTY=0,
    GET =1,
    POST =2,
    PUT =3,
    DELETE =4, 
    PATCH =5,
    CREATED=65,      //Code = 2.01
    DELETED =66,    //Code = 2.02
    VALID = 67,     //Code = 2.03
    CHANGED = 68,   //Code = 2.04
    CONTENT = 69,   //( Code = 2.05 => [2*32+5] = 69 )
    CONTINUE = 95,

    // Client error
    BAD_REQUEST = 128,
    UNAUTHORIZED = 129,
    BAD_OPTION = 130,
    FORBIDDEN = 131,
    NOT_FOUND = 132,
    METHOD_NOT_ALLOWED = 133, //Code = 4.05
    NOT_ACCEPTABLE =134,
    REQUEST_ENTITY_INCOMPLETE = 136,
    PRECONDITION_FAILED = 140,
    REQUEST_ENTITY_TOO_LARGE = 141,
    UNSUPPORTED_CONTENT_FORMAT = 143,

    // Server error
    INTERNAL_SERVER_ERROR =160,
    NOT_IMPLEMENTED = 161,
    BAD_GATEWAY = 162,
    SERVICE_UNAVAILABLE = 163,
    GATEWAY_TIMEOUT = 164,
    PROXY_NOT_SUPPORTED = 165
}coapCodes;


typedef struct
{
    unsigned            tkl:4;
    unsigned            t:2;
    unsigned            ver:2;
    uint8_t             code;    
}coapHeader_t;

typedef union
{
    uint8_t             length_8;
    uint16_t            length_16;
}Length;

typedef struct
{
    uint8_t             type;
    Length              len;
    char                opt_value[MAX_WORD_SIZE];
}Option_t;

typedef struct
{
     uint8_t            name;
     uint32_t           value;    
}queryNameValuePair;

typedef struct
{
    bool                isObserveServerFlag;
    int16_t             observedVertexIndex;
    content_format_t    observedAcceptFormat;
}observeNotify_t;

typedef struct
{
    uint8_t             uri_host[7];
    uint8_t             uri_path[MAX_URI_PATH_COUNT];
    uint16_t            uri_port;  
    content_format_t    content_format;
    content_format_t    accept_format;
    queryNameValuePair  queryList[MAX_URI_QUERY_COUNT];    
    uint8_t             uripath_count;
    uint8_t             uriquery_count;
}Option_List;

typedef struct
{
    uint32_t            destIP;
    uint8_t             clientToken[COAP_TOKEN_SIZE];
    uint8_t             clientTokenSize;
    OptionTypeHandler_t *clientOption;
    payload_t           *clientPayload;
    ObservePayload_t    *observePayload;
    uint16_t            destPort;
    uint16_t            srcPort;
   observeNotify_t      observeNotifiers;   
}coapTransmit_t;

typedef struct
{
    uint16_t            messageID;
    char                token[COAP_TOKEN_SIZE];
    uint32_t            destIP;    
    uint8_t             code;
    uint8_t             type;
    uint16_t            destPort;      
    uint16_t            srcPort;  
    Option_List         optlist;
    uint8_t             opt_count;
    bool                payload_waiting;
    int16_t             payload_length;    
    coapTypes           retCode;    
}coapReceive_t;


void COAP_Init(void);
void COAP_Manage(void);
error_msg COAP_Transmit(coapTransmit_t *ptr,coapTypes Type, coapCodes Code, bool Retransmission);
void COAP_Discover(coapTransmit_t *ptr);
error_msg COAP_StartDiscover(void);
bool COAP_FreeEndpointMessageIDTokenPairIfMatched(uint32_t receviedIPAddress, uint16_t messageID, char *token, uint8_t tokenLength, coapTypes type);

bool COAP_IsEndpointTokenPairMatched(uint32_t receviedIPAddress, char *token);
void COAP_EndofOptions(void);
error_msg COAP_Send(void);
void COAP_InsertLastOptionValue(coapReceive_t *ptr,char *optionValue,uint16_t optionLength);
bool COAP_ParseOptions(coapReceive_t *ptr, int16_t length);
bool COAP_TraverseToResource(coapReceive_t *ptr);
void COAP_Receive(int16_t length);
bool COAP_MessageResponse(coapReceive_t *ptr);
bool COAP_ErrorResponse(coapReceive_t *ptr);
char * COAP_getReceivedClientToken(void);


void COAP_CL_deviceRegistry(int16_t length);

typedef enum
{
    Bad_Request			 					= 400,
    Unauthorized							= 401,
    Payment_Required						= 402,
    Forbidden								= 403,
    Not_Found								= 404,
    Method_Not_Allowed						= 405,
    Not_Acceptable							= 406,
    Proxy_Authentication_Required			= 407,
    Request_Timeout							= 408,
    Conflict								= 409,
    Gone									= 410,
    Length_Required							= 411,
    Precondition_Failed						= 412,
    Payload_Too_Large						= 413,
    Request_URI_Too_Long					= 414,
    Unsupported_Media_Type					= 415,
    Requested_Range_Not_Satisfiable			= 416,
    Expectation_Failed						= 417,
    Im_a_teapot                             = 418,
    Authentication_Timeout					= 419,
    Method_Failure							= 420,
    Enhance_Your_Calm						= 420,
    Misdirected_Request						= 421,
    Unprocessable_Entity					= 422,
    Locked									= 423,
    Failed_Dependency						= 424,
    Upgrade_Required						= 426,
    Precondition_Required					= 428,
    Too_Many_Requests						= 429,
    Request_Header_Fields_Too_Large			= 431,
    Login_Timeout							= 440,
    No_Response								= 444,
    Retry_With								= 449,
    Blocked_by_Windows_Parental_Controls	= 450,
    Unavailable_For_Legal_Reasons			= 451,
    Request_Header_Too_Large				= 494,
    Cert_Error								= 495,
    No_Cert									= 496,
    HTTP_to_HTTPS							= 497,
    Token_expired_invalid					= 498,
    Client_Closed_Request					= 499,
    Token_required							= 499
}COAPERRORS_t; // taken from HTTP

#endif	/* COAP_H */

