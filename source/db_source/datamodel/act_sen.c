/**
  Actuator_Sensor Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    act_sen.c

  Summary:
     This is the implementation of Actuator_Sensor Resource.

  Description:
    This source file provides the implementation of the API for the Actuator_Sensor Resource.

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

#include "act_sen.h"
#include "object_heap.h"
#include "sensor.h"
#include "actuator.h"



int8_t readFromEthActuatorSensoData(int16_t objIdx, menu_items_t mid, act_sen_eeprom_t * p)
{
     char read_ptr[MAX_WORD_SIZE];
    int8_t len =0;    
    
    memset(read_ptr,0,sizeof(read_ptr));
    switch(mid)
    {
        case NAME:
            len += parseData(read_ptr,sizeof(read_ptr));
            p->name = string_insertWord(read_ptr,p->name);
            CfElements.array_map_elements--;
            break;
        default:
            CfElements.array_map_elements--;
            len = -1;
            break;  
    }
    return len;
}

/**************************************************************Query Handlers**************************************************************************/

bool queryNHandler(obj_index_t objIdx,uint32_t query_value)
{     
     bool ret = true;
     act_sen_eeprom_t p;        
     
     if(n_query==0)
     {
         obj_getObjectAtIndex(objIdx, (const void *)&p);
         
         if((uint16_t)p.name == query_value)  //jira: CAE_MCU8-5647
         {           
             n_query = 1;
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

bool queryClHandler(obj_index_t objIdx,uint32_t query_value)
{     
     bool ret = false;
     actuator_eeprom_t p;
     sen_eeprom_t q;
     
     if(cl_query==0)
     {
        for(uint8_t i=0; i<act_sen_count.actuatorCount; i++)
         {
             if(ar[i].resIndex==objIdx)
             {
                 obj_getObjectAtIndex(objIdx, (const void *)&p);
                 
                 if((uint16_t)p.lightColor == query_value)   //jira: CAE_MCU8-5647
                 {
                    cl_query = 1;
                    ret = true;
                    break;
                 }                 
             }
         }
                           
             for(uint8_t i=0; i<act_sen_count.sensorCount; i++)
             {
                 if(sen[i].resIndex == objIdx)
                 {
                     obj_getObjectAtIndex(objIdx, (const void *)&q);
                     if((uint16_t)q.class== query_value)   //jira: CAE_MCU8-5647
                     {                
                         cl_query = 1;
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
     
   
