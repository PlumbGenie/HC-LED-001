 /**
 GestIC driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    gestic.c

  Summary:
    GestIC Driver implementation.

  Description:
    This file provides the GestIC driver implementation.

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

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware.h"
#include "gestic.h"
#include "eusart1.h"
#include "i2c_async.h"
#include "tcpip_config.h"
#include "coap.h"
#include "leds_gestic.h"
#include "coap_resources.h"
#include "ip_database.h"
#include "udpv4.h"
#include "cbor.h"
#include "actuator.h"
#include "context.h"
#include "data_model.h"
#include "spi1.h"


#ifndef _XTAL_FREQ
#define _XTAL_FREQ 25000000UL
#warning _XTAL_FREQ NOT DEFINEND IN THIS FILE.. Include a global file that has this in it
#endif

uint16_t gest_power_level_act[MAX_ZONES];     
//uint8_t lastOptNum;                           //jira: CAE_MCU8-5647
static coapTransmit_t ptr = {0};

typedef struct
{
    char key[37];
} lightModel_t;
lightModel_t lightModel[2];
static uint8_t activeLightIndex = 0;
//OTHERLEDS LEDOther;                          //jira: CAE_MCU8-5647
//uint8_t  LEDNow=0;                           //jira: CAE_MCU8-5647

#define GEST_READ()   SPI1_Exchange8bit(0)
#define GEST_WRITE(a) SPI1_Exchange8bit(a)

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(*(a)))

static void GestProcess(void);
static void GestIC_ReadZoneAll(void);
static GESTPACKET gestPacket;
static uint8_t allZoneLevels[MAX_ZONES];

typedef void (*gest_cmd_function_ptr)(void);

typedef struct
{
    uint8_t command;
    gest_cmd_function_ptr cmdCallBack;
}gestCmdHandler_t;

const gestCmdHandler_t GestCmd_CallBackTable[5] = 
{
    {ZONE_SINGLE          , NULL    },
    {ZONE_ALL             , GestIC_ReadZoneAll    }, 
    {ZONE_NAME            , NULL    },
    {IP_STRING            , NULL    },
    {CONSOLE_STRING       , NULL    }    
};


void GestICSwitchTableCoAPOptions()
{
    COAP_AddURIPathOption(ROOT_DIRECTORY);
    COAP_AddURIPathOption("actuators");
    COAP_AddContentFormatOption(CBOR);    
    COAP_AddURIQueryOption((char *)lightModel[activeLightIndex].key);     //jira: CAE_MCU8-5647
    COAP_AddAcceptOption(CBOR); 
}

void gestPayload()
{
    COAP_CL_nameSpace(1);
    CBOR_Encode_Text_String("pi");
    CBOR_Encode_Unsigned_Int(allZoneLevels[activeLightIndex]);
}


const payload_t gestPayloadTable[] =
{
    {gestPayload}  
};


const OptionTypeHandler_t GestTableCoAPOptions[] = 
{
    {GestICSwitchTableCoAPOptions}
};

void get_gest_keys(void);




void gestIC_init(void)
{   

}

static void GestProcess(void)
{
    uint8_t i = 0;
    const gestCmdHandler_t *hptr;     

    hptr = GestCmd_CallBackTable;    

    for(i=0; i < ARRAYSIZE(GestCmd_CallBackTable); i++)
{    
        if(hptr->command == gestPacket.packet[0])
{
            hptr->cmdCallBack();
    }
        hptr++;
}
}

void GestIC_Update(void)
{
    char str[16];
    uint32_t ip;
    uint8_t i = 0;


    ip = ipdb_getAddress();
 
    memset(str,0,sizeof(str));

    sprintf(str,"%3d.%3d.%3d.%3d",((char*)&ip)[3],((char*)&ip)[2],((char*)&ip)[1],((char*)&ip)[0]);

    GestIC_Write(IP_STRING,str,strlen(str));   //jira: CAE_MCU8-5647
}

 void Gest_PacketRecvd(uint8_t zone, uint8_t level)   
 {

    GESTPACKET sendPkt;
                
    allZoneLevels[zone] = level;
    for(uint8_t i =0; i < MAX_ZONES; i++)
    {
        sendPkt.AllLevels.levels[i] = allZoneLevels[i];
    }
     GestIC_Write(ZONE_ALL,&sendPkt,4);
 }
    
void GestIC_Read(void)
{
    uint8_t i =0;   
    uint8_t ret; 

    GEST_CS_SetLow();    
    for(i =0; i < sizeof(gestPacket); i++)
    {
        ret = GEST_READ();
       gestPacket.packet[i] = ret;
}
    GEST_CS_SetHigh();
    GestProcess();    
}


void GestIC_Write(uint8_t command, void *data, uint8_t len)
{
    uint8_t i =0;
    char *p = data;
    
    
    GEST_CS_SetLow();
    DELAY_US(1);//db- not req
    GEST_WRITE(command);   
    
    for(i =0; i < len; i++)
    {
        GEST_WRITE(*p++);
        }

    GEST_CS_SetHigh();
    }

static void GestIC_ReadZoneAll(void)
    {

    uint8_t i =0;
    char act[SIZE_OF_RESOURCEID];


    //Check with the previous data, update with the new data and send CoAP Packet
    for(i=0; i < MAX_ZONES; i++)
    {
        if(allZoneLevels[i] != gestPacket.packet[i+1u])       //jira: CAE_MCU8-5647
        {
            memset(act,0,sizeof(act));
            //update
            allZoneLevels[i] = gestPacket.packet[i+1u];      //jira: CAE_MCU8-5647
//            gest_power_level_act[i] = allZoneLevels[i];
            activeLightIndex = i;

            //send CoAP Packet
        get_gest_keys();
            
            ptr.destIP = ipdb_classCbroadcastAddress();
            ptr.clientOption = (OptionTypeHandler_t*)GestTableCoAPOptions;   //jira: CAE_MCU8-5647
            ptr.clientPayload = (payload_t *)gestPayloadTable;               //jira: CAE_MCU8-5647
            ptr.srcPort = COAP_CLIENT_SRC_PORT;
            ptr.destPort = COAP_CLIENT_DEST_PORT; 
            COAP_Transmit(&ptr,NON_TYPE,PUT,false);  
            COAP_Send();
            sprintf(act,"ACT%d",i+1);           
//            updatePowerPercent(act,allZoneLevels[i]);         
        }
    }

}





void get_gest_keys(void)
{
//   strcpy(lightModel[0].key,getContextKeywords("CON1",true,0)->kw_word);
//   strcpy(lightModel[1].key,getContextKeywords("CON2",true,0)->kw_word);
}
