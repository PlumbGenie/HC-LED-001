/**
 POT switch driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    pot_switch.c

  Summary:
    POT switch driver  implementation.

  Description:
    This file provides the POT switch driver  implementation.

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
 * File:   pot_switch.c
 * Author: C16813
 *
 * Created on February 12, 2016, 10:48 AM
 */


#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "hardware.h"
#include "pot_switch.h"
#include "tcpip_config.h"
#include "coap.h"
#include "cbor.h"
#include "udpv4.h"
#include "leds_gestic.h"
#include "adc.h"
#include "tmr3.h"
#include "context.h"
#include "coap_options.h"
#include "pot_switch.h"
#include "actuator.h"
#include "data_model.h"


/* READ IN MAIN
 * pot_init_adcResult  = ADC_GetConversion(channel_AN3)/10; *
 * in main.c to get the ADC result                          *         
 * ADCON1 should be set to ADCON1 = 0x0B for CRD board      *
 */

volatile uint8_t pot_dly;
volatile uint16_t pot_initial_adcResult;
adc_result_t pot_adcResult;

void PotInfoTableCoAPOptions()
{
    COAP_AddURIPathOption("mchp");
    COAP_AddURIPathOption("actuators");
    COAP_AddContentFormatOption(CBOR);   
}

void potPayload()
{
    COAP_CL_nameSpace(1);
    CBOR_Encode_Text_String((char *)"ar");
    CBOR_Encode_Unsigned_Int(pot_adcResult);
}


const payload_t potPayloadTable[] =
{
    {potPayload}  
};


const OptionTypeHandler_t PotTableCoAPOptions[] = 
{
    {PotInfoTableCoAPOptions}
};

void pot_sendPacket(uint16_t adcResult);


void pot_send(void)
{   
        pot_adcResult = ADC_GetConversion(channel_AN4)/10; //Read Potentiometer value from ADC (channel 4)

         if(((pot_adcResult > (pot_initial_adcResult + 5)) || (pot_adcResult <(pot_initial_adcResult - 5)))) 
         {
            if(pot_adcResult > 5)
            {
                if(pot_adcResult > 102)pot_adcResult=102;
                pot_initial_adcResult = pot_adcResult; 


                pot_sendPacket(pot_adcResult);

            }
            if(pot_adcResult <= 5)
            {
                pot_adcResult=0;
                if(pot_adcResult != pot_initial_adcResult)
                {
                    pot_initial_adcResult=pot_adcResult;
                    pot_sendPacket(pot_adcResult);
                }
            }
         }
  
}

void pot_control(void)
{    

    if(pot_10ms)
    {
        pot_10ms=0;
        pot_dly++;
        if(pot_dly>=10)
        {
            pot_dly=0;
            pot_send();
        }
    }

}
   

void pot_sendPacket(uint16_t adcResult)
{
    static coapTransmit_t ptr= {0};    

    ptr.destIP = 0xFFFFFFFF;
    ptr.clientOption = (OptionTypeHandler_t *) PotTableCoAPOptions;   //jira: CAE_MCU8-5647
    ptr.clientPayload = (payload_t *)potPayloadTable;                 //jira: CAE_MCU8-5647
    ptr.srcPort = COAP_CLIENT_SRC_PORT;
    ptr.destPort = COAP_CLIENT_DEST_PORT; 
    COAP_Transmit(&ptr,NON_TYPE,PUT,false);    
    COAP_Send();
    __delay_ms(15);
    
}

void pot_init(void)
{
    pot_dly = 0;
    pot_initial_adcResult = ADC_GetConversion(channel_AN4)/10;
}
