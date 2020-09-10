/**
  Main file for implementing the Demo functionality
	
  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

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
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
//#include "mcc.h"
//#include "pin_manager.h"
#include "hardware.h"
#include "log.h"
#include "tcpip_config.h"
#include "network.h"
#include "ipv4.h"
#include "udpv4.h"

#include "tftp.h"
#include "ethernet_driver.h"
#include "adc.h"
#include "lldp_tlv_handler_table.h"
#include "coap.h"
#include "rtcc.h"

#include "udpv4.h"
#include "coap_options.h"
#include "graph.h"
#include "string_heap.h"
#include "eeprom.h"
#include "object_heap.h"
#include "resources.h"
#include "ip_database.h"
#include "tftp.h"
#include "tmr3.h"
#include "tmr1.h"
#include "epwm1.h"
#include "inventory.h"
#include "led_board.h"
#include "curr_volt_sense.h"
#include "device_registry.h"
#include "helper.h"
#include "actuator.h"
#include "boot_process.h"
#include "configuration.h"
#include "curve.h"


#define NeoPin PIN_D0
#define NeoNum 72
//#define RAND_MAX 64
#define ALL_OUT 0x00
#define ALL_IN  0xFF
//#byte PORTA = 0xF80
#define bit_test(ll, bb) ((ll) & (1 <<(bb)))



uint8_t NeoGreen [NeoNum];
uint8_t NeoBlue [NeoNum];
uint8_t NeoRed [NeoNum];
uint8_t NeoWhite [NeoNum];





#ifdef IF_TOGGLE
#include "toggle_switch.h"
#endif
#ifdef IF_POT
#include "pot_switch.h"
#endif
#ifdef IF_GEST
#include "gestic.h"
#endif


#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest) 
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif


//uint16_t allocatedPower=0, maxPower=500;//50.5 watts for a UPoE light   //jira: CAE_MCU8-5647
uint16_t requiredPower=240;        
uint8_t forceUpdate= false;

/**************************************************************************************************/

const int16_t dataBaseVersion =1; 
uint8_t pushNum =1;//db

void ledNRZport_Enable(void)
{
        IO_RD2_LAT =1;
}

void ledNRZport_Disable(void)
{
        IO_RD2_LAT =0;
}
//const char *dataBaseID = "DBID";   //jira: CAE_MCU8-5647

/* test bit n in word to see if it is on 
       assumes words are 32 bits long        */  

uint8_t  bit_test_was (uint8_t  wrd, uint8_t  n)  
{  
    if ( n < 0  || n > 7 )  
       return  0;  

    if ( (wrd >> (7 - n)) & 0x1 )  
       return 1;  
    else  
       return 0;  
}  

void pulse_00(void)
{
        asm("bsf LATD,0");
        asm("nop");
        asm("nop");
        asm("bcf LATD,0");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");    
}

void pulse_01(void)
{
        asm("bsf LATD,0");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("bcf LATD,0");
        asm("nop");
        asm("nop");
        //asm("nop");
        //asm("nop");    
}

void NeoBit (unsigned int Bitx)
{
   if (Bitx == 1){
   //{ output_high (NeoPin); delay_cycles (6); output_low (NeoPin); } // delay_cycles (3); // Bit '1' 
       pulse_01();
   }
   else
   {
   //{ output_high (NeoPin); delay_cycles (3); output_low (NeoPin); } // delay_cycles (6); // Bit '0'
       pulse_00();
   }
}

void NeoInit (void)
{
   uint8_t NeoPixel;
   for (NeoPixel = 0; NeoPixel < NeoNum; NeoPixel++)   
   {
      if (NeoPixel < 10)
         { NeoGreen[NeoPixel] = 0; NeoBlue[NeoPixel] = 0; NeoRed[NeoPixel] = 0xFF; NeoWhite[NeoPixel] = 0; }
      else if ((NeoPixel >= 10) & (NeoPixel < 20))
         { NeoGreen[NeoPixel] = 0; NeoBlue[NeoPixel] = 0xFF; NeoRed[NeoPixel] = 0; NeoWhite[NeoPixel] = 0; }
      else if ((NeoPixel >= 20) & (NeoPixel < 30))
         { NeoGreen[NeoPixel] = 0; NeoBlue[NeoPixel] = 0xFF; NeoRed[NeoPixel] = 0xFF; NeoWhite[NeoPixel] = 0; }
      else if ((NeoPixel >= 30) & (NeoPixel < 40))
         { NeoGreen[NeoPixel] = 0xFF; NeoBlue[NeoPixel] = 0; NeoRed[NeoPixel] = 0; NeoWhite[NeoPixel] = 0; }
      else if ((NeoPixel >= 40) & (NeoPixel < 50))
         { NeoGreen[NeoPixel] = 0xFF; NeoBlue[NeoPixel] = 0; NeoRed[NeoPixel] = 0xFF; NeoWhite[NeoPixel] = 0; }
      else if ((NeoPixel >= 50) & (NeoPixel < NeoNum))
         { NeoGreen[NeoPixel] = 0xFF; NeoBlue[NeoPixel] = 0xFF; NeoRed[NeoPixel] = 0; NeoWhite[NeoPixel] = 0; }      
   }
}
void NeoDraw (void)
{
   uint8_t NeoPixel;
   signed int BitCount;
   for (NeoPixel = 0; NeoPixel < NeoNum; NeoPixel++)
   {    
      for (BitCount = 7; BitCount >= 0; BitCount--)      
         NeoBit(bit_test(NeoGreen[NeoPixel], BitCount));      
      for (BitCount = 7; BitCount >= 0; BitCount--)           
         NeoBit(bit_test(NeoRed[NeoPixel], BitCount));            
      for (BitCount = 7; BitCount >= 0; BitCount--)      
         NeoBit(bit_test(NeoBlue[NeoPixel], BitCount));
      for (BitCount = 7; BitCount >= 0; BitCount--)      
         NeoBit(bit_test(NeoWhite[NeoPixel], BitCount)); 
   }
   //output_low (NeoPin);
   asm("bcf LATD,0");
}
void NeoRotate (void)
{
   uint8_t NeoPixel;   
   for (NeoPixel = 0; NeoPixel < NeoNum - 1; NeoPixel++)   
   {           
      NeoGreen[NeoPixel] = NeoGreen[NeoPixel + 1];
      NeoBlue[NeoPixel] = NeoBlue[NeoPixel + 1];
      NeoRed[NeoPixel] = NeoRed[NeoPixel + 1];
      NeoWhite[NeoPixel] = NeoWhite[NeoPixel + 1];
   }
   NeoGreen[NeoNum - 1] = NeoGreen[0];
   NeoBlue[NeoNum - 1] = NeoBlue[0];
   NeoRed[NeoNum - 1] = NeoRed[0];
   NeoWhite[NeoNum - 1] = NeoWhite[0];
}

void GreenLED(void)
{
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptDisable();

        uint8_t a=0;
        for(a=0; a<25; a++)
        {
        
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        }
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptEnable();
        __delay_us(60);
        
}

void BlueLED(void)
{
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptDisable();

        uint8_t a=0;
        for(a=0; a<25; a++)
        {
        
        
        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
  
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
             
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
/*        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
*/        
        }
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptEnable();
        __delay_us(60);
        
}

void RedLED(void)
{
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptDisable();

        uint8_t a=0;
        for(a=0; a<25; a++)
        {
            
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
       
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        }
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptEnable();
        __delay_us(60);
        
}

void WhiteLED(void)
{
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptDisable();

        uint8_t a=0;
        for(a=0; a<25; a++)
        {
 
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
       
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
        pulse_00();
 
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
        pulse_01();
       
        }
//        INTERRUPT_PeripheralInterruptEnable();
        INTERRUPT_GlobalInterruptEnable();
        __delay_us(60);
        
}

void main(void)
{
    unsigned long ip;
    time_t ip_timer;
    bool dataBaseVersionChanged=false;
    
    SYSTEM_Initialize();
    ledNRZport_Disable();
    NeoInit ();

#ifdef IF_LIGHT
//    char value[16];   //jira: CAE_MCU8-5647
    LLDP_InitRxTx(); // you can choose one or the other as well Rx or Tx only
    LLDP_SetDesiredPower(requiredPower);
    LLDP_setPortDescription("PortDescription");
    LLDP_setAssetID("AssetID");
    LLDP_setFirmwareRevision("FirmwareRev");
    LLDP_setHardwareRevision("HardwareRevShev");
    LLDP_setSoftwareRevision("SoftwareRev");
    LLDP_setSerialNumber("SerialNum");
    LLDP_setManufacturer("Manufacturer");
    LLDP_setModelName("ModelName");
    LLDP_setMUDInfo("I am secure");

            
#endif
    // enable the high priority interrupts
    IPEN = 1;

    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    Network_WaitForLink();  
    
    logMsg("link started", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    time(&ip_timer);
    
    led_status.DHCP_INDICATOR = 0;
    target_power_level_pwm[0] = Curve[25] >> 2;
    
if(SWITCH_S1_PORT == 0)
{
    forceUpdate = true;
}

    
// The #ifndef IF_TOGGLE is present here since the IF_TOGGLE configuration does  
// not required a datamodel. Broadcast packets are sent out to control all the 
// lights in the current application.
#ifndef IF_TOGGLE 

    dataBaseVersionChanged = databaseChanged(dataBaseVersion, forceUpdate); //jira: M8TS-679
    obj_init(); //jira: M8TS-679
    string_init();
    graph_init();
#endif
    
    
#ifdef IF_POT
    pot_init();
#endif

#ifdef IF_GEST
    gestIC_init();
#endif
    
   
#ifndef IF_TOGGLE  
    if(dataBaseVersionChanged)
    {
        int16_t databaseSuccess=0; //jira: M8TS-679
        //Everything should come under create DataBase
        led_status.DBUpdate = 1;
        printf("Create a new database\n"); //jira: M8TS-679
        
        initResourceUUID();//TODO :remove and add to create resource dynamically
        if((databaseSuccess= createDatabase())!= -1) //jira: M8TS-679
        {
            updateDatabaseVersion(dataBaseVersion); //jira: M8TS-679
        }
        led_status.DBUpdate =0;

    }
    else{
        printf("reusing saved database\n"); //jira: M8TS-679
        localResourceInit();
    }
#endif

#ifdef ENABLE_BOOTLOADER
    addDFDResource();
    addDFUResource();
#endif
    
//    addActOrSenResource("actuator1",ACTUATOR_TYPE);
    
#ifdef IF_LIGHT
    addActOrSenResource((char *)"actuator1",ACTUATOR_TYPE);
#endif
    
#ifdef IF_IO_STARTER_EXPANSION
    addActOrSenResource((char *)"sensor1",SENSOR_TYPE);   //jira: CAE_MCU8-5647
    addActOrSenResource((char *)"sensor2",SENSOR_TYPE);   //jira: CAE_MCU8-5647
#endif
    
    ledNRZport_Enable();

    while(1)
    {
//        time_t now = 0;
//        time(&now);        
//        blip();
//        blip();
        
//        RedLED();
//        __delay_ms(1000);
//        GreenLED();
//        __delay_ms(1000);
//        BlueLED();
//        __delay_ms(1000);
//        WhiteLED();
//        __delay_ms(1000);
//        __delay_ms(1000);

        
        
        INTERRUPT_GlobalInterruptDisable();
        NeoDraw ();
        NeoRotate ();
        INTERRUPT_GlobalInterruptEnable();        
        __delay_ms(1);

        Network_Manage();
        
#ifdef ENABLE_BOOTLOADER
        dfdBootProcess();
        dfuBootProcess();
#endif
        
#ifdef IF_LIGHT
        LLDP_Run();
        LLDP_SetDesiredPower(requiredPower);              
#endif
//        time(&t);        
        ip = ipdb_getAddress();     
        if(ip)
        {
            led_status.DHCP_INDICATOR = 1;              
            
            if(Button_Press())
            {
                switch(pushNum)
                {
                    case 1:
                        DeviceDiscover(0xFFFFFFFF,65527); 
                        pushNum++;
                        break;
                    case 2: 
                        printf("\r\n Button Press:%d Adding 2 actuators  \r\n",pushNum);

                        addActOrSenResource((char *)"actuator1",ACTUATOR_TYPE);   //jira: CAE_MCU8-5647
                        addActOrSenResource((char *)"actuator2",ACTUATOR_TYPE);   //jira: CAE_MCU8-5647
                        
                        printf("\r\n Added two actuator resources \r\n");     
                        pushNum++;
                        break;      
                    case 3: 
                         printf("\r\n Button Press:%d adding a sensor  \r\n",pushNum);
                         
                         addActOrSenResource((char *)"sensor1",SENSOR_TYPE);      //jira: CAE_MCU8-5647  
                        
                        printf("\r\n Added a new Sensor Resource \r\n");
                        pushNum++;
                        break;
                    case 4: 
                         printf("\r\n Button Press:%d removing an actuator  \r\n",pushNum);
                         removeThisResource((char *)"actuator2",ACTUATOR_TYPE);  //jira: CAE_MCU8-5647
                        
                        
                        printf("\r\n Removed A Resource \r\n");
                        pushNum++;
                        break;
                    case 5: 
                         printf("\r\n Button Press:%d adding many actuators \r\n",pushNum);
                         
                         addActOrSenResource((char *)"actuator2",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator3",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator4",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator5",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator6",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator7",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator8",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator9",ACTUATOR_TYPE); //jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator10",ACTUATOR_TYPE);//jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator11",ACTUATOR_TYPE);//jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator12",ACTUATOR_TYPE);//jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator13",ACTUATOR_TYPE);//jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator14",ACTUATOR_TYPE);//jira: CAE_MCU8-5647
                         addActOrSenResource((char *)"actuator15",ACTUATOR_TYPE);//jira: CAE_MCU8-5647

                         
                        printf("\r\n Added new Resources \r\n");
                        pushNum++;
                        break;
                    default:
                        pushNum=1;
                        break;
                }
                DeviceDiscover(0xFFFFFFFF,65527); 
                printf("\r\n Device IP Address: %d.%d.%d.%d \r\n",((char*)&ip)[3],((char*)&ip)[2],((char*)&ip)[1],((char*)&ip)[0]);
            }   
        }
    }
    return;
}
    

