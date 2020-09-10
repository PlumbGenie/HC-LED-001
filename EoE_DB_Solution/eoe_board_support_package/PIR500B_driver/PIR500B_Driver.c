/**
 PIR500B motion sensor driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    tmr3.c

  Summary:
    PIR500B motion sensor driver implementation.

  Description:
    This file provides the PIR500B motion sensor driver implementation.

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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "PIR500B_Driver.h"
#include "hardware.h"
#include "log.h"
#include "actuator.h"
#include "coap.h"
#include "tcpip_config.h"
#include "coap_options.h"
#include "cbor.h"
#include "coap_put.h"

//void PIR500BTableCoAPOptions()
//{
//    COAP_AddOptions((coap_option_numbers_t)URI_PATH,"connectedlighting",STRING);
//    COAP_AddOptions((coap_option_numbers_t)URI_PATH,"actuators",STRING);
//    COAP_AddOptions((coap_option_numbers_t)CONTENT_FORMAT,"60",INTEGER) ;   
//}

bool motionFlag = true;

void PIR500BOnPayload()
{
    COAP_CL_nameSpace(1);
    CBOR_Encode_Text_String("pp");
    CBOR_Encode_Unsigned_Int(100);
}

void PIR500BOffPayload()
{
    COAP_CL_nameSpace(1);
    CBOR_Encode_Text_String("pp");
    CBOR_Encode_Unsigned_Int(0);
}

void PIR500BMotionDeviceInit(void)
{
    // Motion Sensor
    
    MOTION_PIR500B_INTERRUPT_TRIS = 1;             // Configure the interrupt pin as input
    MOTION_PIR500B_ENABLE_TRIS    = 1;             // Configure the enable pin as input
    MOTION_PIR500B_ENABLE_PORT = 1;                // Enable the sensor
    
}

void detectPIR500BMotion (void)
{
    coapClient_t ptr; 
    
//    time_t now_time, time_to_put;
//    time(&now_time);  
    if(MOTION_PIR500B_INTERRUPT_PORT  ==1 )
    {   
        //        motionFlag = true;
        printf("Motion Detected \r\n"); 
        memset(&ptr, 0, sizeof(ptr));
        //        logMessage("Motion Detected", LOG_KERN, LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET))  ;
        ptr.destIP = MAKE_IPV4_ADDRESS(255,255,255,255);
        //        ptr.clientOption = PIR500BTableCoAPOptions;
        COAP_Client(&ptr,NON,PUT,false); 
        COAP_AddOptions((coap_option_numbers_t)URI_PATH,"connectedlighting",STRING);
        COAP_AddOptions((coap_option_numbers_t)URI_PATH,"actuators",STRING);
        COAP_AddOptions((coap_option_numbers_t)CONTENT_FORMAT,"60",INTEGER) ; 
        //    COAP_AddOptions((coap_option_numbers_t)URI_QUERY,(char *)&lightModel[activeLightIndex].key, STRING);
        COAP_EndofOptions();
        PIR500BOnPayload();     
        COAP_Send();   
        updatePowerPercent("ACT1",100);        
        __delay_ms(15);
           
    }
    
    else if(MOTION_PIR500B_INTERRUPT_PORT  ==0)
    {
        
            motionFlag == false;
            printf("No motion detected \r\n");
            memset(&ptr, 0, sizeof(ptr));
            //        logMessage("No motion detected", LOG_KERN, LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET))  ;
            ptr.destIP = MAKE_IPV4_ADDRESS(255,255,255,255);
//            //        ptr.clientOption = PIR500BTableCoAPOptions;
            COAP_Client(&ptr,NON,PUT,false); 
            COAP_AddOptions((coap_option_numbers_t)URI_PATH,"connectedlighting",STRING);
            COAP_AddOptions((coap_option_numbers_t)URI_PATH,"actuators",STRING);
            COAP_AddOptions((coap_option_numbers_t)CONTENT_FORMAT,"60",INTEGER) ; 
//            //        //    COAP_AddOptions((coap_option_numbers_t)URI_QUERY,(char *)&lightModel[activeLightIndex].key, STRING);
            COAP_EndofOptions();
            PIR500BOffPayload();
            COAP_Send();
            updatePowerPercent("ACT1",0);
            
            __delay_ms(15);
        
    }
    else
    {
        printf("Interrupt not generated \r\n");
    }
    
}