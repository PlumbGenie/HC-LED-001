/**
 Timer 1 driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    tmr1.c

  Summary:
    Timer 1 driver implementation.

  Description:
    This file provides the Timer 1 driver implementation.

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

/*
 * File:   tmr1.c
 * Author: C16008
 *
 * Created on January 14, 2016, 1:41 PM
 */


#include <xc.h>
#include "tmr1.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "hardware.h"
#include "led_board.h"
#include "rtcc.h"


volatile uint16_t seconds_counter;
volatile uint8_t PoESACount;

void tmr1_init(void)
{
    TMR1CS = 0;     // Internal Clock [FOSC/4]
    T1OSCEN = 1;    // enable crystal driver
    T1CKPS0 = 0;    // no pre scaler
    T1CKPS1 = 0;
    T1SYNC = 1;     // synchronize the T1
    TMR1ON = 1;

    TMR1 = TMR1_RELOAD; //tmr1 reload
    

    TMR1IP = 1; // High Priority
    TMR1IE = 1; // Interrupt Enable
    seconds_counter = TMR1_TICKS_PER_SECOND;    
}

inline void maintain_PoESA(void)
{
    PoESACount--;
    if(PoESACount ==0)
    {
        if (POE_STAY_ALIVE_LAT) PoESACount = PoESA_OFF_TIME;
        else PoESACount = PoESA_ON_TIME;
        POE_STAY_ALIVE_LAT ^= 1;
    }
}


void tmr1_handler(void)
{

    if(TMR1IE && TMR1IF)
    {
        TMR1IF = 0;
        TMR1ON = 0;
        TMR1 = TMR1_RELOAD;//tmr1 reload
        TMR1ON = 1;
        seconds_counter--;
        if(seconds_counter == 0)
        {
            seconds_counter = TMR1_TICKS_PER_SECOND;
            rtcc_handler();            
            led_update();
        }
    }
    maintain_PoESA();
}