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

#ifndef _GESTIC_H
#define _GESTIC_H


/** GENERAL DEFINES *******************************************************/

struct dataXChangeT {
        uint8_t updated;
        uint8_t Xpos, Ypos, Zpos;
        uint8_t PosValid:1; 
        uint8_t flick_we; 
        uint8_t flick_ew; 
        uint8_t flick_sn; 
        uint8_t flick_ns; 
        uint8_t touch_east;
        uint8_t touch_west;
        uint8_t touch_center;
        uint8_t airWheelactive;
        int8_t airWheelcnt;
};

extern uint16_t gest_power_level_act[2];
extern struct dataXChangeT dataXChange;
extern void clear_Xchange(void);


#define MSGID_SENSORDATAOUT 0x91

//StreamingOutput Mask (DataOutputConfig), bitmask, 2Bytes
#define STREAMOUT_DSPINFO      0x0001               //b0 : DSPInfo field
#define STREAMOUT_GESTUREINFO  0x0002               //b1 : GestureInfo field
#define STREAMOUT_TOUCHINFO    0x0004               //b2 : TouchInfo field
#define STREAMOUT_AIRWHEELINFO 0x0008               //b3 : AirWheelInfo field
#define STREAMOUT_XYZPOSITION  0x0010               //b4 : XYZPosition field

//SytemInfo Mask, bitmask, 1Byte
#define SI_POSVALID         0x01                    //b0 : PositionValid
#define SI_AIRWHEELVALID    0x02                    //b1 : AirWheelValid

//Tap Bitmask
#define BITSHIFT_TAP_SOUTH       5
#define BITSHIFT_TAP_WEST        6
#define BITSHIFT_TAP_NORTH       7
#define BITSHIFT_TAP_EAST        8
#define BITSHIFT_TAP_CENTER      9

//Gesture Info (decimal number)
#define GI_NOGESTURE        0
#define GI_GARBAGEMOD       1
#define GI_FLICK_WE         2
#define GI_FLICK_EW         3
#define GI_FLICK_SN         4
#define GI_FLICK_NS         5

//Tap Info
#define GI_TAP_WEST         50
#define GI_TAP_EAST         51
#define GI_TAP_SOUTH        52
#define GI_TAP_NORTH        53
#define GI_TAP_CENTER       54

//TouchInfo (bitshft)
#define TOUCH_NORTH_BIT 2
#define TOUCH_EAST_BIT 3
#define TOUCH_SOUTH_BIT 0
#define TOUCH_WEST_BIT 1
#define TOUCH_CENTER_BIT 4

//Airwheel Info
#define GI_AIRWHEEL_CW      90
#define GI_AIRWHEEL_CCW     91


void gestIC_init(void);
void gestIC_Clearbuff(void); 
void gest_process(void);
//void DEMO_TCP_echo_server(char *message);






#endif