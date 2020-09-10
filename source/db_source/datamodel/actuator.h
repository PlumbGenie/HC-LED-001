/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    actuator.h

  Summary:
    This is the header file for the actuator.c

  Description:
    This header file provides the API for the Actuator Resource.

 */

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


#ifndef ACTUATOR_H
#define ACTUATOR_H


#include "act_sen.h"

typedef struct 
{
  const char name[SIZE_OF_QUERY_NAME_ENUM];
  menu_items_t id;
} actuator_lst;

typedef struct
{ 
    act_sen_eeprom_t act_eep_1;   
    string_index_t lightColor; //cl   
}actuator_eeprom_t;
    
 typedef struct
 {   
     int16_t resIndex;
 
     uint8_t percentPower;//pp
     uint8_t percentIntensity;//pi 
     char    overrideFlag[1];//or    
     uint16_t adjustRate;//ar 
     uint16_t adjustTime;//at
     uint16_t degKelvin; //oK
 }actuator_t;

extern actuator_t ar[MAX_ACTUATORS_COUNT];//Ram variables of the actuators


extern restfulMethodStructure_t actuatorMethods;

extern bool queryPpHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryPiHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryOrHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryArHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryAtHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryOkHandler(obj_index_t objIdx,uint32_t query_value);


extern bool actuatorUpdated;


int16_t initActuatorResource(void);
void localActuatorsInit(void);
error_msg deleteActuatorResource(obj_index_t objIdx);
void createSensorResourceID(uint8_t i);


typedef uint8_t (*ramcbordataFunctionPointer)(menu_items_t, char *);
typedef struct
{
    ramcbordataFunctionPointer actuatorCBORGetRamDataMethod;    
    ramcbordataFunctionPointer actuatorCBORPutRamDataMethod;    
}ramCborDataTable_t;

typedef uint8_t (*ramtextdataFunctionPointer)(menu_items_t, char *);
typedef struct
{
    ramtextdataFunctionPointer actuatorTEXTGetRamDataMethod;    
    ramtextdataFunctionPointer actuatorTEXTPutRamDataMethod;    
}ramTextDataTable_t;

int8_t readFromEthActuatorData(int16_t, menu_items_t , actuator_eeprom_t *);

uint8_t actuator_RamdataGetHandle(menu_items_t, char *read_ptr, int16_t actIdx);
int8_t actuator_RamdataPutHandle(menu_items_t, char *read_ptr, int16_t actIdx);


uint8_t actuator_TEXTramdataGetHandle(menu_items_t, char *read_ptr, int16_t actIdx);
uint8_t actuator_TEXTramdataPutHandle(menu_items_t, char *read_ptr, int16_t actIdx);


#endif /* defined(ACTUATOR_H) */
