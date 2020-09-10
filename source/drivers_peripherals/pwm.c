/**
 PWM implementation

  Company:
    Microchip Technology Inc.

  File Name:
    pwm.c

  Summary:
    PWM implementation.

  Description:
    This file provides the PWM implementation.

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

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "pin_manager.h"
#include "pwm.h"

uint16_t pwm_limit;
uint16_t target_power_level_pwm[2];
uint16_t current_power_level_pwm[2];

void pwm_init(void)
{
    PWM1_PORT_TRIS = 1; // Disable port for set up
    

    PR2 = 0xFF;
    

    CCPR1L=0;
    CCPR1H=0;
    CCPR5L= 0;
    CCPR5H= 0;
    CCP1CON = 0b00001100;  // Single Output, PWM Mode, Active High
    ECCP1AS = 0x00;
    
    CCP5CON = 0b00001100; 

    T2CON = 0b00001110;   // Post 1:1, Timer ON, Pre 1:16

    while(TMR2IF==0);  // Wait for timer overflow before enabling port
    

    PWM1_PORT_TRIS = 0; // re-enable port.

    current_power_level_pwm[0]=0;
    target_power_level_pwm[0]=0;
    
    current_power_level_pwm[1]=0;
    target_power_level_pwm[1]=0;
    
    pwm_limit = 250; //Initial limit is 25%

    
    
}



void pwm1_set(uint16_t p)
{
    uint8_t PWMH, PWML;

//    if(p > pwm_limit)p=pwm_limit;    // Limit to 10bit range


    PWMH=(uint8_t)(p>>2);  // Get high 8 bits

    p &= 0x03;             // Get low 2 bits in position
    PWML=(uint8_t)(p<<4);


    CCP1CON &= 0x0F;  // Load Low bits of PWM
    CCP1CON |= PWML;


    CCPR1L=PWMH;       // Load high byte of PWM


}

void pwm2_set(uint16_t p)
{
    uint8_t PWMH, PWML;

//    if(p > 1023)p=1023;    // Limit to 10bit range


    PWMH=(uint8_t)(p>>2);  // Get high 8 bits

    p &= 0x03;             // Get low 2 bits in position
    PWML=(uint8_t)(p<<4);

 // Load high byte of PWM
//    
    CCP5CON &= 0x0F;  // Load Low bits of PWM
    CCP5CON |= PWML;


    CCPR5L=PWMH;     


}


void pwm1_dim(void)
{
    if(target_power_level_pwm[0] == current_power_level_pwm[0]) return;
    

    if(target_power_level_pwm[0] >current_power_level_pwm[0])
         {
             ++current_power_level_pwm[0];
             pwm1_set(current_power_level_pwm[0]);


         }
    else if(current_power_level_pwm[0] > target_power_level_pwm[0])
         {
             --current_power_level_pwm[0];
             pwm1_set(current_power_level_pwm[0]);

         }
}
//
void pwm2_dim(void)
{
    if(target_power_level_pwm[1] >current_power_level_pwm[1])
         {
             ++current_power_level_pwm[1];
             pwm2_set(current_power_level_pwm[1]);

         }
     if(current_power_level_pwm[1] > target_power_level_pwm[1])
         {
             --current_power_level_pwm[1];
             pwm2_set(current_power_level_pwm[1]);
         }
}

void pwm_setLimit(uint16_t limit)
{
    pwm_limit = 2 * limit;
    if (pwm_limit > 1023) pwm_limit=1023;
}