/**
 LED driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    led_board.c

  Summary:
    LED driver implementation.

  Description:
    This file provides the LED driver implementation.

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
#include "hardware.h"
#include "led_board.h"

led_t led_status;


void led_init (void)
{
    #ifdef __XC8
    LED_GREEN_TRIS = 0; // Green LED on new CRD Board
    LED_GREEN_LAT  = 0;
    
    LED_RED_TRIS = 0; // Red LED on new CRD Board
    LED_RED_LAT  = 0;
    
    POE_STAY_ALIVE_TRIS = 0; // PoE Stay Alive
    #endif
}

void led_update(void)
{
    #ifdef __XC8
    if(led_status.DBUpdate==1)
    {
        LED_RED_LAT ^= 1; 
    }
    
    else if(led_status.DHCP_INDICATOR == 1)
    {
        LED_GREEN_LAT = 1; 
        LED_RED_LAT = 0; 
    }
    else
    {        
        LED_GREEN_LAT ^= 1;
        LED_RED_LAT = 0; 
    }
    if(led_status.LLDP_INDICATOR == 1)
    {
        LED_RED_LAT = 1;    
    }
    else if(led_status.DHCP_INDICATOR==1)
    {
        LED_RED_LAT ^= 1;  
        
    }
    
    #endif
}
