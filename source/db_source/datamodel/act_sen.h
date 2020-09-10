/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    act_sen.h

  Summary:
    This is the header file for the act_sen.c

  Description:
    This header file provides the API for the Actuator_Sensor Resource.

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

#ifndef ACTUATOR_SENSOR_H
#define	ACTUATOR_SENSOR_H

#include "coapMethods.h"
#include "data_model.h"
#include "string_heap.h"

typedef struct
{
    string_index_t name;//n    
}act_sen_eeprom_t;
    
   
    
int8_t readFromEthActuatorSensoData(int16_t objIdx, menu_items_t mid, act_sen_eeprom_t * p);

void localSensorsInit(void);

extern bool queryNHandler   (int16_t,uint32_t query_value);
extern bool queryUHandler   (int16_t,uint32_t query_value);
extern bool queryVHandler   (int16_t,uint32_t query_value);
extern bool querySvHandler  (int16_t,uint32_t query_value);
extern bool queryBvHandler  (int16_t,uint32_t query_value);
extern bool querySHandler   (int16_t,uint32_t query_value);
extern bool queryTHandler   (int16_t,uint32_t query_value);
extern bool queryClHandler  (int16_t,uint32_t query_value);
extern bool queryMHandler   (int16_t,uint32_t query_value);
extern bool queryAHandler   (int16_t,uint32_t query_value);
extern bool queryCaHandler  (int16_t,uint32_t query_value);

#endif	/* ACTUATOR_SENSOR_H */

