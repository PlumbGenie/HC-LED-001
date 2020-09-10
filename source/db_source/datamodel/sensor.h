/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    sensor.h

  Summary:
    This is the header file for the sensor.c

  Description:
    This header file provides the API for the Sensor Resource.

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


#ifndef SENSOR_H
#define SENSOR_H

#include "act_sen.h" 

 typedef struct 
{
  const char name[SIZE_OF_QUERY_NAME_ENUM];
  menu_items_t id;
}sensor_lst;


 typedef struct 
 {
    int16_t resIndex;
     
    int32_t value;//v    //jira: CAE_MCU8-5647
    time_t time;//t 
 }sen_t;
 
 typedef struct
{
    act_sen_eeprom_t sen_eep_1;
    string_index_t class;//cl
    string_index_t units;//u
    uint8_t multiplier; //m  
}sen_eeprom_t;

extern restfulMethodStructure_t sensorMethods;

extern sen_t sen[MAX_SENSORS_COUNT];

void createSensorResourceID(uint8_t i);

int16_t initSensorResource(void);

#endif /* defined(SENSOR_H) */
