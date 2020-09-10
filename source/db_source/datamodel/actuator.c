/**
  Actuator Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    actuator.c

  Summary:
     This is the implementation of Actuator Resource.

  Description:
    This source file provides the implementation of the API for the Actuator Resource.

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
#include "actuator.h"
#include "tcpip_types.h"
//#include "act_sen.h"
#include "epwm1.h"
#include "gestic.h"
#include "curve.h"
#include "contentFormat.h"
#include "graph.h"


actuator_t ar[MAX_ACTUATORS_COUNT];
//bool actuatorUpdated = false;  //jira: CAE_MCU8-5647

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

#define separator ','


#define ACTUATOR_ELEMENTS 9


//obj_heap_ptr_t actuator_index_object;  //jira: CAE_MCU8-5647
obj_index_t actuator_index_object_index;
int8_t readFromEthActuatorData(int16_t, menu_items_t , actuator_eeprom_t *);

const uint16_t Curve[CURVE_STEPS]={
 0, 2, 4, 6, 8, 10, 12, 14, 16, 18,
 20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
 42, 46, 48, 52, 56, 60, 64, 68, 72, 76,
 80, 84, 88, 92, 96, 100, 104, 108, 112, 116,
 120, 124, 128, 132, 136, 140, 144, 148, 152, 156,
 162, 168, 174, 180, 186, 192, 198, 204, 210, 216,
 223, 230, 237, 244, 251, 258, 265, 272, 279, 286,
 294, 302, 310, 318, 326, 334, 342, 350, 358, 366,
 380, 394, 408, 422, 436, 464, 478, 492, 506, 520,
 550, 580, 610, 640, 670, 700, 730, 760, 790, 810,
 1023
};

const actuator_lst act_eeprom_items_list[2] = {
    { "n", NAME},    
    { "lc", COLOR}    
};

const actuator_lst act_ram_items_list[6] = {
    {"pp", PP},    
    {"pi", PI},
    {"or", OR},
    {"ar", AR},
    {"at", AT},
    {"oK", OK}    
};


int16_t initActuatorResource(void)
 {
    actuator_eeprom_t ae;
    memset(&ae,0,sizeof(ae));
    actuator_index_object_index = obj_malloc(sizeof (actuator_eeprom_t), "RSRC");    
   
    ae.act_eep_1.name = string_insertWord("SomeLight",ae.act_eep_1.name);   
    ae.lightColor = string_insertWord("AABBCCDD",ae.lightColor);  
    

    obj_setObjectAtIndex(actuator_index_object_index, (const void *) &ae);

    return (actuator_index_object_index);
}

error_msg deleteActuatorResource(obj_index_t objIdx)
{
    obj_free(objIdx); 
    return SUCCESS;
}

void localActuatorsInit(void) 
{
    uint16_t i=0,j=0;
    vertex_index_t idx;
    vertex_t v;
    uint16_t vCount;

    vCount=getActCount();

    
    if(graph_get_vertex_count() && vCount)
    {

        for(i = 0; i < MAXVERTEXCOUNT ;i ++)
        {   
            if(vCount != 0)
            {
                idx=graph_get_vertex_idx_verified_byType(ACTUATOR_TYPE,i);


                if(idx != -1)
                {
                    graph_getVertexAtIndex(&v,idx);

                    if(v.objIdx > 0)//non datatype should be ignored
                    {
                        restfulHandler[ACTUATOR_TYPE]->ramInitializer((void*)&ar[j],(uint8_t)v.objIdx); //jira: CAE_MCU8-5647
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

error_msg actuatorInit(void *rdata, obj_index_t objIdx) {
    actuator_t *p;

    p = (actuator_t *) rdata;
    p->percentPower = 100;   
    p->percentIntensity = 100;   
    strncpy(p->overrideFlag,"N",1);
    p->adjustRate = 0;
    p->adjustTime = 0;
    p->degKelvin = 3000;
    p->resIndex=objIdx; 

    return SUCCESS;

}

error_msg actuatorGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = SUCCESS;
    actuator_eeprom_t p;
    char *read_ptr = NULL;
    static uint8_t actuatorElements;   
    
    actuatorElements = ACTUATOR_ELEMENTS;
    
    

    obj_getObjectAtIndex(objIdx, (const void *) &p);
    writeAuxData(&actuatorElements,MAP);
    

    Get_ToEthernet((char *) "uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);   
    
    Get_ToEthernet((char *)"n",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.act_eep_1.name),TEXT_STRING);    
   

    if(getActCount())
    {   
        for (uint16_t i = 0; i <MAX_ACTUATORS_COUNT; i++)
        {
            if (ar[i].resIndex == objIdx)
            {               
                actuator_RamdataGetHandle(UNKNOWN, read_ptr, i);
                ret = SUCCESS;
                break;
            }
        }
    }
    
    Get_ToEthernet((char *)"lc",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p.lightColor),TEXT_STRING);
    
    return ret;
}

error_msg actuatorPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = SUCCESS;
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len = 0, len1 = 0, len2 = 0;
    int8_t temp_len = -1;
    menu_items_t mid;
//    bool found = false; //jira: CAE_MCU8-5647
    const actuator_lst *choice = NULL;
    const actuator_lst *act_choice = NULL;
    actuator_eeprom_t p;    
    uint8_t payload_length;    
    static bool iseepromread = false;

    memset(&p, 0, sizeof (actuator_eeprom_t));
 

    payload_length = PayloadLength;
    while (payload_length) {
        len = 0;
        len1 = 0;
        len2 = 0;
        memset(read_ptr, 0, sizeof (read_ptr));

        if (CfElements.array_map_elements > 0) {
            len += parseData(read_ptr,sizeof(read_ptr));
            choice = NULL;
            
            for (uint8_t i = 0; i < ARRAY_SIZE(act_eeprom_items_list); i++) {
                if (strncmp(read_ptr, act_eeprom_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0) {
                    choice = act_eeprom_items_list + i;
//                    found = true; //jira: CAE_MCU8-5647
                    break;
                }
            }                     
            mid = choice ? choice->id : UNKNOWN;
            switch (mid) {
                case UNKNOWN: //Only RAM Data
                    act_choice = act_ram_items_list;
                    act_choice = NULL;
                    for (uint8_t i = 0; i < ARRAY_SIZE(act_ram_items_list); i++) {
                        if (strncmp(read_ptr, act_ram_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0) {
                            act_choice = act_ram_items_list + i;
                            break;
                        }
                    }
                    mid = act_choice ? act_choice->id : UNKNOWN;
                    memset(read_ptr, 0, sizeof (read_ptr));

                    
                    for(uint16_t i=0; i<MAX_ACTUATORS_COUNT;i++ )
                    {
                        if(ar[i].resIndex == objIdx)
                        {  
                            temp_len = -1;
                            temp_len = actuator_RamdataPutHandle(mid, read_ptr, i);
                            if(temp_len != -1)
                            {
                                len2+= (uint8_t)temp_len; //jira: CAE_MCU8-5647
                                ret = SUCCESS;
                                break;
                            }
                            else
                                return ERROR;
                           
                        }
                    }

                    break;
                default:
                    if (iseepromread == false) {
                        obj_getObjectAtIndex(objIdx, (const void *) &p);
                        iseepromread = true;
                    }
                    temp_len = -1;
                    temp_len = readFromEthActuatorData(objIdx, mid, &p);
                    if(temp_len != -1)
                    {
                        len1 += (uint8_t)temp_len; //jira: CAE_MCU8-5647
                        ret = SUCCESS;
                    }
                    else
                        return ERROR;
                    break;

            }
            if (len1 == 0 && len2 == 0) //default case
            {
                len += parseData(read_ptr,sizeof(read_ptr)); // - for UUID compare it with uuid in the eeprom
                CfElements.array_map_elements--;
            } else {
                len += len1;
                len += len2;
            }

            payload_length -= len;            
//            found = false; //jira: CAE_MCU8-5647
        } 
        else {            
            payload_length = 0;
        }

    }
    if (iseepromread == true) {
        obj_setObjectAtIndex(objIdx, (const void *) &p);
        iseepromread = false;
    }    

    return ret;
}

error_msg actuatorDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{    
    actuator_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.lightColor;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.act_eep_1.name;
    ret = string_removeWord(stringIdxToBeRemoved);

    return ret;
}

error_msg actuatorPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{   
    error_msg ret = ERROR;

    return ret;
}

error_msg actuatorPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{   
    error_msg ret = ERROR;

    return ret;
}





restfulMethodStructure_t actuatorMethods = {actuatorInit, actuatorGetter, actuatorPutter, actuatorPatcher, actuatorDeleter, actuatorPoster,initActuatorResource};

/**************************************************************RAM Data Handlers***********************************************************************/

uint8_t actuator_RamdataGetHandle(menu_items_t mid, char *read_ptr, int16_t actIdx) {

    uint8_t len = 0;
    char buf[8]; //jira: CAE_MCU8-5647

    Get_ToEthernet((char *)"pp",TEXT_STRING); 
    itoa(buf,ar[(uint8_t)actIdx].percentPower,10); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER8); //jira: CAE_MCU8-5647


    Get_ToEthernet((char *)"pi",TEXT_STRING); 
    itoa(buf,ar[(uint8_t)actIdx].percentIntensity,10); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER8);//jira: CAE_MCU8-5647

    Get_ToEthernet((char *)"or",TEXT_STRING); //jira: CAE_MCU8-5647
    Get_ToEthernet(ar[(uint8_t)actIdx].overrideFlag,TEXT_STRING); //jira: CAE_MCU8-5647

    Get_ToEthernet((char *)"at",TEXT_STRING);  //jira: CAE_MCU8-5647
    itoa(buf,ar[(uint8_t)actIdx].adjustTime, 10); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER16); //jira: CAE_MCU8-5647

    Get_ToEthernet((char *)"ar",TEXT_STRING);   //jira: CAE_MCU8-5647
    itoa(buf,ar[(uint8_t)actIdx].adjustRate,10); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER16);  //jira: CAE_MCU8-5647
    
    Get_ToEthernet((char *)"oK",TEXT_STRING);   //jira: CAE_MCU8-5647
    itoa(buf,ar[(uint8_t)actIdx].degKelvin,10); //jira: CAE_MCU8-5647
    Get_ToEthernet((char *)buf,UNSIGNED_INTEGER16);  //jira: CAE_MCU8-5647

  
    return len;
}

int8_t actuator_RamdataPutHandle(menu_items_t mid, char *read_ptr, int16_t actIdx) {
    int8_t len = 0;
    switch (mid) 
    {
        case PI:
            len = parseData(read_ptr,sizeof(read_ptr));
            Put_ToResource(read_ptr,&ar[(uint8_t)actIdx].percentIntensity,UNSIGNED_INTEGER8); //jira: CAE_MCU8-5647

#ifdef IF_LIGHT
            if (((strncmp(ar[(uint8_t)actIdx].overrideFlag,"N",1)) == 0) || ((strncmp(ar[(uint8_t)actIdx].overrideFlag,"n",1)) == 0))    //jira: CAE_MCU8-5647   
            {
                target_power_level_pwm[(uint8_t)actIdx] = ar[(uint8_t)actIdx].percentIntensity;         //jira: CAE_MCU8-5647
                target_power_level_pwm[(uint8_t)actIdx] = Curve[target_power_level_pwm[(uint8_t)actIdx]] >> 2;
                fadeCount = (ar[(uint8_t)actIdx].adjustTime)/10;
                if(target_power_level_pwm[(uint8_t)actIdx] > current_power_level_pwm[(uint8_t)actIdx])
                {
                    stateFlags.fadeUp = 1;
                }
                else
                {
                    stateFlags.fadeDown = 1;
                }
                if(fadeCount != 0)
                {
                    pwm1_fadeDelta();
                }
            }
#endif
#ifdef IF_GEST
            gest_power_level_act[actIdx] = ar[actIdx].percentIntensity;
#endif
            CfElements.array_map_elements--;
            break;
        case OR:
            len = parseData(read_ptr,sizeof(read_ptr));
            Put_ToResource(read_ptr,ar[(uint8_t)actIdx].overrideFlag,TEXT_STRING);  //jira: CAE_MCU8-5647
           if (((strncmp(ar[(uint8_t)actIdx].overrideFlag,"N",1)) == 0) || ((strncmp(ar[(uint8_t)actIdx].overrideFlag,"n",1)) == 0)){      //jira: CAE_MCU8-5647
#ifdef IF_LIGHT               
                target_power_level_pwm[(uint8_t)actIdx] = ar[(uint8_t)actIdx].percentIntensity;  //jira: CAE_MCU8-5647
                target_power_level_pwm[(uint8_t)actIdx] = Curve[target_power_level_pwm[(uint8_t)actIdx]]; //jira: CAE_MCU8-5647
#endif
#ifdef IF_GEST
                gest_power_level_act[actIdx] = ar[actIdx].percentIntensity;
#endif
            } else {
#ifdef IF_LIGHT               
                target_power_level_pwm[(uint8_t)actIdx] = Curve[100];  //jira: CAE_MCU8-5647
                EPWM1_LoadDutyValue(target_power_level_pwm[(uint8_t)actIdx]); //jira: CAE_MCU8-5647
                current_power_level_pwm[(uint8_t)actIdx]=Curve[100];  //jira: CAE_MCU8-5647
#endif
#ifdef IF_GEST
                gest_power_level_act[actIdx] = Curve[100];
#endif
            }
            CfElements.array_map_elements--;
            break;
        case PP:
            len = parseData(read_ptr,sizeof(read_ptr));
            Put_ToResource(read_ptr,&ar[(uint8_t)actIdx].percentPower,UNSIGNED_INTEGER8);  //jira: CAE_MCU8-5647
            CfElements.array_map_elements--;
            break;       
        case AT:
            len = parseData(read_ptr,sizeof(read_ptr));
            ar[(uint16_t)actIdx].adjustTime = 0;
            Put_ToResource(read_ptr,&ar[(uint8_t)actIdx].adjustTime,UNSIGNED_INTEGER16);   //jira: CAE_MCU8-5647
            CfElements.array_map_elements--;
            break;
        case AR:
            len = parseData(read_ptr,sizeof(read_ptr));
            ar[(uint16_t)actIdx].adjustTime = 0;
            Put_ToResource(read_ptr,&ar[(uint8_t)actIdx].adjustRate,UNSIGNED_INTEGER16);  //jira: CAE_MCU8-5647
            CfElements.array_map_elements--;
            break;            
        default:           
            CfElements.array_map_elements--;
            len = -1;
            break;
    }
    return len;
}

int8_t readFromEthActuatorData(obj_index_t objIdx, menu_items_t mid, actuator_eeprom_t *p)
{
    
    char read_ptr[MAX_WORD_SIZE];
    int8_t len =0;
    
    
    memset(read_ptr,0,sizeof(read_ptr));
    switch(mid)
    {
        case COLOR:
            len += parseData(read_ptr,sizeof(read_ptr));
            p->lightColor = string_insertWord(read_ptr,p->lightColor);
            CfElements.array_map_elements--;
            break;
        default:
            len += readFromEthActuatorSensoData(objIdx, mid, &p->act_eep_1);            
            break;  
    }
    return len;
}


void updatePowerPercent(const char *name, uint8_t percentPower) 
{
    int16_t actVertexIdx; 
    vertex_t act;
    
    actVertexIdx = graph_get_vertex_idx_byName(name);
    graph_getVertexAtIndex(&act,actVertexIdx);

    for(uint8_t i=0;i<MAX_ACTUATORS_COUNT;i++)
    {
        if(ar[i].resIndex == act.objIdx)
        {
            ar[i].percentPower = percentPower;          
            break;
        }
    }  
}


/**************************************************************Query Handlers**************************************************************************/
bool queryPpHandler(obj_index_t objIdx, uint32_t query_value) {
    bool ret = false;
    
    if (pp_query == 0) {
      for (uint8_t i = 0; i < MAX_ACTUATORS_COUNT; i++) {
            if (ar[i].resIndex==objIdx) {
                if (ar[i].percentPower == query_value) {
                    pp_query = 1;
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    } else
        ret = true;


    return ret;
}

bool queryPiHandler(obj_index_t objIdx, uint32_t query_value) {
    
    bool ret = false;

    if (pi_query == 0) {
     for (uint8_t i = 0; i < MAX_ACTUATORS_COUNT; i++) {
            if (ar[i].resIndex==objIdx) {
                if (ar[i].percentIntensity == query_value) {
                    pi_query = 1;
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    } else
        ret = true;
   
    return ret;
}

bool queryOrHandler(obj_index_t objIdx, uint32_t query_value) {
    bool ret = false;
  
    if (or_query == 0) {
        for (uint8_t i = 0; i < MAX_ACTUATORS_COUNT; i++) {
            if (ar[i].resIndex==objIdx) {
                if ((uint16_t)ar[i].adjustRate == query_value) {  //jira: CAE_MCU8-5647
                    or_query = 1;
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    } else
        ret = true;

    return ret;
}


bool queryAtHandler(obj_index_t objIdx, uint32_t query_value) {
    bool ret = false;
   
    if (at_query == 0) {
        for (uint8_t i = 0; i < MAX_ACTUATORS_COUNT; i++) {
            if (ar[i].resIndex==objIdx) {
                if ((uint16_t)ar[i].adjustTime == query_value) {  //jira: CAE_MCU8-5647
                    at_query = 1;
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    } else
        ret = true;

    return ret;
}

bool queryArHandler(obj_index_t objIdx, uint32_t query_value) {
    bool ret = false;
  
    if (ar_query == 0) {
        for (uint8_t i = 0; i < MAX_ACTUATORS_COUNT; i++) {
            if (ar[i].resIndex==objIdx) {
                if ((uint16_t)ar[i].adjustRate == query_value) {  //jira: CAE_MCU8-5647
                    ar_query = 1;
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    } else
        ret = true;

    return ret;
}

bool queryOkHandler(obj_index_t objIdx, uint32_t query_value) {
    bool ret = false;
  
    if (ok_query == 0) {
        for (uint8_t i = 0; i < MAX_ACTUATORS_COUNT; i++) {
            if (ar[i].resIndex==objIdx) {
                if ((uint16_t)ar[i].adjustRate == query_value) {  //jira: CAE_MCU8-5647
                    ok_query = 1;
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    } else
        ret = true;

    return ret;
}




