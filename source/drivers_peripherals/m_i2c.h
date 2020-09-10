#ifndef _M_I2C_H
#define _M_I2C_H

#include <xc.h>
#include <stdint.h>

#define i2cDelay(d)   __delay_us(d)

// This would have to be changed for each different micro due to different
// i2c pins
//#define SCL_TRIS               TRISFbits.TRISF6
//#define SCL_LAT                LATFbits.LATF6
//#define SCL_PORT               PORTFbits.RF6
//#define SDA_TRIS               TRISFbits.TRISF7
//#define SDA_LAT                LATFbits.LATF7
//#define SDA_PORT               PORTFbits.RF7


// Regular use functions
void i2cInit();
uint8_t i2cWrite(uint8_t Address,uint8_t *Data,uint8_t Num);
uint8_t i2cRead(uint8_t Address,uint8_t *Data,uint8_t Num);

// Low Level Functions
uint8_t i2cWriteByte(uint8_t Byte);
uint8_t i2cWriteBytes(uint8_t len, const void *Byte);
uint8_t i2cReadByte(uint8_t *Byte, uint8_t AckNack);
uint8_t i2cReadBytes(uint8_t len, uint8_t *Byte);

uint8_t i2cReadBit(void);
uint8_t i2cSendStart(void);
uint8_t i2cSendStop(void);
void    i2cClock(void);
uint8_t i2cGetAck(void);
uint8_t i2cSendAck(void);
uint8_t i2cSendNack(void);


#endif

