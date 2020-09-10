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

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hardware.h"
#include "gestic_i2c.h"
#include "uart.h"
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
#include "pwm.h"
#include "coap_contentFormat.h"
#include "data_model.h"


#ifndef _XTAL_FREQ
#define _XTAL_FREQ 25000000UL
#warning _XTAL_FREQ NOT DEFINEND IN THIS FILE.. Include a global file that has this in it
#endif

uint16_t gest_power_level_act[2];
uint8_t lastOptNum;
coapClient_t ptr = {0};

typedef struct
{
    uint8_t lightLevel;
    char key[37];
} lightModel_t;
lightModel_t lightModel[2];
static uint8_t activeLightIndex = 0;
OTHERLEDS LEDOther;
uint8_t  LEDNow=0;


void GestICSwitchTableCoAPOptions()
{
    COAP_AddOptions((coap_option_numbers_t)URI_PATH,ROOT_DIRECTORY,STRING);
    COAP_AddOptions((coap_option_numbers_t)URI_PATH,"actuators",STRING);
    COAP_AddOptions((coap_option_numbers_t)CONTENT_FORMAT,"60",INTEGER);    
    COAP_AddOptions((coap_option_numbers_t)URI_QUERY,(char *)&lightModel[activeLightIndex].key, STRING);
    COAP_AddOptions((coap_option_numbers_t)ACCEPT,"60",INTEGER); 
}

void gestPayload()
{
    COAP_CL_nameSpace(1);
    CBOR_Encode_Text_String("pp");
    CBOR_Encode_Unsigned_Int(lightModel[activeLightIndex].lightLevel);
}


payload_t gestPayloadTable[] =
{
    {gestPayload}  
};


OptionTypeHandler_t GestTableCoAPOptions[] = 
{
    {GestICSwitchTableCoAPOptions}
};

void get_gest_keys(void);
uint8_t gestIC_read(void);


#define GESTIC_I2C_ADDR     0x42    // default i2c address of MGC3130


#define MAX_LEN_TO_GESTIC   16      /* maximum length of message to be transferred */
#define MD_BUF_LEN          14

struct msgHeaderT {
        uint8_t size;
        uint8_t flags;
        uint8_t seq;
        uint8_t id;
};


static union {
    uint8_t buf[MD_BUF_LEN];
    struct msgHeaderT header;
    struct {                            // follow the Sensor Data Output
        struct msgHeaderT header;       //       Size, Flags, Seq, ID
        uint16_t streamingOutputMask;   //       StreamingOutputMask
        uint8_t timeStamp;              //       ....
        uint8_t systemInfo;
//        uint16_t dspInfo;
        uint32_t gestureInfo;
//        uint32_t touchInfo;
        uint8_t  airWheelCounter;
        uint8_t  airWheelUnused;
//        uint16_t posX;
//        uint16_t posY;
//        uint16_t posZ;                   // 26 Bytes
    } sensorData;
} md;

//static union {
//    uint8_t buf[MD_BUF_LEN];
//    struct msgHeaderT header;
//    struct {                            // follow the Sensor Data Output
//        struct msgHeaderT header;       //       Size, Flags, Seq, ID
//        uint16_t streamingOutputMask;   //       StreamingOutputMask
//        uint8_t timeStamp;              //       ....
//        uint8_t systemInfo;
////        uint16_t dspInfo;
//        uint32_t gestureInfo;        
//        uint8_t  airWheelCounter;
//        uint8_t  airWheelUnused;                         // 26 Bytes
//    } sensorData;
//} md;

static uint8_t AirWheelActivePrevious = 0;   // AirWheel status in the previous run
static int16_t AirWheelValuePrevious=0;                 // AirWheel counter value each time that the flag in the SystemInfo goes from 0 to 1
static int16_t AirWheelDiff = 0; //a value of +32:a full clockwise circle; a value of -32: a counterclockwise cricle

#define ResetDelay(a) do { uint16_t z=a; do{DELAY_MS(1);z--;}while(z); } while(0)

void gestIC_init(void)
{    
  #ifdef __XC8
    ResetDelay(100);
    GEST_RESET_LAT = 0; // Reset
    GEST_RESET_TRIS = 0;
    GEST_EIO_TRIS = 1;
    GEST_EIO_LAT = 0;
    ResetDelay(140);
    GEST_RESET_LAT = 1;
    CMCON = 0b00000111;  
    gest_power_level_act[0]=0;
    gest_power_level_act[1]=0;
  #endif
}

bool gestIC_readMode(void)
{
  #ifdef __XC8
    if(GEST_EIO_PORT == 1)
      return false;
    else
    {
      GEST_EIO_TRIS = 0;
      GEST_EIO_LAT = 0;
      return true;
    }
  #else
    return false;
  #endif
}

void gestIC_readComplete(void)
{
  #ifdef __XC8
    GEST_EIO_TRIS = 1;
  #endif
}

uint8_t gestIC_read(void)
{
      // bits 0 to 4 must be set in the StreamOutputMask to have valid data to read
//    uint16_t streamOutRequired = (STREAMOUT_GESTUREINFO | STREAMOUT_AIRWHEELINFO);
    uint16_t streamOutRequired = (STREAMOUT_GESTUREINFO | STREAMOUT_AIRWHEELINFO );

    uint8_t retVal = GI_NOGESTURE;    

    // Check I/O pin for new Gest data.
 

    if(gestIC_readMode() == false) return 0;

    md.header.flags=0;
    md.header.id=0;
    md.header.seq=0;
    md.header.size=0;
   
    i2c1_blockingMasterRead(GESTIC_I2C_ADDR,&md.buf,14);

    gestIC_readComplete();

   // while(PORTAbits.RA5==0); // Wait for release of line
    DELAY_US(100);

    // Process data and return the current gesture
    
 
//    if(md_idx < 4)
//        return GI_NOGESTURE;

    if(md.sensorData.header.id != MSGID_SENSORDATAOUT)
        return GI_NOGESTURE;

    if(md.sensorData.header.size < 10 )  // message too short for our sensor data configuration
        return GI_NOGESTURE;              //DSP(2)+Gesture(4)+Touch(4)+AirWheel(2)+XYZPosition(6)+NoisePower(4)= 22bytes

    // ok, we got a sensor data message. Check that the 5 first bits of the StreamOutputMask are set to indicate valid data are present
    if((md.sensorData.streamingOutputMask & streamOutRequired) == streamOutRequired) {

//        uint32_t TouchInfo = md.sensorData.touchInfo;
        uint8_t AirWheelActive = (md.sensorData.systemInfo & SI_AIRWHEELVALID) != 0; // AirWheel is active and valid if bit1 of SystemInfo is set
        int16_t AirWheelValueNew = md.sensorData.airWheelCounter;

        /* FLICK DETECTION */
	retVal = md.sensorData.gestureInfo & 0xFF; // keep only the first byte (b0..b7) that gives the gesture information (in DECIMAL)    

        /* TAP DETECTION */
//        if(TouchInfo & 1<<BITSHIFT_TAP_SOUTH ) {
//            retVal = GI_TAP_SOUTH;
//        }
//        else if(TouchInfo & 1<<BITSHIFT_TAP_WEST) {
//            retVal = GI_TAP_WEST;
//        }
//        else if(TouchInfo & 1<<BITSHIFT_TAP_NORTH) {
//            retVal = GI_TAP_NORTH;
//        }
//        else if(TouchInfo & 1<<BITSHIFT_TAP_EAST) {
//            retVal = GI_TAP_EAST;
//        }
//        else if(TouchInfo & 1<<BITSHIFT_TAP_CENTER) {
//            retVal = GI_TAP_CENTER;
//        }
//        else {}

        /* AIRWHEEL DETECTION */
        //store the airwheel counter when the airwheel is started
        if (AirWheelActive && !AirWheelActivePrevious)
        {
            AirWheelValuePrevious = md.sensorData.airWheelCounter;
        }
        else if (AirWheelActive)
        {
            if( AirWheelValuePrevious < 64 && AirWheelValueNew > 192 ){
                    // "crossing zero point" happened
                    AirWheelDiff += ( (AirWheelValueNew-256) - AirWheelValuePrevious);
            }
            else if( AirWheelValuePrevious > 192 && AirWheelValueNew < 64 ){
                    // "crossing zero point" happened
                    AirWheelDiff += ( (AirWheelValueNew+256) - AirWheelValuePrevious);
            }
            else{
                    AirWheelDiff += AirWheelValueNew - AirWheelValuePrevious ;
            }

            if (AirWheelDiff >= 32)
            {
                AirWheelDiff=0;
                retVal = GI_AIRWHEEL_CW;
            }
            else if (AirWheelDiff <= -32)
            {
                AirWheelDiff=0;
                retVal = GI_AIRWHEEL_CCW;
            }
//            else{}
        }
        AirWheelActivePrevious = AirWheelActive;    // save the status for the next run
        AirWheelValuePrevious = AirWheelValueNew;        
//        putch(retVal);
        if(retVal>1)
        {
//            LATEbits.LATE5^=1;
        }    

        
    }
    
    return (retVal);

}

void gest_process(void)
{
    uint8_t gest;
    
    static uint8_t sendSomething = 0;
    static uint8_t DimTimer;
   if(ipdb_getAddress()==0)
       return;
    
    lightModel[0].lightLevel=gest_power_level_act[0];
    lightModel[1].lightLevel=gest_power_level_act[1];
    
    gest= gestIC_read(); 
    if(DimTimer > 0)
    {
        DimTimer--;
        if(DimTimer==0)
        {
            LEDOther.DimDown=0;
            LEDOther.DimUp=0;
            if(gest==0)gest=0xFF; // Force an update if not going through already
        }

    }
    if(gest>1)
    {
      switch(gest)
      {
          case GI_AIRWHEEL_CCW:
              if(lightModel[activeLightIndex].lightLevel > 0)
              {
                  lightModel[activeLightIndex].lightLevel -= 10;           
              }
              DimTimer=12;
              LEDOther.DimDown=1;
              sendSomething=1;
              break;
          case GI_AIRWHEEL_CW:
              if(lightModel[activeLightIndex].lightLevel < 100)
              {
                  lightModel[activeLightIndex].lightLevel += 10;                    
              }
              DimTimer=12;
              LEDOther.DimUp=1;
              sendSomething=1;
              break;

          case GI_FLICK_NS:
              lightModel[activeLightIndex].lightLevel = 0;
              LEDOther.DimDown=1;
              DimTimer=12;
              sendSomething=1;
              break;

          case GI_FLICK_SN:
              lightModel[activeLightIndex].lightLevel = 100;
              LEDOther.DimUp=1;
              DimTimer=12;
              sendSomething=1;
              break;
          case GI_FLICK_EW:
              sendSomething = 0;
              activeLightIndex = 0;
              LEDOther.DimUp=1;
              DimTimer=12;

              break;

          case GI_FLICK_WE:
              sendSomething = 0;
              activeLightIndex = 1;
              LEDOther.DimDown=1;
              DimTimer=12;
              break;
          default:
              break;
      }
    if(sendSomething)
    {
        LEDOther.Zone1 = 0;
        LEDOther.Zone2 = 0;

        if(activeLightIndex == 0) LEDOther.Zone1 = 1;
        else LEDOther.Zone2 = 1;
        LEDNow = lightModel[activeLightIndex].lightLevel/9;
        led_write(LEDNow,(uint8_t)LEDOther.dat);
//        memset(&ptr,0,sizeof(ptr)); //sizeof not working properly
        ptr.clientOption = NULL;
        ptr.clientPayload = NULL;
        ptr.destIP = 0;
        memset(ptr.clientToken,0,sizeof(ptr.clientToken));
        
        get_gest_keys();
        sendSomething=0;
        if(activeLightIndex)
        {     
            gest_power_level_act[1]=lightModel[activeLightIndex].lightLevel;
            ptr.destIP = ipdb_classCbroadcastAddress();
            ptr.clientOption = GestTableCoAPOptions;
            ptr.clientPayload = gestPayloadTable;
            COAP_Client(&ptr,NON,PUT,false); 
            COAP_Send();
            updatePowerPercent("actuator2",lightModel[activeLightIndex].lightLevel);
            
//            DELAY_MS(15);

        }
        else
        {            
            gest_power_level_act[0]=lightModel[activeLightIndex].lightLevel;
            ptr.destIP = ipdb_classCbroadcastAddress();
            ptr.clientOption = GestTableCoAPOptions;
            ptr.clientPayload = gestPayloadTable;
            COAP_Client(&ptr,NON,PUT,false);  
            COAP_Send();
            updatePowerPercent("actuator1",lightModel[activeLightIndex].lightLevel);
//            DELAY_MS(15);
        }
    }
    }
       LEDOther.Zone1 = 0;
       LEDOther.Zone2 = 0;

       if(activeLightIndex == 0) LEDOther.Zone1 = 1;
       else LEDOther.Zone2 = 1;
       LEDNow = lightModel[activeLightIndex].lightLevel/9;
       led_write(LEDNow,(uint8_t)LEDOther.dat);
}


void get_gest_keys(void)
{
   strcpy(lightModel[0].key,getContextKeywords("CON1",true,0)->string_word);
   strcpy(lightModel[1].key,getContextKeywords("CON2",true,0)->string_word);
}
