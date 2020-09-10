/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    file_template_source.c

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

 */

/*

?  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "rtcc.h"

#include "hardware.h"

/******************************************************************************/
volatile time_t deviceTime;
volatile bool dirtyTime;

volatile uint16_t seconds_counter;
volatile uint8_t PoESACount;
/******************************************************************************/

void rtcc_init(void)
{
    deviceTime = 1293861600;
    seconds_counter = TMR1_TICKS_PER_SECOND;
    TMR1_SetInterruptHandler(rtcc_handler);
}

void rtcc_set(time_t t)
{
    deviceTime = t;
}

time_t rtcc_get(void)
{
    return deviceTime;
}

bool rtcc_isDirty(void)
{
    return dirtyTime;
}

time_t time(time_t *t)
{
    bool   gie_val;
    time_t  the_time;
    
    gie_val = (bool)GIE;  //jira: CAE_MCU8-5647
    GIE = 0;
    the_time = deviceTime;
    GIE = gie_val;

    if(t)
    {
        *t = the_time;
    }

    return (the_time);
}



/********************** RTCC Interrupt Handler************************/
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

void rtcc_handler(void)
{
    seconds_counter--;
    if(seconds_counter == 0)
    {
        seconds_counter = TMR1_TICKS_PER_SECOND;
        deviceTime++;
        led_update();
    }
    maintain_PoESA();
}
  
