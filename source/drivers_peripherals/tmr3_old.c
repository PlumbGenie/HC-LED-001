/**
 Timer 3 driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    tmr3.c

  Summary:
    Timer 3 driver implementation.

  Description:
    This file provides the Timer 3 driver implementation.

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
 * File:   tmr3.c
 * Author: C16008
 *
 * Created on February 2, 2016, 11:37 AM
 */


#include <xc.h>
#include <stdint.h>
#include "tmr3.h"
#include "pwm.h"

volatile uint16_t tmr3_seconds_counter;
volatile uint8_t flag_1ms, pot_10ms, pot_1ms, pwm_1ms, pwm_10ms;
volatile uint16_t gest500ms;
volatile uint8_t gest200ms;

void tmr3_init(void)
{
    TMR3CS = 0;     // Internal Clock [FOSC/4]    
    T3CKPS0 = 0;    // no pre scaler
    T3CKPS1 = 0;
    T3SYNC = 1;     // synchronize the T3
    TMR3ON = 1;

    TMR3 = TMR3_RELOAD; //tmr3 reload
    flag_1ms = 0;
    pot_10ms =0 ;
    pot_1ms =0;
    pwm_1ms=0;
    pwm_10ms=0;
    gest500ms = 0;
    gest200ms = 0;

    TMR3IP = 1; // High Priority
    TMR3IE = 1; // Interrupt Enable
    tmr3_seconds_counter = TMR3_TICKS_PER_SECOND;
}

void tmr3_handler(void)
{
    static uint8_t Countms=0;
    
    if(TMR3IE && TMR3IF)
    {
        TMR3IF = 0;
        TMR3ON = 0;
        TMR3 = TMR3_RELOAD;//tmr3 reload
        TMR3ON = 1;
       
        Countms++;
        
        pwm_1ms=1;
        pot_1ms=1;
        
        pwm1_dim();
        
        tmr3_seconds_counter--;
        if(tmr3_seconds_counter == 0)
        {       
            tmr3_seconds_counter = TMR3_TICKS_PER_SECOND; 
        }
        if(Countms==10)
        {
            pot_10ms=1;
            pwm_10ms=1;
            Countms=0;
            gest500ms += 10;
            gest200ms +=10;
        }
            
        }
    }
