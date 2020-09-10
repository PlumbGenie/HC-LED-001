/**
  Current and voltage sensing API Header File

  Company:
    Microchip Technology Inc.

  File Name:
    curr_volt_sense.h

  Summary:
    Header file for the current and voltage sensing driver.

  Description:
    This header file provides APIs for the current and voltage sensing for power measurement.

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
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef POWER_CAP_H
#define	POWER_CAP_H

#include <stdint.h>
#include "adc.h"

#define POWER_LIMIT                 48   // Power should be limited to 48 W
#define ADC_MULTIPLYING_FACTOR      32 //Reading in mV (3.3V/1023 = 3.22 mV) since it is a 10 bit ADC (Multiply by 10 to avoid floating point numbers) 
#define V48_VOLTAGE_DIVIDER_FACTOR  500 // Multiply by 20 and divide by 1000 to avoid using floating point numbers (20/1000 = 1/50) (Further divide by 10 to remove the multiplying factor of 10 due to ADC_MULTIPLYING_FACTOR)
#define I48_CURRENT_DIVIDING_FACTOR 20 // Divide by 2 to get the current value (since ADC gives output in voltage) (Further divide by 10 to remove the multiplying factor of 10 due to ADC_MULTIPLYING_FACTOR) The current value is in mA
#define POWER_LLDP_FACTOR           100 // Divide by 1000 to convert current from mA to A and multiply by 10 to get power as required for LLDP






// I_48 and V_48 ADC channels
#define I48_ADC_CHANNEL channel_AN2
#define V48_ADC_CHANNEL channel_AN3




uint16_t powerDrawn(void); // Calculated power

#endif	/* POWER_CAP_H */

