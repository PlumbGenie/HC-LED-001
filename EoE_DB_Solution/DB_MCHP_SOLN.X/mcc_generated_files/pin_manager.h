/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using MPLAB(c) Code Configurator

  @Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.26
        Device            :  PIC18F67J60
        Version           :  1.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

    Microchip licenses to you the right to use, modify, copy and distribute
    Software only when embedded on a Microchip microcontroller or digital signal
    controller that is integrated into your product or third party product
    (pursuant to the sublicense terms in the accompanying license agreement).

    You should refer to the license agreement accompanying this Software for
    additional information regarding your rights and obligations.

    SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
    MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
    IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
    CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
    OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
    CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
    SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

*/


#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set channel_AN2 aliases
#define channel_AN2_TRIS               TRISAbits.TRISA2
#define channel_AN2_LAT                LATAbits.LATA2
#define channel_AN2_PORT               PORTAbits.RA2
#define channel_AN2_ANS                anselRA2bits.anselRA2
#define channel_AN2_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define channel_AN2_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define channel_AN2_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define channel_AN2_GetValue()           PORTAbits.RA2
#define channel_AN2_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define channel_AN2_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define channel_AN2_SetAnalogMode()  do { anselRA2bits.anselRA2 = 1; } while(0)
#define channel_AN2_SetDigitalMode() do { anselRA2bits.anselRA2 = 0; } while(0)

// get/set channel_AN3 aliases
#define channel_AN3_TRIS               TRISAbits.TRISA3
#define channel_AN3_LAT                LATAbits.LATA3
#define channel_AN3_PORT               PORTAbits.RA3
#define channel_AN3_ANS                anselRA3bits.anselRA3
#define channel_AN3_SetHigh()            do { LATAbits.LATA3 = 1; } while(0)
#define channel_AN3_SetLow()             do { LATAbits.LATA3 = 0; } while(0)
#define channel_AN3_Toggle()             do { LATAbits.LATA3 = ~LATAbits.LATA3; } while(0)
#define channel_AN3_GetValue()           PORTAbits.RA3
#define channel_AN3_SetDigitalInput()    do { TRISAbits.TRISA3 = 1; } while(0)
#define channel_AN3_SetDigitalOutput()   do { TRISAbits.TRISA3 = 0; } while(0)
#define channel_AN3_SetAnalogMode()  do { anselRA3bits.anselRA3 = 1; } while(0)
#define channel_AN3_SetDigitalMode() do { anselRA3bits.anselRA3 = 0; } while(0)

// get/set channel_AN4 aliases
#define channel_AN4_TRIS               TRISAbits.TRISA4
#define channel_AN4_LAT                LATAbits.LATA4
#define channel_AN4_PORT               PORTAbits.RA4
#define channel_AN4_ANS                anselRA3bits.anselRA4
#define channel_AN4_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define channel_AN4_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define channel_AN4_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define channel_AN4_GetValue()           PORTAbits.RA4
#define channel_AN4_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define channel_AN4_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define channel_AN4_SetAnalogMode()  do { anselRA4bits.anselRA4 = 1; } while(0)
#define channel_AN4_SetDigitalMode() do { anselRA4bits.anselRA4 = 0; } while(0)
// get/set SWITCH_S1 aliases
#define SWITCH_S1_TRIS               TRISBbits.TRISB0
#define SWITCH_S1_LAT                LATBbits.LATB0
#define SWITCH_S1_PORT               PORTBbits.RB0
#define SWITCH_S1_SetHigh()            do { LATBbits.LATB0 = 1; } while(0)
#define SWITCH_S1_SetLow()             do { LATBbits.LATB0 = 0; } while(0)
#define SWITCH_S1_Toggle()             do { LATBbits.LATB0 = ~LATBbits.LATB0; } while(0)
#define SWITCH_S1_GetValue()           PORTBbits.RB0
#define SWITCH_S1_SetDigitalInput()    do { TRISBbits.TRISB0 = 1; } while(0)
#define SWITCH_S1_SetDigitalOutput()   do { TRISBbits.TRISB0 = 0; } while(0)

// get/set RTCC_CS aliases
#define RTCC_CS_TRIS               TRISBbits.TRISB4
#define RTCC_CS_LAT                LATBbits.LATB4
#define RTCC_CS_PORT               PORTBbits.RB4
#define RTCC_CS_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define RTCC_CS_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define RTCC_CS_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define RTCC_CS_GetValue()           PORTBbits.RB4
#define RTCC_CS_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define RTCC_CS_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)

// get/set STORAGE_CE aliases
#define STORAGE_CE_TRIS               TRISBbits.TRISB5
#define STORAGE_CE_LAT                LATBbits.LATB5
#define STORAGE_CE_PORT               PORTBbits.RB5
#define STORAGE_CE_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define STORAGE_CE_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define STORAGE_CE_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define STORAGE_CE_GetValue()           PORTBbits.RB5
#define STORAGE_CE_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define STORAGE_CE_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)

// get/set PWM1_PORT aliases
#define PWM1_PORT_TRIS               TRISCbits.TRISC2
#define PWM1_PORT_LAT                LATCbits.LATC2
#define PWM1_PORT_PORT               PORTCbits.RC2
#define PWM1_PORT_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define PWM1_PORT_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define PWM1_PORT_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define PWM1_PORT_GetValue()           PORTCbits.RC2
#define PWM1_PORT_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define PWM1_PORT_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)

// get/set STORAGE_SDI1 aliases
#define STORAGE_SDI1_TRIS               TRISCbits.TRISC4
#define STORAGE_SDI1_LAT                LATCbits.LATC4
#define STORAGE_SDI1_PORT               PORTCbits.RC4
#define STORAGE_SDI1_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define STORAGE_SDI1_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define STORAGE_SDI1_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define STORAGE_SDI1_GetValue()           PORTCbits.RC4
#define STORAGE_SDI1_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define STORAGE_SDI1_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)

// get/set STORAGE_SDO1 aliases
#define STORAGE_SDO1_TRIS               TRISCbits.TRISC5
#define STORAGE_SDO1_LAT                LATCbits.LATC5
#define STORAGE_SDO1_PORT               PORTCbits.RC5
#define STORAGE_SDO1_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define STORAGE_SDO1_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define STORAGE_SDO1_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define STORAGE_SDO1_GetValue()           PORTCbits.RC5
#define STORAGE_SDO1_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define STORAGE_SDO1_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)

// get/set RC6 procedures
#define RC6_SetHigh()    do { LATCbits.LATC6 = 1; } while(0)
#define RC6_SetLow()   do { LATCbits.LATC6 = 0; } while(0)
#define RC6_Toggle()   do { LATCbits.LATC6 = ~LATCbits.LATC6; } while(0)
#define RC6_GetValue()         PORTCbits.RC6
#define RC6_SetDigitalInput()   do { TRISCbits.TRISC6 = 1; } while(0)
#define RC6_SetDigitalOutput()  do { TRISCbits.TRISC6 = 0; } while(0)

// get/set RC7 procedures
#define RC7_SetHigh()    do { LATCbits.LATC7 = 1; } while(0)
#define RC7_SetLow()   do { LATCbits.LATC7 = 0; } while(0)
#define RC7_Toggle()   do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)
#define RC7_GetValue()         PORTCbits.RC7
#define RC7_SetDigitalInput()   do { TRISCbits.TRISC7 = 1; } while(0)
#define RC7_SetDigitalOutput()  do { TRISCbits.TRISC7 = 0; } while(0)

// get/set GEST_CS aliases
#define GEST_CS_TRIS               TRISDbits.TRISD1
#define GEST_CS_LAT                LATDbits.LATD1
#define GEST_CS_PORT               PORTDbits.RD1
#define GEST_CS_SetHigh()            do { LATDbits.LATD1 = 1; } while(0)
#define GEST_CS_SetLow()             do { LATDbits.LATD1 = 0; } while(0)
#define GEST_CS_Toggle()             do { LATDbits.LATD1 = ~LATDbits.LATD1; } while(0)
#define GEST_CS_GetValue()           PORTDbits.RD1
#define GEST_CS_SetDigitalInput()    do { TRISDbits.TRISD1 = 1; } while(0)
#define GEST_CS_SetDigitalOutput()   do { TRISDbits.TRISD1 = 0; } while(0)

// get/set POE_AT_FLAG aliases
#define POE_AT_FLAG_TRIS               TRISEbits.TRISE2
#define POE_AT_FLAG_LAT                LATEbits.LATE2
#define POE_AT_FLAG_PORT               PORTEbits.RE2
#define POE_AT_FLAG_SetHigh()            do { LATEbits.LATE2 = 1; } while(0)
#define POE_AT_FLAG_SetLow()             do { LATEbits.LATE2 = 0; } while(0)
#define POE_AT_FLAG_Toggle()             do { LATEbits.LATE2 = ~LATEbits.LATE2; } while(0)
#define POE_AT_FLAG_GetValue()           PORTEbits.RE2
#define POE_AT_FLAG_SetDigitalInput()    do { TRISEbits.TRISE2 = 1; } while(0)
#define POE_AT_FLAG_SetDigitalOutput()   do { TRISEbits.TRISE2 = 0; } while(0)

// get/set LED_GREEN aliases
#define LED_GREEN_TRIS               TRISEbits.TRISE4
#define LED_GREEN_LAT                LATEbits.LATE4
#define LED_GREEN_PORT               PORTEbits.RE4
#define LED_GREEN_SetHigh()            do { LATEbits.LATE4 = 1; } while(0)
#define LED_GREEN_SetLow()             do { LATEbits.LATE4 = 0; } while(0)
#define LED_GREEN_Toggle()             do { LATEbits.LATE4 = ~LATEbits.LATE4; } while(0)
#define LED_GREEN_GetValue()           PORTEbits.RE4
#define LED_GREEN_SetDigitalInput()    do { TRISEbits.TRISE4 = 1; } while(0)
#define LED_GREEN_SetDigitalOutput()   do { TRISEbits.TRISE4 = 0; } while(0)

// get/set LED_RED aliases
#define LED_RED_TRIS               TRISEbits.TRISE5
#define LED_RED_LAT                LATEbits.LATE5
#define LED_RED_PORT               PORTEbits.RE5
#define LED_RED_SetHigh()            do { LATEbits.LATE5 = 1; } while(0)
#define LED_RED_SetLow()             do { LATEbits.LATE5 = 0; } while(0)
#define LED_RED_Toggle()             do { LATEbits.LATE5 = ~LATEbits.LATE5; } while(0)
#define LED_RED_GetValue()           PORTEbits.RE5
#define LED_RED_SetDigitalInput()    do { TRISEbits.TRISE5 = 1; } while(0)
#define LED_RED_SetDigitalOutput()   do { TRISEbits.TRISE5 = 0; } while(0)

// get/set POE_STAY_ALIVE aliases
#define POE_STAY_ALIVE_TRIS               TRISFbits.TRISF2
#define POE_STAY_ALIVE_LAT                LATFbits.LATF2
#define POE_STAY_ALIVE_PORT               PORTFbits.RF2
#define POE_STAY_ALIVE_ANS                anselRF2bits.anselRF2
#define POE_STAY_ALIVE_SetHigh()            do { LATFbits.LATF2 = 1; } while(0)
#define POE_STAY_ALIVE_SetLow()             do { LATFbits.LATF2 = 0; } while(0)
#define POE_STAY_ALIVE_Toggle()             do { LATFbits.LATF2 = ~LATFbits.LATF2; } while(0)
#define POE_STAY_ALIVE_GetValue()           PORTFbits.RF2
#define POE_STAY_ALIVE_SetDigitalInput()    do { TRISFbits.TRISF2 = 1; } while(0)
#define POE_STAY_ALIVE_SetDigitalOutput()   do { TRISFbits.TRISF2 = 0; } while(0)
#define POE_STAY_ALIVE_SetAnalogMode()  do { anselRF2bits.anselRF2 = 1; } while(0)
#define POE_STAY_ALIVE_SetDigitalMode() do { anselRF2bits.anselRF2 = 0; } while(0)

// get/set IO_RD0 aliases
#define IO_RD0_TRIS                 TRISDbits.TRISD0
#define IO_RD0_LAT                  LATDbits.LATD0
#define IO_RD0_PORT                 PORTDbits.RD0
#define IO_RD0_SetHigh()            do { LATDbits.LATD0 = 1; } while(0)
#define IO_RD0_SetLow()             do { LATDbits.LATD0 = 0; } while(0)
#define IO_RD0_Toggle()             do { LATDbits.LATD0 = ~LATDbits.LATD0; } while(0)
#define IO_RD0_GetValue()           PORTDbits.RD0
#define IO_RD0_SetDigitalInput()    do { TRISDbits.TRISD0 = 1; } while(0)
#define IO_RD0_SetDigitalOutput()   do { TRISDbits.TRISD0 = 0; } while(0)

// get/set IO_RD2 aliases
#define IO_RD2_TRIS                 TRISDbits.TRISD2
#define IO_RD2_LAT                  LATDbits.LATD2
#define IO_RD2_PORT                 PORTDbits.RD2
#define IO_RD2_SetHigh()            do { LATDbits.LATD2 = 1; } while(0)
#define IO_RD2_SetLow()             do { LATDbits.LATD2 = 0; } while(0)
#define IO_RD2_Toggle()             do { LATDbits.LATD2 = ~LATDbits.LATD2; } while(0)
#define IO_RD2_GetValue()           PORTDbits.RD2
#define IO_RD2_SetDigitalInput()    do { TRISDbits.TRISD2 = 1; } while(0)
#define IO_RD2_SetDigitalOutput()   do { TRISDbits.TRISD2 = 0; } while(0)


/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    //PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);


//Manually add for software I2C
#ifdef IF_IO_STARTER_EXPANSION
//I/O Starter Expansion Board I2C Configuration

#define SDA_TRIS                TRISFbits.TRISF7
#define SCL_TRIS                TRISFbits.TRISF6

#define SDA_PORT                PORTFbits.RF7
#define SCL_PORT                PORTFbits.RF6

#define SDA_LAT                 LATFbits.LATF7
#define SCL_LAT                 LATFbits.LATF6

#elif IF_TOGGLE
#define TOGGLE_TRIS         TRISDbits.TRISD0      //AN3
#define TOGGLE_STATE        PORTDbits.RD0         //AN3
#endif
#endif // PIN_MANAGER_H
/**
 End of File
*/