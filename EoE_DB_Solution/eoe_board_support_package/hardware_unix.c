/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    hardware.c

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

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

#include "uart.h"
#include "mssp_spi1_master.h"
#include "tmr1.h"
#include "eeprom.h"
#include "hardware_unix.h"
#include "led_board.h"
#include "mac_eeprom.h"

#ifndef IF_BOOTLOADER
// #include "rtcc.h"
#include "adc.h"
#include "eeprom.h"
#include "pwm.h"
#include "i2c_async.h" 
#include "tmr3.h"
#endif

bool INT1E = 0;
bool GIE = 0;
bool TMR1IE = 0;
bool TMR1IF = 0;
bool TMR1ON = 0;
bool TMR1 = 0;

latabits_t LATAbits;
latbbits_t LATBbits;
latcbits_t LATCbits;
latdbits_t LATDbits;
latebits_t LATEbits;
latfbits_t LATFbits;

portabits_t PORTAbits;
portbbits_t PORTBbits;
portcbits_t PORTCbits;
portdbits_t PORTDbits;
portebits_t PORTEbits;
portfbits_t PORTFbits;

trisabits_t TRISAbits;
trisbbits_t TRISBbits;
triscbits_t TRISCbits;
trisdbits_t TRISDbits;
trisebits_t TRISEbits;
trisfbits_t TRISFbits;

void initHW(void)
{
    led_init();    
    spi1_init();
    uart_init();
    tmr1_init();
    tmr3_init();
    ADC_Initialize();        
    pwm_init();
    storageInit();
    macEeprInit();
    I2C1_Initialize();//db   
}
