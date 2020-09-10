/**
 Toggle switch driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    toggle_switch.c

  Summary:
    Toggle switch driver implementation.

  Description:
    This file provides the Toggle switch driver implementation.

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

/*
 * File:   toggle_switch.c
 * Author: C16813
 *
 * Created on February 12, 2016, 10:03 AM
 */


#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "coap.h"
#include "coap_resources.h"
#include "udpv4.h"
#include "tcpip_config.h"
#include "leds_gestic.h"
#include "toggle_switch.h"
#include "hardware.h"
#include "cbor.h"
#include "data_model.h"


static int8_t debounce = 0;
static bool switch_pos = false;
static bool switch_pv = false;
static uint8_t sw_val;
static uint8_t tog_val;
//uint8_t opValue = 0;   //jira: CAE_MCU8-5647
   
#ifndef IF_TOGGLE
 #define TOGGLE_STATE  0
#endif

void ToggleInfoTableCoAPOptions()
{
    COAP_AddURIPathOption("mchp");
    COAP_AddURIPathOption("actuator1");
    COAP_AddContentFormatOption(CBOR);        
}


void togglePayload()
{
    COAP_CL_nameSpace(1);
    CBOR_Encode_Text_String((char *)"op");
    CBOR_Encode_Unsigned_Int(tog_val);
}


const payload_t togglePayloadTable[] =
{
    {togglePayload}  
};


const OptionTypeHandler_t ToggleTableCoAPOptions[] = 
{
    {ToggleInfoTableCoAPOptions}             //jira: CAE_MCU8-5647
};


void toggle_switch(void)
{
    coapTransmit_t ptr;    

    tog_val = TOGGLE_STATE;

    if(Debounce() && sw_val!= tog_val)
    {
        switch_pv = switch_pos;
        sw_val = tog_val;
        
        ptr.destIP = MAKE_IPV4_ADDRESS(255,255,255,255);
        ptr.clientOption = (OptionTypeHandler_t *)ToggleTableCoAPOptions;   //jira: CAE_MCU8-5647
        ptr.clientPayload = (payload_t *)togglePayloadTable;                //jira: CAE_MCU8-5647
        ptr.srcPort = COAP_CLIENT_SRC_PORT;
        ptr.destPort = COAP_CLIENT_DEST_PORT;
        COAP_Transmit(&ptr,NON_TYPE,PUT,false);  
        COAP_Send();
        __delay_ms(15);
    }
}

int Debounce()
{
    
    if(TOGGLE_STATE) debounce ++;
    else debounce --;

    if(debounce > 50)
    {
        debounce = 50;
        switch_pos = true;
    }
    else if(debounce < 0)
    {
        debounce = 0;
        switch_pos = false;
    }
    
    if(switch_pv != switch_pos)
    {
        return 1;
    }
    return 0;
}


