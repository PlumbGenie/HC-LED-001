/**
  Data Model Configuration file
	
  Company:
    Microchip Technology Inc.

  File Name:
    data_model.h

  Summary:
    Header file for Data Model User configuration options

  Description:
    This header file provides the Data Model User configuration options.

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

#ifndef data_model_h
#define data_model_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "uuid.h"
#include "object_heap.h"
#include "tcpip_types.h"
#include "contentFormat.h"


/*********************************************************************************************
 * The Database [Heap] Configuration
 *     MAXVERTEXCOUNT / MAXEDGES / MAXQUERYCOUNT / SIZE_OF_RESOURCEID / MAX_WORD_SIZE
 *          - The memory allocated in heap. 
 * 
 ********************************************************************************************/
#define MAXVERTEXCOUNT                      (75)
#define MAXEDGES                            (75)
#define MAXQUERYCOUNT                       (32)
#define SIZE_OF_RESOURCEID                  (5)
#define MAX_WORD_SIZE                       (32)


/*********************************************************************************************
 * CoAP Configuration
 *     Configurable CoAP Features
 * 
 *********************************************************************************************/
#define DEFAULT_CONTENT_FORMAT              CBOR
#define MAX_URI_PATH_COUNT                  (5)
#define MAX_URI_QUERY_COUNT                 (5)

/**********************************************************************************************
 * DataModel Configuration
 *   Select the Main Resources
 *          - ACTUATORS and/or SENSORS
 *              For Only ACTUATORS               - Comment #define SENSORS
 *              For Only SENSORS                 - Comment #define ACTUATORS
 *              For both Actuators and Sensors   - Uncommnet #define ACTUATORS and #define SENSORS
 *          - Enter the number of Actuators
 *          - Enter the number of Sensors
 * 
 *  MAX_CONTEXT_KEYWORDS -  The Maximum Keywords supported by each Main resource [Actuators and/or Sensors ]
 * 
 **********************************************************************************************/
#define    ROOT_DIRECTORY               "mchp"
#define    ACTUATORS                    "actuators"
#define    SENSORS                      "sensors"
#define    INVENTORY                    "inventory"
#define    CONTEXT                      "context"
#define    LOCATION                     "location"
#define    NETWORK                      "network"
#define    IDENTITY                     "identity"
#define    DFU                          "DFU"
#define    DFD                          "DFD"


#define   MAX_ACTUATORS_COUNT             (15)
#define   MAX_SENSORS_COUNT               (5)

#define   MAX_CONTEXT_KEYWORDS            (5)

/*********************************************************************************************
 *  DataModel Configuration
 *   Configurable "RESOURCE_TYPES" enum - MAY add a Resource type
 *   Do not remove NONRESOURCE_TYPE
 *********************************************************************************************/
typedef struct
{
    uint8_t actuatorCount;
    uint8_t sensorCount;
}act_sen_count_t;

extern act_sen_count_t act_sen_count;

typedef enum
{
    MY_FAV_RESOURCE,
    DFD_TYPE,
    DFU_TYPE,
    CONTEXT_TYPE,
    ACTUATOR_TYPE,
    SENSOR_TYPE,
    IDENTITY_TYPE,
    NETWORK_TYPE,
    LOCATION_TYPE,
    INVENTORY_TYPE,
    IP_TYPE,
    ACTUATOR_SENSOR_TYPE, //specifically for queries
    NONRESOURCE_TYPE=-1
}coapResourceTypes_t;

/*********************************************************************************************
 * DataModel Configuration
 *   Query Name Enums - MUST have same instance of enum  Query Name for each query
 *                    - The UNKNOWN enum MUST not be removed *                    
 *  Size of Query Name Enum - 8
 * 
 *********************************************************************************************/
#define SIZE_OF_QUERY_NAME_ENUM              (8u)  //jira: CAE_MCU8-5647
typedef enum {\
    UNKNOWN,NAME, UNITS, VALUE, STRVAL,\
    BOOLVAL, VALSUM, TIME, CLASS, MUL, ACC, CAL, COLOR,\
    PP, PI, OR, II, OK, AT, AR, APD,CN, MN, MF, MP,\
    ENAM, ECLA, AKEY, DOMN, IMPO, ROLE, KEYW, TFTP_SRV_ADD,\
    TFTP_FILE_NAME,TFTP_FIRM_REV,TIME_BEGIN,TFTP_BLOCK_SIZE,\
    MADT, MADR, MDNS,EDNS,IP, DNS, NTP,UPDATE_TIME} menu_items_t;
extern menu_items_t menu_items;
/*********************************************************************************************
 * DataModel Configuration
 *  MUST have same instance of the Queries Status - Need to be extern 
 * 
 *********************************************************************************************/

extern bool impo_query;
extern bool domn_query;
extern bool role_query;
extern bool keyw_query;

extern bool pp_query;
extern bool pi_query;
extern bool or_query;
extern bool at_query;
extern bool ar_query;
extern bool ok_query;

extern bool n_query;
extern bool u_query;
extern bool v_query;
extern bool t_query;
extern bool cl_query;
extern bool m_query;


extern bool enam_query;
extern bool ecla_query;
extern bool akey_query;

extern bool payl_query;

extern bool hwrv_query;
extern bool swrv_query;
extern bool fwrv_query;
extern bool snum_query;
extern bool manu_query;
extern bool modl_query;

/*********************************************************************************************
 * DataModel Configuration
 *  Resource ID - MUST have the same instance of the Resource ID
 * 
 * 
 *********************************************************************************************/

//

//extern const char act_id[SIZE_OF_RESOURCEID];
//extern const char sen_id[SIZE_OF_RESOURCEID];
//extern const char loc_id[SIZE_OF_RESOURCEID];
//extern const char con_id[SIZE_OF_RESOURCEID];
//extern const char net_id[SIZE_OF_RESOURCEID];
//extern const char idt_id[SIZE_OF_RESOURCEID];
//extern const char inv_id[SIZE_OF_RESOURCEID];
//
//extern const char none[SIZE_OF_RESOURCEID];
//extern const char dfd_id[SIZE_OF_RESOURCEID];
//extern const char dfu_id[SIZE_OF_RESOURCEID];


   
//void addActuatorResource(char *actName);
//void addSensorResource  (char *senName);
int16_t addActOrSenResource(char *, uint8_t);
int16_t addDFDResource     (void);
int16_t addDFUResource     (void);



/*Create Sensor ResourceID.
 * The function will create a unique Resource ID starting with "SEN" for each Sensors
 * 
 * @param void
 * 
 * @return void
 * 
 */
void createSensorResourceID(uint8_t i);

/*Create Context, Identity, Location, Network and Inventory  ResourceID.
 * The function will create a unique Resource ID starting with "CONT","IDT","LOC","NET" and "INV" respectively.
 * These are the sub resources for Actuators and Sensors
 * 
 * @param void
 * 
 * @return void
 * 
 */
void createSubResourceID(uint8_t i);


/*Create new Database
 * 
 * This function creates a new Database. 
 * All resources are organized in a Tree format. 
 * Also adds the query list which are supported by the data model.
 * Returns -1 for error //jira: M8TS-679
 */
int16_t createDatabase(void); //jira: M8TS-679


/*Create Resources
 * 
 * This function is used to create and allocate memory for main resources in the database  
 * 
 */
void initResourceUUID(void);



/*Resources Initialization
 * 
 * This function is used to Initialize the resources which are stored in RAM and are not part of database model
 */
void localResourceInit(void);



/*Resets ALL the Query Status to false 
 * The function resets the query Status. 
 * This function is required to reset all query list after each CoAP Packet. 
 * 
 */
extern void Query_Reset();



/*  - Remove UUID if not required*/
typedef struct
{       int16_t uuid;
}uuid_eeprom_t;

typedef enum
{
   NOT_DEFINED,
   E,
   UUID  
}dataModelEnumItems;

typedef struct 
{
  const char *name;
  dataModelEnumItems id;
} dataModelList_t;


extern uuid_eeprom_t uuid_eepr;
extern const char *uuid;
extern bool firstPutEntry;
extern char rsrc_uuid[UUID_STR_SIZE];
char *getResourceUuid(int16_t idx);

void initActSenCount(const char *asct);
void setActCount(uint8_t num);
void setSenCount(uint8_t num);
uint8_t getActCount(void);
uint8_t getSenCount(void);
uint16_t getSubActSenResourceCount(void);
error_msg removeThisResource(char *, uint8_t);
void setResourceUpdateFlags(obj_index_t objIdx);
void setNumberOfResourceCount(uint8_t count);
uint8_t getNumberOfResourceCount(void);
void addPayloadOverHead(obj_index_t objIdx, uint8_t numOfResources);
int16_t processPayloadOverHead( uint8_t receivedPayloadLength,content_format_t cfType);
void initDiscoveryLength(void);
void setDiscoveryLength(uint32_t num);
uint32_t getDiscoveryLength(void);
void COAP_CL_nameSpace(uint8_t elementCount);

#endif /* data_model_h */
