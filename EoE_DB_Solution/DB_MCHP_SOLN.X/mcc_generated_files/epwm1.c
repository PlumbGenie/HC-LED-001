/**
  ECCP1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    eccp1.c

  @Summary
    This is the generated driver implementation file for the ECCP1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs 

  @Description
    This source file provides APIs for ECCP1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs  - 1.45
        Device            :  PIC18F67J60
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include "epwm1.h"
#include "hardware.h"

/**
  Section: Macro Declarations
*/

#define PWM1_INITIALIZE_DUTY_VALUE    0

/**
  Section: EPWM Module APIs
*/

uint16_t target_power_level_pwm[2];
uint16_t current_power_level_pwm[2];

uint32_t fadeDelta;
volatile uint32_t fadeCount;
fadeFlags stateFlags;

volatile fade_counter fadeAccumulator;

void EPWM1_Initialize (void)
{
    // Set the PWM to the options selected in PIC10 / PIC12 / PIC16 / PIC18 MCUs 
    // CCP1M P1A,P1C: active high; P1B,P1D: active high; DC1B 0; P1M single; 
    CCP1CON = 0x0C;
    
    // P1RSEN automatic_restart; P1DC0 0; 
    ECCP1DEL = 0x80;
    
    // ECCPASE operating; PSSBD P1BP1D_0; PSSAC P1AP1C_0; ECCPAS disabled; 
    ECCP1AS = 0x00;
    
    // CCPR1L 0; 
    CCPR1L = 0x00;
    
    // CCPR1H 0; 
    CCPR1H = 0x00;
    
    //Manually add:
    PR2 = 0xFF;
    T2CON = 0b00001110;   // Post 1:1, Timer ON, Pre 1:16
    while(TMR2IF==0);  // Wait for timer overflow before enabling port
    
    current_power_level_pwm[0]=0;
    target_power_level_pwm[0]=0;
    
    current_power_level_pwm[0]=0;
    target_power_level_pwm[0]=0;
    
}

void EPWM1_LoadDutyValue(uint16_t dutyValue)
{
   // Writing to 8 MSBs of pwm duty cycle in CCPRL register
    CCPR1L = ((dutyValue & 0x03FC)>>2);
   // Writing to 2 LSBs of pwm duty cycle in CCPCON register
    CCP1CON = ((uint8_t)(CCP1CON & 0xCF) | ((dutyValue & 0x0003)<<4));
}

//Manually add:
void pwm1_dim(void)
{
    if(target_power_level_pwm[0] == current_power_level_pwm[0]) 
    {
        return;
    }
    else if((fadeCount == 0) && (target_power_level_pwm[0] != current_power_level_pwm[0]))
    {
        current_power_level_pwm[0] = target_power_level_pwm[0];
        EPWM1_LoadDutyValue(current_power_level_pwm[0]);
    }
    else if((fadeCount > 0) && (target_power_level_pwm[0] != current_power_level_pwm[0]))
    {
        fadeCount--;
        if(fadeCount == 0)
        {
            current_power_level_pwm[0] = target_power_level_pwm[0];
            stateFlags.fadeUp = stateFlags.fadeDown = 0;
        }
        else
        {
            // Not the last step of a fade. We should just update
            // the fade counter by applying the increment/decrement
        //Decrement counter and signal that the fader is running
            if(stateFlags.fadeUp == 1)
            {
                fadeAccumulator.counter += fadeDelta;
            }
            else if(stateFlags.fadeDown == 1) 
            {
                fadeAccumulator.counter -= fadeDelta;
            }
            current_power_level_pwm[0] = fadeAccumulator.byte[2];
            EPWM1_LoadDutyValue(current_power_level_pwm[0]);
        }
    }
}

void pwm1_fadeDelta(void)
{
    if (stateFlags.fadeUp == 1)
    {
        fadeDelta = target_power_level_pwm[0] - current_power_level_pwm[0];
    }
    else if (stateFlags.fadeDown == 1)
    {
        fadeDelta = current_power_level_pwm[0] - target_power_level_pwm[0];
    }          
    fadeDelta <<= 16;
    fadeDelta /= fadeCount;
    // Load accumulator values
    fadeAccumulator.byte[0] = 0;
    fadeAccumulator.byte[1] = 0;
    fadeAccumulator.byte[2] = (uint8_t)current_power_level_pwm[0];
    fadeAccumulator.byte[3] = current_power_level_pwm[0] >> 8;
}
/**
 End of File
*/
