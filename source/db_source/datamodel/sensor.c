/**
  Sensor Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    sensor.c

  Summary:
     This is the implementation of Sensor Resource.

  Description:
    This source file provides the implementation of the API for the Sensor Resource.

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

#include "sensor.h"
#include "tcpip_types.h"
#include "object_heap.h"
//#include "coap_contentFormat.h"
#include "graph.h"
#include "act_sen.h"
#ifdef IF_IO_STARTER_EXPANSION 
#include "at30ts74_temperature_sensor.h"
#include "light_sensor.h"
#endif

#define SENSOR_ELEMENTS 7
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

obj_index_t sensor_index_object_index;
sen_t sen[MAX_SENSORS_COUNT];
static void sensor_ramDataGetHandle(menu_items_t mid, char *read_ptr,int16_t senIdx);

const sensor_lst sen_items_list[1] = {
  { "n", NAME } 
};


void createSensorResourceID(uint8_t i)
{             
}

int16_t initSensorResource(void)
{  
    sen_eeprom_t se;   
    memset(&se,0,sizeof(se));
    sensor_index_object_index = obj_malloc(sizeof(sen_eeprom_t),"RSRC");
    
   
    se.sen_eep_1.name = string_insertWord("Sensor",se.sen_eep_1.name); 
    se.multiplier=10;    
    se.units = string_insertWord("K",se.units);  
    se.class = string_insertWord("color",se.class);  
    
    obj_setObjectAtIndex(sensor_index_object_index, (const void *)&se);
     
    return sensor_index_object_index;

}

error_msg sensorGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = SUCCESS;
    sen_eeprom_t p; 
    char *read_ptr = NULL;      //jira: CAE_MCU8-5647
    menu_items_t mid = UNKNOWN; //jira: CAE_MCU8-5647
    static uint8_t sensorElements;
    char buf[8]; //jira: CAE_MCU8-5647
    
    sensorElements = SENSOR_ELEMENTS;
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    /**** Use Encode to write data to the buffer ****/
#ifdef IF_IO_STARTER_EXPANSION 
    for( uint16_t i =0; i< MAX_SENSORS_COUNT; i++)
     {
        if(sen[i].resIndex == objIdx)
        {
            if(i == 1)
            {
                //Temp Sensor in Celsius
                p.sen_eep_1.name = string_insertWord("TempSensor",p.sen_eep_1.name);
                p.units = string_insertWord("C", p.units);
            }
            else if(i == 0)
            {
                //Light Sensor
                 p.sen_eep_1.name = string_insertWord("LightSensor",p.sen_eep_1.name);
                  p.units = string_insertWord("Ee", p.units);
            }
        }
    }
#endif
    writeAuxData(&sensorElements,MAP);    
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    Get_ToEthernet((char *)"n",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.sen_eep_1.name),TEXT_STRING);
    
    Get_ToEthernet((char *)"cl",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.class),TEXT_STRING);
    
    Get_ToEthernet((char *)"u",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.units),TEXT_STRING);
    
    Get_ToEthernet((char *)"m",TEXT_STRING); 
    itoa(buf,p.multiplier,10); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER8); //jira: CAE_MCU8-5647
    
   
    
    if(getSenCount())
    {
        for( uint16_t i =0; i< MAX_SENSORS_COUNT; i++)
        {
            if(sen[i].resIndex == objIdx)
            {
                sensor_ramDataGetHandle(mid,read_ptr,i);
                ret=SUCCESS;
                break;
            }
        }
    }      
    return ret;    
}

error_msg sensorPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret =  ERROR;    
    
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;
    uint8_t i;
    menu_items_t mid;
    const sensor_lst *choice = NULL;
    choice = sen_items_list;
    uint8_t payload_length;
    sen_eeprom_t p;   
    int8_t temp_len = -1;
    
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    payload_length = PayloadLength;
    
    while(payload_length)
    {
        memset(read_ptr,0,sizeof(read_ptr));
        len =0;
        len += parseData(read_ptr,sizeof(read_ptr));
        
        for(i = 0, choice = NULL; i < 4; i++)
        {
            if (strncmp(read_ptr, sen_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0)
            {
                choice = sen_items_list + i;
                break;
            }
        }           
        memset(read_ptr,0,sizeof(read_ptr));
        mid = choice ? choice->id : UNKNOWN;        
        switch(mid)
        {           
            default:
                temp_len = readFromEthActuatorSensoData(objIdx, mid, &p.sen_eep_1);
                if(temp_len != -1)
                {
                    len += (uint8_t)temp_len; //jira: CAE_MCU8-5647
                    ret = SUCCESS;
                    break;
                }
                else
                    return ERROR;
                
        }      
        payload_length -= len;
    }
    obj_setObjectAtIndex(objIdx, (const void *)&p);     
   
    return ret;
    
}

static void sensor_ramDataGetHandle(menu_items_t mid, char *read_ptr,int16_t senIdx)
{
    char buf[8];  //jira: CAE_MCU8-5647
 #ifdef IF_IO_STARTER_EXPANSION    
    if(senIdx == 1)
    {
        //Temp Sensor in Celsius
       sen[(uint16_t)senIdx].value = (uint16_t)AT30TS74_ReadTemperature(); //jira: CAE_MCU8-5647
    }
    else if(senIdx == 0)
    {
        //Light Sensor
         sen[(uint16_t)senIdx].value =  ReadLightSensor(); //jira: CAE_MCU8-5647
    }
#endif
    Get_ToEthernet((char *)"v",TEXT_STRING); 
    itoa(buf,sen[(uint16_t)senIdx].value,10 ); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER32); //jira: CAE_MCU8-5647
    
    Get_ToEthernet((char *)"t",TEXT_STRING); 
    itoa(buf,sen[(uint16_t)senIdx].time,10 ); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER32); // - Change to time encoding       //jira: CAE_MCU8-5647
}



error_msg sensorPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}


error_msg sensorPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{

    error_msg ret = ERROR;
     
    return ret;
}


error_msg sensorDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)

{
    sen_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.class;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.sen_eep_1.name;
    ret = string_removeWord(stringIdxToBeRemoved);   
    
    stringIdxToBeRemoved = p.units;
    ret = string_removeWord(stringIdxToBeRemoved);

    return ret;
}

void localSensorsInit(void)
{
    uint16_t j=0,i=0,vCount;
    vertex_index_t idx;
    vertex_t v;

    
    vCount=getSenCount();

    if (graph_get_vertex_count() && vCount)
    {
        for(i = 0;i < MAXVERTEXCOUNT;i ++)
        {   
            if(vCount != 0)
            {
                idx=graph_get_vertex_idx_verified_byType(SENSOR_TYPE,i);

                if(idx != -1)
                {
                    graph_getVertexAtIndex(&v,idx);
 
                    if(v.objIdx > 0)// 0 is non data type and non datatype should be ignored
                    {
                        restfulHandler[SENSOR_TYPE]->ramInitializer((void*)&sen[j],v.objIdx);
                        j++;
                        vCount--;
                    }
                }
            }
            else
                break;

        }
    }
}

error_msg sensorInit(void *rdata, obj_index_t objIdx)
{
   sen_t *p;

    p = (sen_t *)rdata;
    p->resIndex = objIdx;   
    p->time = 1495822297;
    p->value=100;
   
    return SUCCESS;
}




restfulMethodStructure_t sensorMethods = {sensorInit,sensorGetter,sensorPutter,sensorPatcher,sensorDeleter,sensorPoster,initSensorResource};


/******************************Queries*/

bool queryUHandler(obj_index_t objIdx,uint32_t query_value)
{     
     bool ret = false;
     sen_eeprom_t p;
     
     if(u_query==0)
     {
         obj_getObjectAtIndex(objIdx, (const void *)&p);
         
         if((uint16_t)p.units == query_value) //jira: CAE_MCU8-5647
         {           
             u_query = 1;
             ret = true;
         }
         else
         {            
             ret = false;
         }
     }
    else
         ret =  true;
     
    return ret;       
}

bool queryVHandler(obj_index_t objIdx,uint32_t query_value)  
{     
     bool ret = false;
//     sen_t p;  //jira: CAE_MCU8-5647
     
     if(v_query==0)
     {
         for(uint8_t i=0; i<act_sen_count.sensorCount; i++)
         {
             if(sen[i].resIndex == objIdx)
             {
                 if((uint32_t)sen[i].value== query_value)  //jira: CAE_MCU8-5647
                 {                
                     v_query = 1;
                     ret = true;
                     break;
                 }
             }
         }           
     }    
    else
         ret =  true;
     
    return ret;       
}

bool queryMHandler(obj_index_t objIdx,uint32_t query_value)
{     
     bool ret = false;
     sen_eeprom_t p;
     
     if(m_query==0)
     {
         obj_getObjectAtIndex(objIdx, (const void *)&p);
         
         if(p.multiplier == query_value)
         {            
             m_query = 1;
             ret = true;          
         }
         else
         {
             ret = false;
         }
     }
    else
         ret =  true;
     
    return ret;       
}



bool queryTHandler(obj_index_t objIdx,uint32_t query_value)
{     
     bool ret = false;
//     sen_t p; //jira: CAE_MCU8-5647
     
     if(t_query==0)
     {
         for(uint8_t i=0; i<act_sen_count.sensorCount; i++)
         {
             if(sen[i].resIndex == objIdx)
             {
                 if((uint32_t)sen[i].time== query_value)  //jira: CAE_MCU8-5647
                 {                
                     t_query = 1;
                     ret = true;
                     break;
                 }
             }
         }
     }    
     else
         ret =  true;
     
    return ret;       
}




