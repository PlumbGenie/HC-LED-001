#ifndef __HARDWARE_UNIX__
#define __HARDWARE_UNIX__

#include <unistd.h>
#include <stdbool.h>

#define _XTAL_FREQ         
       
#define LED_GREEN_LAT      
#define LED_GREEN_TRIS     
#define LED_RED_LAT        
#define LED_RED_TRIS       

#define SWITCH_S1_LAT      
#define SWITCH_S1_TRIS     
#define SWITCH_S1_STATE    

#define POE_STAY_ALIVE_LAT 
#define POE_STAY_ALIVE_TRIS

#define RTCC_TICK          
#define RTCC_CS_TRIS       
#define CS_RTCC_HIGH()     
#define CS_RTCC_LOW()      

#define ASSERT_STORAGE_CE()
#define DEASSERT_STORAGE_CE()
#define STORAGE_CE_TRIS    
#define STORAGE_SCK1_TRIS  
#define STORAGE_SDI1_TRIS  
#define STORAGE_SDO1_TRIS  
#define PWM1_PORT_LAT      
#define PWM1_PORT_TRIS     

#ifdef IF_GEST
#define GEST_RESET_LAT     
#define GEST_EIO_LAT       
#define GEST_RESET_TRIS    
#define GEST_EIO_TRIS      
#define GEST_EIO_PORT      
#endif

#ifdef IF_TOGGLE
#define TOGGLE_TRIS        
#define TOGGLE_STATE       
#endif

#define ASSERT_MAC_ADDRESS_EEPROM_CS()
#define DEASSERT_MAC_ADDRESS_EEPROM_CS()

#define enableToggle()

#define NO_OPERATION() do{ usleep(1); } while(0)
#define BIT bool

#define DELAY_MS(a) do{ usleep(a*1000); } while(0)
#define DELAY_US(a) do{ usleep(a); } while(0)

extern bool INT1E;
extern bool GIE;
extern bool TMR1IE;
extern bool TMR1IF;
extern bool TMR1ON;
extern bool TMR1;

typedef struct {
    int LATA0:1;
    int LATA1:1;
    int LATA2:1;
    int LATA3:1;
    int LATA4:1;
    int LATA5:1;
    int LATA6:1;
    int LATA7:1;
} latabits_t;

typedef struct {
    int LATB0:1;
    int LATB1:1;
    int LATB2:1;
    int LATB3:1;
    int LATB4:1;
    int LATB5:1;
    int LATB6:1;
    int LATB7:1;
} latbbits_t;

typedef struct {
    int LATC0:1;
    int LATC1:1;
    int LATC2:1;
    int LATC3:1;
    int LATC4:1;
    int LATC5:1;
    int LATC6:1;
    int LATC7:1;
} latcbits_t;

typedef struct {
    int LATD0:1;
    int LATD1:1;
    int LATD2:1;
    int LATD3:1;
    int LATD4:1;
    int LATD5:1;
    int LATD6:1;
    int LATD7:1;
} latdbits_t;

typedef struct {
    int LATE0:1;
    int LATE1:1;
    int LATE2:1;
    int LATE3:1;
    int LATE4:1;
    int LATE5:1;
    int LATE6:1;
    int LATE7:1;
} latebits_t;

typedef struct {
    int LATF0:1;
    int LATF1:1;
    int LATF2:1;
    int LATF3:1;
    int LATF4:1;
    int LATF5:1;
    int LATF6:1;
    int LATF7:1;
} latfbits_t;

typedef struct {
    int RA0:1;
    int RA1:1;
    int RA2:1;
    int RA3:1;
    int RA4:1;
    int RA5:1;
    int RA6:1;
    int RA7:1;
} portabits_t;

typedef struct {
    int RB0:1;
    int RB1:1;
    int RB2:1;
    int RB3:1;
    int RB4:1;
    int RB5:1;
    int RB6:1;
    int RB7:1;
} portbbits_t;

typedef struct {
    int RC0:1;
    int RC1:1;
    int RC2:1;
    int RC3:1;
    int RC4:1;
    int RC5:1;
    int RC6:1;
    int RC7:1;
} portcbits_t;

typedef struct {
    int RD0:1;
    int RD1:1;
    int RD2:1;
    int RD3:1;
    int RD4:1;
    int RD5:1;
    int RD6:1;
    int RD7:1;
} portdbits_t;

typedef struct {
    int RE0:1;
    int RE1:1;
    int RE2:1;
    int RE3:1;
    int RE4:1;
    int RE5:1;
    int RE6:1;
    int RE7:1;
} portebits_t;

typedef struct {
    int RF0:1;
    int RF1:1;
    int RF2:1;
    int RF3:1;
    int RF4:1;
    int RF5:1;
    int RF6:1;
    int RF7:1;
} portfbits_t;

typedef struct {
    int TRISA0:1;
    int TRISA1:1;
    int TRISA2:1;
    int TRISA3:1;
    int TRISA4:1;
    int TRISA5:1;
    int TRISA6:1;
    int TRISA7:1;
} trisabits_t;

typedef struct {
    int TRISB0:1;
    int TRISB1:1;
    int TRISB2:1;
    int TRISB3:1;
    int TRISB4:1;
    int TRISB5:1;
    int TRISB6:1;
    int TRISB7:1;
} trisbbits_t;

typedef struct {
    int TRISC0:1;
    int TRISC1:1;
    int TRISC2:1;
    int TRISC3:1;
    int TRISC4:1;
    int TRISC5:1;
    int TRISC6:1;
    int TRISC7:1;
} triscbits_t;

typedef struct {
    int TRISD0:1;
    int TRISD1:1;
    int TRISD2:1;
    int TRISD3:1;
    int TRISD4:1;
    int TRISD5:1;
    int TRISD6:1;
    int TRISD7:1;
} trisdbits_t;

typedef struct {
    int TRISE0:1;
    int TRISE1:1;
    int TRISE2:1;
    int TRISE3:1;
    int TRISE4:1;
    int TRISE5:1;
    int TRISE6:1;
    int TRISE7:1;
} trisebits_t;

typedef struct {
    int TRISF0:1;
    int TRISF1:1;
    int TRISF2:1;
    int TRISF3:1;
    int TRISF4:1;
    int TRISF5:1;
    int TRISF6:1;
    int TRISF7:1;
} trisfbits_t;

extern latabits_t LATAbits;
extern latbbits_t LATBbits;
extern latcbits_t LATCbits;
extern latdbits_t LATDbits;
extern latebits_t LATEbits;
extern latfbits_t LATFbits;

extern portabits_t PORTAbits;
extern portbbits_t PORTBbits;
extern portcbits_t PORTCbits;
extern portdbits_t PORTDbits;
extern portebits_t PORTEbits;
extern portfbits_t PORTFbits;

extern trisabits_t TRISAbits;
extern trisbbits_t TRISBbits;
extern triscbits_t TRISCbits;
extern trisdbits_t TRISDbits;
extern trisebits_t TRISEbits;
extern trisfbits_t TRISFbits;

#endif
