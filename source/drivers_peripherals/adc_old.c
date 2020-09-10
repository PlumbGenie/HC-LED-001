/**
  ADC Driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    adc.c

  Summary:
    ADC driver implementation.

  Description:
    This file provides the ADC driver API implementation.

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


#include "adc.h"
#include "hardware.h"

/**
  Section: Macro Declarations
*/

#define ACQ_US_DELAY 5

/**
  Section: ADC Module APIs
*/

void ADC_Initialize(void)
{
   
    #ifdef __XC8
    // GO_nDONE stop; ADON enabled; CHS AN3;
    ADCON0 = 0x09;

    // ADPREF chip_VDD; ADNREF chip_VSS;PCG<3:0> 1011
    ADCON1 = 0x09;

    //ADFM right; ADCS FOSC/4;
    ADCON2 = 0xBE;

    // ADRESL 0x0;
    ADRESL = 0x00;

    // ADRESH 0x0;
    ADRESH = 0x00;
    #endif

}

void ADC_StartConversion(adc_channel_t channel)
{
    #ifdef __XC8
    // select the A/D channel
    ADCON0bits.CHS = channel;

    // Turn on the ADC module
    ADCON0bits.ADON = 1;

    // Acquisition time delay
    DELAY_US(ACQ_US_DELAY);

    // Start the conversion
    ADCON0bits.GO_nDONE = 1;
    #endif
}

bool ADC_IsConversionDone()
{
    #ifdef __XC8
    // Start the conversion
    return (!ADCON0bits.GO_nDONE);
    #else
        return 0;
    #endif
}

adc_result_t ADC_GetConversionResult(void)
{
    #ifdef __XC8
    // Conversion finished, return the result
    return ((ADRESH << 8) + ADRESL);
    #else
        return 0;
    #endif
}

adc_result_t ADC_GetConversion(adc_channel_t channel)
{
    #ifdef __XC8
   // Select the A/D channel
    ADCON0bits.CHS = channel;

    // Turn on the ADC module
    ADCON0bits.ADON = 1;

    // Acquisition time delay
    DELAY_US(ACQ_US_DELAY);

    // Start the conversion
    ADCON0bits.GO_nDONE = 1;

    // Wait for the conversion to finish
    while (ADCON0bits.GO_nDONE)
    {
    }

    // Conversion finished, return the result
    //return ((ADRESH << 8) + ADRESL);
    return (ADRES);
    #else
        return 0;
    #endif
}
/**
 End of File
*/
