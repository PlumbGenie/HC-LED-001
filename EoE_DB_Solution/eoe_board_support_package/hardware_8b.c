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

#include <xc.h>
#include <stdio.h>
#include "uart.h"
#include "mssp_spi1_master.h"
#include "tmr1.h"
#include "eeprom.h"
#include "hardware.h"
#include "led_board.h"
#include "mac_eeprom.h"
#include "rtcc.h"
#ifndef IF_BOOTLOADER
// #include "rtcc.h"
#include "adc.h"
#include "eeprom.h"
#include "pwm.h"
#include "i2c_async.h" 
#include "tmr3.h"
#include "m_i2c.h"
#include "at30ts74_temperature_sensor.h"
#endif


#ifndef __XC8
bool INT1E = 0;
bool GIE = 0;
bool TMR1IE = 0;
bool TMR1IF = 0;
bool TMR1ON = 0;
bool TMR1 = 0;

struct {
    int LATA0:1;
    int LATA1:1;
    int LATA2:1;
    int LATA3:1;
    int LATA4:1;
    int LATA5:1;
    int LATA6:1;
    int LATA7:1;
} LATAbits;

struct {
    int LATB0:1;
    int LATB1:1;
    int LATB2:1;
    int LATB3:1;
    int LATB4:1;
    int LATB5:1;
    int LATB6:1;
    int LATB7:1;
} LATBbits;

struct {
    int LATE0:1;
    int LATE1:1;
    int LATE2:1;
    int LATE3:1;
    int LATE4:1;
    int LATE5:1;
    int LATE6:1;
    int LATE7:1;
} LATEbits;


#endif

//void delay_ms(uint8_t ms)
//{
//    volatile uint16_t us;
//    
//    us = 0;
//    us = ms *1000;
//    while(us)
//    {
//        __delay_us(100);
//        us = us-100;
//    }
//}


bool PoE_TWO_EVENT_DETECTED_FLAG;


void initHW(void)
{
#ifdef __XC8

    // Oscillator
    OSCCON = 0x02; // primary oscillator    
    // Enable 41.6667MHz with PLL on PIC18F97J60
    OSCTUNE = 0x40;

//   // Floating
//    TRISJ = 0; // PORTJ is all the LED's
//    LATJ = 0;

    //PORT pins setup
    TRISCbits.TRISC7 = 1; // RC7 is input
    TRISCbits.TRISC6 = 0; // RC6 is output    
    
    // SSP/EEPROM/RTC Initializations
    STORAGE_CE_TRIS = 0;                 //AT25M02 storageOM1 CE o/p
    STORAGE_CE_TRIS = 0;                 //AT25M02 storageOM2 CE o/p
    RTCC_CS_TRIS = 0;                    //RTCC CS     - o/p
    STORAGE_SCK1_TRIS = 0;               //storageOM SCK1 - o/p   
    STORAGE_SDI1_TRIS = 1;               //storageOM SDI1 - i/p    
    STORAGE_SDO1_TRIS = 0;               //storageOM SDO1 - o/p  
    
    
    POE_AT_FLAG_TRIS = 1;               // AT_FLAG - i/p
    
    if(POE_AT_FLAG_PORT ==1)
    {
        PoE_TWO_EVENT_DETECTED_FLAG = false;
    }
    else if(POE_AT_FLAG_PORT ==0)
    {
        PoE_TWO_EVENT_DETECTED_FLAG = true;
    
    }
    else 
    {
        PoE_TWO_EVENT_DETECTED_FLAG = false;
    }
    
    
    
    //TICK
    TRISBbits.TRISB1 = 1;               //RTCC Tick   - i/p
    
    #ifdef IF_GEST
//        GEST_RESET_TRIS = 0;   //output
    #endif
    
    #ifdef IF_TOGGLE
        TOGGLE_TRIS = 1;   //Input
    #endif
    
    // set the ADC1 to the options selected in the User Interface
    TRISAbits.RA2 = 1;
    TRISAbits.RA3 = 1;
    
    led_init();
    // LED's
    TRISA = 0b11111100;// RA0, RA1 are Ethernet LED's

    // Enable internal PORTB pull-ups
    RBPU = 0;
    // Switch S1 on CRD Board
    TRISBbits.TRISB0  = 1; // Switch S1 i/p on CRD Board 
    INTCON2bits.nRBPU = 0;
    
    //SPI
    spi1_init();
    // UART to Debug
    uart_init();
    
    
    #ifndef IF_BOOTLOADER
        // Timers
        // timer 1 configured in RTCC.C        
        tmr1_init();
        rtcc_init();
        // Timer 3
        tmr3_init();
        //ADC
        ADC_Initialize();
        
        #ifdef IF_LIGHT
            pwm_init();
        #endif
      
        #ifndef IF_TOGGLE
            //Database EEPROM
            storageInit();
            //MAC EEPROM
            macEeprInit();
        #endif
        
        #ifdef IF_GEST
            // I2C for GestIC
//            I2C1_Initialize();//db  i2c gest  
        #endif
#ifdef IF_IO_STARTER_EXPANSION
            i2cInit();
            AT30TS74_Initialize();
#endif
        
    #endif   
    
#endif
}
