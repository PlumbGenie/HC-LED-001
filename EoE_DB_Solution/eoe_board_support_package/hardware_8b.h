#ifndef __8BIT_HARDWARE_DEFINITION
#define __8BIT_HARDWARE_DEFINITION

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define _XTAL_FREQ              41666667UL    //41.6667MHz
       
#define LED_GREEN_LAT           LATEbits.LATE4 
#define LED_GREEN_TRIS          TRISEbits.TRISE4
#define LED_RED_LAT             LATEbits.LATE5     
#define LED_RED_TRIS            TRISEbits.TRISE5   

#define SWITCH_S1_LAT           LATBbits.LATB0
#define SWITCH_S1_TRIS          TRISBbits.TRISB0 
#define SWITCH_S1_STATE         PORTBbits.RB0

#define POE_STAY_ALIVE_LAT      LATFbits.LATF2
#define POE_STAY_ALIVE_TRIS     TRISFbits.TRISF2

#define POE_AT_FLAG_LAT         LATEbits.LATE2
#define POE_AT_FLAG_PORT        PORTEbits.RE2
#define POE_AT_FLAG_TRIS        TRISEbits.TRISE2

#define RTCC_TICK               PORTBbits.RB1
#define RTCC_CS_TRIS            TRISBbits.TRISB4 
#define CS_RTCC_HIGH()          do{LATBbits.LATB4 = 1;} while(0)
#define CS_RTCC_LOW()           do{LATBbits.LATB4 = 0;} while(0)

#define STORAGE_CE              LATBbits.LATB5
#define STORAGE_CE_TRIS         TRISBbits.TRISB5
#define STORAGE_SCK1_TRIS       TRISCbits.TRISC3
#define STORAGE_SDI1_TRIS       TRISCbits.TRISC4
#define STORAGE_SDO1_TRIS       TRISCbits.TRISC5
#define ASSERT_STORAGE_CE()     do{ LATBbits.LATB5 = 0; } while(0)
#define DEASSERT_STORAGE_CE()   do{ LATBbits.LATB5 = 1; } while(0)

#define PWM1_PORT_LAT           LATCbits.LATC2
#define PWM1_PORT_TRIS          TRISCbits.TRISC2


//ECC I2C Configuration 
//
//#define SDA_TRIS  TRISEbits.TRISE1
//#define SCL_TRIS  TRISEbits.TRISE0
//
//#define SDA_PORT   PORTEbits.RE1
//#define SCL_PORT   PORTEbits.RE0
//
//#define SDA_LAT     LATEbits.LATE1
//#define SCL_LAT     LATEbits.LATE0

#ifdef IF_IO_STARTER_EXPANSION

//I/O Starter Expansion Board I2C Configuration

#define SDA_TRIS                TRISFbits.TRISF7
#define SCL_TRIS                TRISFbits.TRISF6

#define SDA_PORT                PORTFbits.RF7
#define SCL_PORT                PORTFbits.RF6

#define SDA_LAT                 LATFbits.LATF7
#define SCL_LAT                 LATFbits.LATF6

#endif


#ifdef IF_GEST

//I2C Gest

//#define SDA_TRIS                TRISFbits.TRISF7
//#define SCL_TRIS                TRISFbits.TRISF6
//
//#define SDA_PORT                PORTFbits.RF7
//#define SCL_PORT                PORTFbits.RF6
//
//#define SDA_LAT                 LATFbits.LATF7
//#define SCL_LAT                 LATFbits.LATF6
//
//#define GEST_RESET_LAT          LATDbits.LATD0      //RST
//#define GEST_EIO_LAT            LATBbits.LATB2      //EIO
//#define GEST_RESET_TRIS         TRISDbits.TRISD0    //RST
//#define GEST_EIO_TRIS           TRISBbits.TRISB2    //EIO
//#define GEST_EIO_PORT           PORTBbits.RB2       //EIO

//SPI Gest
#define GEST_INT_TRIS           TRISBbits.TRISB2      
#define GEST_INT_PORT           PORTBbits.RB2
#define GEST_CS_TRIS           TRISDbits.TRISD1
#define GEST_CS_HIGH()         do{LATDbits.LATD1 = 1;} while(0)
#define GEST_CS_LOW()          do{LATDbits.LATD1 = 0;} while(0)
#endif


#ifdef IF_WEATHER
#define BME280_SCK1_TRIS    TRISCbits.TRISC3
#define BME280_SDI1_TRIS    TRISCbits.TRISC4
#define BME280_SDO1_TRIS    TRISCbits.TRISC5
#define BME280_CS_TRSIS     TRISDbits.TRISD1
#endif

#ifdef IF_MOTION
#define MOTION_PIR500B_INTERRUPT_TRIS     TRISBbits.TRISB2   
#define MOTION_PIR500B_INTERRUPT_LAT      LATBbits.LATB2   
#define MOTION_PIR500B_INTERRUPT_PORT     PORTBbits.RB2   

#define MOTION_PIR500B_ENABLE_TRIS     TRISDbits.TRISD0   
#define MOTION_PIR500B_ENABLE_LAT      LATDbits.LATD0   
#define MOTION_PIR500B_ENABLE_PORT     PORTDbits.RD0 
#endif

#ifdef IF_GEST
//DB - I2c Gest
//#define SDA_TRIS                TRISFbits.TRISF7
//#define SCL_TRIS                TRISFbits.TRISF6
//
//#define SDA_PORT                PORTFbits.RF7
//#define SCL_PORT                PORTFbits.RF6
//
//#define SDA_LAT                 LATFbits.LATF7
//#define SCL_LAT                 LATFbits.LATF6
//
//#define GEST_RESET_LAT          LATDbits.LATD0      //RST
//#define GEST_EIO_LAT            LATBbits.LATB2      //EIO
//#define GEST_RESET_TRIS         TRISDbits.TRISD0    //RST
//#define GEST_EIO_TRIS           TRISBbits.TRISB2    //EIO
//#define GEST_EIO_PORT           PORTBbits.RB2       //EIO
#endif

#ifdef IF_TOGGLE
#define TOGGLE_TRIS         TRISDbits.TRISD0      //AN3
#define TOGGLE_STATE        PORTDbits.RD0         //AN3
#endif


#define ASSERT_MAC_ADDRESS_EEPROM_CS()        do{ LATBbits.LATB4 = 0; } while(0)
#define DEASSERT_MAC_ADDRESS_EEPROM_CS()      do{ LATBbits.LATB4 = 1; } while(0)

#define enableToggle()      do{TRISDbits.TRISD1 = 1;}while(0)      //AN2

#define NO_OPERATION()     NOP()
#define DELAY_MS(a)        do{ unsigned int _d = a; while(_d--) { __delay_ms(1); } } while(0)
#define DELAY_US(a)        __delay_us(a)
#define BIT bit


extern bool PoE_TWO_EVENT_DETECTED_FLAG;

#endif
