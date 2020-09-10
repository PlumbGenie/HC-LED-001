/**
 GestIC driver API Header File

  Company:
    Microchip Technology Inc.

  File Name:
   gestic.h

  Summary:
    Header file for the GestIC implementation.

  Description:
    This header file provides APIs for the GestIC driver implementation

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

#ifndef _GESTIC_H
#define _GESTIC_H

/** GENERAL DEFINES *******************************************************/

void gestIC_init(void);
void gestIC_Clearbuff(void); 
void gest_process(void);

//========= COMMAND STRUCTURE ==============
//  Format: [C][C][C][C][C][C][C][R/W]
//  C = Command
//  R/W = Read or write request  0 = Write 1= Read
//  The commands defiend below are either read, write or both
//   [R] in the comment indicates read Only
//   [W] in the comment indicates write only
//   [RW] in the comment indicates read or write
// Note these options may change in the future.

// List of Commands 

#define DATA_WRITE         0    // LSB 0 writes data to the GestIC board
#define DATA_READ          1    // LSB 1 reads data from the gest board - Read ONLY reads levels for now 

#define ZONE_SINGLE     0x02    // [W]  Update a single Zone
#define ZONE_ALL        0x04    // [RW] Update or read all zones 
#define ZONE_NAME       0x06    // [W]  Update a zones name
#define IP_STRING       0x08    // [W]  Update the IP string displayed
#define CONSOLE_STRING  0x0A    // [W]  Write a line to the console

#define SPIBUFFMAX      17      // Max Packet / Buffer size
#define MAX_ZONES       4


typedef union
{
    struct // ZONE_SINGLE
    {
//        uint8_t command;
        uint8_t zone;     // Zone to update    
        uint8_t level;    // New level for that zone
    }SingleLevel;
    
    struct // ZONE_ALL
    {
//        uint8_t command;
        uint8_t levels[MAX_ZONES];
    }AllLevels;
    
    struct  // ZONE_NAME
    {
//        uint8_t command;
        uint8_t zone;    // Zone number to set the name for
        char name[16];
    }ZoneName;
    
    struct   // IP_STRING
    {
//        uint8_t command;
        char ipaddress[16];
        
    }IPAddress;
    
    struct   // CONSOLE_WRITE
    {
//        uint8_t command;
        char string[16]; 
    }Console;
    
    
    char packet[SPIBUFFMAX];  // Raw packet data
    
}GESTPACKET;
//void GestIC_Init(void);
//void GestIC_Read(void);

void GestIC_Read();
void GestIC_Write(uint8_t command, void *data, uint8_t len);
void GestIC_Update(void);

 void Gest_PacketRecvd(uint8_t zone, uint8_t level);
//void DEMO_TCP_echo_server(char *message);

extern uint16_t gest_power_level_act[MAX_ZONES];




#endif