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


//const char *dataBaseID = "DBID";   //jira: CAE_MCU8-5647

void main(void)
{
    unsigned long ip;
    time_t ip_timer;
    bool dataBaseVersionChanged=false;
    
    SYSTEM_Initialize();

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

    while(1)
    {
//        time_t now = 0;
//        time(&now);        
        
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
    

