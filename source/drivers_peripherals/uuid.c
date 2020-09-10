/*
 * File:   uuid.c
 */
/**
  RFC 4122 implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    uuid.c

  Summary:
     This is the implementation of RFC 4122

  Description:
    This source file provides the implementation of the API for the UUID generation.

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

/* The following information applies to variant #1 UUIDs:
 *
 *
 * The adjusted time stamp is split into three fields, and the clockSeq
 * is split into two fields.
 * Instead of clock_seq_hi_and_reserved device ID field is used in the implementation so that a unique 8 bit ID can be assigned to each device
 * Node ID is the MAC address of the device
 *
 * |<------------------------- 32 bits -------------------------->|
 *
 * +--------------------------------------------------------------+
 * |                     low 32 bits of time                      |  0-3  .time_low
 * +-------------------------------+-------------------------------
 * |     mid 16 bits of time       |  4-5               .time_mid
 * +-------+-----------------------+
 * | vers. |   hi 12 bits of time  |  6-7               .time_hi_and_version
 * +-------+-------+---------------+
 * |Res|  clkSeqHi |  8                                  .device ID instead of .clock_seq_hi_and_reserved
 * +---------------+
 * |   clkSeqLow   |  9                                 .clock_seq_low
 * +---------------+----------...-----+
 * |            node ID               |  8-16           .node
 * +--------------------------...-----+
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "tcpip_config.h"
#include "tmr1.h"
#include "rtcc.h"
#include "ipv4.h"
#include "udpv4.h"
#include "network.h"
#include "uuid.h"
#include "ethernet_driver.h"
#include "tcpv4.h"
#include "adc.h"
#include "mac_address.h"
#include "dns_client.h"
#include "ntp.h"
#include "udpv4_port_handler_table.h"
#include "mac_eeprom.h"
#include "uuid.h"

#include "hardware.h"


//bool no_ntp = true;    //jira: CAE_MCU8-5647

/*******************************************Function prototypes ********************************************/

/*make a UUID from the timestamp, clockseq and node ID */
static void format_uuid_v1(mchp_uuid_t* uuid, mchp_uuid_time_clk_seq_t timestamp, uuid_node_t node);

/*get time from NTP server. Time can also be obtained alternatively using RTCC */
static mchp_uuid_time_clk_seq_t  get_NTP_Time_clk_seq(void);
static uint16_t generate_random(void);

/*assign the unique device ID as required for the device_Number field in UUID */
/* Currently not using this functionality */
//static uint8_t assign_Device_Number(uint8_t device_num);    //jira: CAE_MCU8-5647


static void format_uuid_v1(mchp_uuid_t* uuid, mchp_uuid_time_clk_seq_t timestamp, uuid_node_t node)
{ 
    uuid->time_low = (unsigned long)(timestamp.lo & 0xFFFFFFFF);
    uuid->time_mid = (unsigned short)((timestamp.hi) & 0xFFFF);
    uuid->time_hi_and_version = (unsigned short)((timestamp.hi & 0xFFFF0000)>> 16);
    
    uuid->clock_seq_low = timestamp.clk_seq_low & 0xFFu;           //jira: CAE_MCU8-5647
    uuid->clock_seq_hi_and_reserved = (timestamp.clk_seq_high_and_version & 0x3Fu) >> 8u;  //jira: CAE_MCU8-5647
    uuid->clock_seq_hi_and_reserved |= 0x80u;                      //jira: CAE_MCU8-5647
    uuid->clock_seq_hi_and_reserved &= 0xBFu;                      //jira: CAE_MCU8-5647
//    uuid->device_ID.dev_Num = device_num;
    memcpy(uuid->node, &node, sizeof uuid->node);   
}


static mchp_uuid_time_clk_seq_t get_NTP_Time_clk_seq(void)
{
    
    char date[20];    
    char timeString[16];
    int prev_min = 0;                         //jira: CAE_MCU8-5647
    static int inited = 0;
    uint32_t mchp_uuid_time = 0;
    uint64_t NTP_timestamp;
    mchp_uuid_time_clk_seq_t mchp_uuid_timestamp;
    struct tm *recv_fraction;
         
    time_t theTime = time(NULL);
    recv_fraction = gmtime(&theTime);
    
    memset(date,0,sizeof(date));      
    sprintf(date,"Date: %d-%d-%d", recv_fraction->tm_mon+1, recv_fraction->tm_mday, recv_fraction->tm_year+1900);
    memset(timeString,0, sizeof(timeString));
    sprintf(timeString, "Time: %d:%d:%d", recv_fraction->tm_hour, recv_fraction->tm_min, recv_fraction->tm_sec);
    
    mchp_uuid_time = ((uint32_t)(recv_fraction->tm_hour) * 3600) + ((uint16_t)(recv_fraction->tm_min) * 60) + ((uint8_t)(recv_fraction->tm_sec));
    NTP_timestamp.n2  = 0x00000000;
    NTP_timestamp.n1  = mchp_uuid_time;
    
    mchp_uuid_timestamp.lo = uuid_base_time_diff_lo + NTP_timestamp.n1;
    mchp_uuid_timestamp.hi = uuid_base_time_diff_hi + NTP_timestamp.n2;
    sprintf(date,"%lu",(unsigned long)mchp_uuid_time);
    
    // Handler overflows
    if (mchp_uuid_timestamp.lo < uuid_base_time_diff_lo || mchp_uuid_timestamp.lo < NTP_timestamp.n1)
    {
        mchp_uuid_timestamp.hi += 1;
    }            
    
    //For UUID version information (currently using version 1)
    mchp_uuid_timestamp.hi |= uuid_version1_mask;                
    sprintf(date,"%lu", (unsigned long)mchp_uuid_timestamp.hi);
    if(recv_fraction->tm_min != prev_min)
    {
        prev_min = recv_fraction->tm_min;                
    }     
    if (!inited) 
    {
        srand(mchp_uuid_time);
        inited = 1;
    }
    mchp_uuid_timestamp.clk_seq_low = (uint8_t)rand();    //jira: CAE_MCU8-5647
    
    return mchp_uuid_timestamp;
}

//jira: CAE_MCU8-5647
//static uint8_t assign_Device_Number(uint8_t device_num)
//{
//    uint8_t assigned_Number;
//    
//    assigned_Number = device_num;
//    
//    return assigned_Number;
//}


uint8_t uuid_compare(mchp_uuid_t *u1, mchp_uuid_t *u2)
{
    uint8_t i;
//jira: CAE_MCU8-5647
//    COMPARE_CHECK(u1->time_low, u2->time_low);
//    COMPARE_CHECK(u1->time_mid, u2->time_mid);
//    COMPARE_CHECK(u1->time_hi_and_version, u2->time_hi_and_version);
//    COMPARE_CHECK(u1->clock_seq_hi_and_reserved, u2->clock_seq_hi_and_reserved);
//    COMPARE_CHECK(u1->clock_seq_low, u2->clock_seq_low);
    
    for (i = 0; i < 6u; i++) {
        if (u1->node[i] < u2->node[i])
            return -1;
        if (u1->node[i] > u2->node[i])
            return 1;
    }
    return 0;
}


mchp_uuid_t uuid_eeprm_Read_and_Send(void)
{
    mchp_uuid_t uuid_current;
    char UUID_buffer_current[30];
    
    macEeprReadBlock (UUID_MEMORY_ADDR, &uuid_current, sizeof(mchp_uuid_t));
    memset(UUID_buffer_current, 0, sizeof(UUID_buffer_current)); 
    sprintf(UUID_buffer_current, "%.8lX%.4X%.4X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X",(long)uuid_current.time_low, uuid_current.time_mid, uuid_current.time_hi_and_version, 
                                                                                 uuid_current.clock_seq_hi_and_reserved, uuid_current.clock_seq_low,
                                                                                 uuid_current.node[0],uuid_current.node[1],uuid_current.node[2],uuid_current.node[3],uuid_current.node[4],uuid_current.node[5]);
   
    bool started =(bool) UDP_Start(0xFFFFFFFFu,SOURCEPORT_UUID,DESTPORT_UUID);   //jira: CAE_MCU8-5647
    if(started == SUCCESS)
    {
        ETH_WriteBlock(UUID_buffer_current, sizeof(UUID_buffer_current));
        UDP_Send();
    }
    
    return uuid_current;
}


void uuid_eeprm_Read(char *uuid)
{  
    macEeprReadBlock(UUID_MEMORY_ADDR, uuid, UUID_STR_SIZE); 
}


void uuid_eeprm_Write(char *uuid)
{
    macEeprWriteBlock(UUID_MEMORY_ADDR,uuid,UUID_STR_SIZE);       
}


void uuid_create(mchp_uuid_t *uuid)
{
    uuid_node_t node;
    mchp_uuid_time_clk_seq_t UUID_currentTime_clk_seq;
    const mac48Address_t *mac;
            
    //To obtain time from NTP and UUID version information and clock_seq   
    UUID_currentTime_clk_seq = get_NTP_Time_clk_seq();                    
                                      //To obtain the MAC address for UUID node
    mac = MAC_getAddress();
    node.nodeID[0] = mac->mac_array[0]; 
    node.nodeID[1] = mac->mac_array[1]; NO_OPERATION();
    node.nodeID[2] = mac->mac_array[2]; NO_OPERATION();
    node.nodeID[3] = mac->mac_array[3]; NO_OPERATION();
    node.nodeID[4] = mac->mac_array[4]; NO_OPERATION();
    node.nodeID[5] = mac->mac_array[5]; NO_OPERATION();
                            
//    device_num = assign_Device_Number(device_Num);
    
    /* stuff fields into the UUID */
    format_uuid_v1(uuid, UUID_currentTime_clk_seq, node);
}


void uuidValidate(char *uuid_str)
{    
    unsigned char uuid_created;
    mchp_uuid_t uuid_var;
    
    uuid_created = macEeprRead(UUID_VALID_ADDR);
    uuid_eeprm_Read(uuid_str);
    
    if((uuid_created != UUID_CREATED) || strlen(uuid_str)==0)
    {
        printf("Creating UUID \r\n");
        uuid_create(&uuid_var);
        sprintf(uuid_str,"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",((char*)&uuid_var.time_low)[3],((char*)&uuid_var.time_low)[2],((char*)&uuid_var.time_low)[1],((char*)&uuid_var.time_low)[0],
                ((char*)&uuid_var.time_mid)[1],((char*)&uuid_var.time_mid)[0],
                ((char*)&uuid_var.time_hi_and_version)[1],((char*)&uuid_var.time_hi_and_version)[0],
                ((char*)&uuid_var.clock_seq_hi_and_reserved)[0],
                ((char*)&uuid_var.clock_seq_low)[0],
                (uuid_var.node[0]),(uuid_var.node[1]),(uuid_var.node[2]),(uuid_var.node[3]),(uuid_var.node[4]),(uuid_var.node[5]));
       
        uuid_eeprm_Write(uuid_str);
        macEeprWrite(UUID_VALID_ADDR,UUID_CREATED);
    }
    else
    {
        printf("uuid_str = %s \r\n", uuid_str);
    }
}
