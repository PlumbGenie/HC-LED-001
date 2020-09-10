/**
  TMR3 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    tmr3.c

  @Summary
    This is the generated driver implementation file for the TMR3 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs 

  @Description
    This source file provides APIs for TMR3.
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
#include "tmr3.h"

#include "epwm1.h"
/**
  Section: Global Variable Definitions
*/
volatile uint16_t timer3ReloadVal;

volatile uint16_t tmr3_seconds_counter;
volatile uint8_t flag_1ms, pot_10ms, pot_1ms, pwm_1ms, pwm_10ms;

void (*TMR3_InterruptHandler)(void);

/**
  Section: TMR3 APIs
*/

void TMR3_Initialize(void)
{
    //Set the Timer to the options selected in the GUI

    //T3CKPS 1:1; TMR3CS FOSC/4; nT3SYNC synchronize; TMR3ON off; T3CCP1 tmr1_and_tmr2_for_ECCPx/CCPx; RD16 disabled; 
    T3CON = 0x04;

    //TMR3H 215; 
    TMR3H = 0xD7;

    //TMR3L 80; 
    TMR3L = 0x50;

    // Load the TMR value to reload variable
    timer3ReloadVal=TMR3;

    TMR3IP = 1; // High Priority
    // Clearing IF flag before enabling the interrupt.
    PIR2bits.TMR3IF = 0;

    // Enabling TMR3 interrupt.
    PIE2bits.TMR3IE = 1;

    // Set Default Interrupt Handler
    TMR3_SetInterruptHandler(TMR3_DefaultInterruptHandler);

    // Start TMR3
    TMR3_StartTimer();
    //Manually add the following:
    flag_1ms = 0;
    pot_10ms =0 ;
    pot_1ms =0;
    pwm_1ms=0;
    pwm_10ms=0;
    tmr3_seconds_counter = TMR3_INTERRUPT_TICKER_FACTOR;
}

void TMR3_StartTimer(void)
{
    // Start the Timer by writing to TMRxON bit
    T3CONbits.TMR3ON = 1;
}

void TMR3_StopTimer(void)
{
    // Stop the Timer by writing to TMRxON bit
    T3CONbits.TMR3ON = 0;
}

uint16_t TMR3_ReadTimer(void)
{
    uint16_t readVal;
    uint8_t readValHigh;
    uint8_t readValLow;
    
    readValLow = TMR3L;
    readValHigh = TMR3H;
    
    readVal = ((uint16_t)readValHigh << 8) | readValLow;

    return readVal;
}

void TMR3_WriteTimer(uint16_t timerVal)
{
    if (T3CONbits.nT3SYNC == 1)
    {
        // Stop the Timer by writing to TMRxON bit
        T3CONbits.TMR3ON = 0;

        // Write to the Timer3 register
        TMR3H = (timerVal >> 8);
        TMR3L = (uint8_t) timerVal;

        // Start the Timer after writing to the register
        T3CONbits.TMR3ON =1;
    }
    else
    {
        // Write to the Timer3 register
        TMR3H = (timerVal >> 8);
        TMR3L = (uint8_t) timerVal;
    }
}

void TMR3_Reload(void)
{
    // Write to the Timer3 register
    TMR3H = (timer3ReloadVal >> 8);
    TMR3L = (uint8_t) timer3ReloadVal;
}

void TMR3_ISR(void)
{
    static volatile uint16_t CountCallBack = 0;

    // Clear the TMR3 interrupt flag
    PIR2bits.TMR3IF = 0;
    
     // Write to the Timer3 register
     TMR3H = (timer3ReloadVal >> 8);
     TMR3L =  (uint8_t) timer3ReloadVal;

    // callback function - called every 10th pass
    if (++CountCallBack >= TMR3_INTERRUPT_TICKER_FACTOR)
    {
        // ticker function call
        TMR3_CallBack();

        // reset ticker counter
        CountCallBack = 0;
    }
}

void TMR3_CallBack(void)
{
    // Add your custom callback code here

    if(TMR3_InterruptHandler)
    {
        TMR3_InterruptHandler();
    }
}

void TMR3_SetInterruptHandler(void (* InterruptHandler)(void)){
    TMR3_InterruptHandler = InterruptHandler;
}

void TMR3_DefaultInterruptHandler(void){
    // add your TMR3 interrupt custom code
    // or set custom function using TMR3_SetInterruptHandler()
    static uint8_t Countms=0;
    

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
            tmr3_seconds_counter = TMR3_INTERRUPT_TICKER_FACTOR; 
        }
        if(Countms==10)
        {
            pot_10ms=1;
            pwm_10ms=1;
            Countms=0;
        }
}

/**
 End of File
*/
