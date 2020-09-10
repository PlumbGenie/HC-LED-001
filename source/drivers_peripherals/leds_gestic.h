/**
 GestIC LEDs driver API Header File

  Company:
    Microchip Technology Inc.

  File Name:
   leds_gestic.h

  Summary:
    Header file for the GestIC LEDs implementation.

  Description:
    This header file provides APIs for the GestIC LEDs driver implementation

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

#ifndef _I2CLED_H
#define _I2CLED_H

void led_write(uint8_t level, uint8_t indicators);

typedef union
{
        struct
        {
            unsigned int Zone1 :1 ;
            unsigned int Zone2 :1 ;
            unsigned int DimUp :1 ;
            unsigned int DimDown :1 ;
            unsigned int Background :1;
            unsigned int spare :3;
        };
        uint8_t dat;

}OTHERLEDS;



//
//typedef struct
//{
//    uint8_t lightLevel;
//    char lightName[6];
//} lightModel_t;
//
//
//lightModel_t lightModel[2];




#endif
