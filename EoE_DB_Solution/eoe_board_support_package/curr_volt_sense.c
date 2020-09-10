/**
 Current and voltage sensing

  Company:
    Microchip Technology Inc.

  File Name:
    curr_volt_sense.c

  Summary:
    ADC driver implementation.

  Description:
    This file provides the current and voltage sensing implementation for power measurement.

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

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include "adc.h"
#include "curr_volt_sense.h"
//#include "actuator.h"

/****************************************Helper Functions*******************************/
static uint16_t readVoltage(adc_channel_t channel);
static uint16_t readCurrent(adc_channel_t channel);


static uint16_t readVoltage(adc_channel_t channel)
{
    adc_result_t voltageADCResult;
    voltageADCResult = (ADC_GetConversion(channel) * ADC_MULTIPLYING_FACTOR)/V48_VOLTAGE_DIVIDER_FACTOR; //Read specified ADC channel (Reading in mV 3.3/1023 since it is a 10 bit ADC) 
    return voltageADCResult;
}


static uint16_t readCurrent(adc_channel_t channel)
{
    adc_result_t currentADCResult;
    currentADCResult = (ADC_GetConversion(channel) * ADC_MULTIPLYING_FACTOR)/I48_CURRENT_DIVIDING_FACTOR; //Read specified ADC channel (Reading in mV 3.3/1023 since it is a 10 bit ADC)
    return currentADCResult;
}


uint16_t powerDrawn(void)
{
    uint16_t I_48Current, V_48Voltage;
    uint16_t powerVal = 0;
    
    I_48Current = readCurrent(I48_ADC_CHANNEL); // result is in mA
    V_48Voltage = readVoltage(V48_ADC_CHANNEL); // (multiply by 20, divide by 1000 to get the reading in V)

    powerVal = (V_48Voltage * I_48Current)/POWER_LLDP_FACTOR; //(Actually divide by 1000, divide by 100 is for LLDP)

//    updatepowerDrawn("actuator1", powerVal);
//    printf("power Drawn= %d\r\n",powerVal);
    return powerVal;
}
