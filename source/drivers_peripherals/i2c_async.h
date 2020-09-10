/**
  I2C Driver API Header File

  Company:
    Microchip Technology Inc.

  File Name:
    i2c_async.h

  Summary:
    Header file for the I2C Driver implementation.

  Description:
    This header file provides APIs for the I2C Driver implementation.

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


#ifndef I2C_ASYNC_H
#define	I2C_ASYNC_H

#include "i2c_types.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdlib.h>

typedef enum
{
    i2c_stop,
    i2c_restart,
    i2c_continue,
    i2c_resetLink
} i2c_operations_t;

typedef i2c_operations_t (*i2c_callback)(void *p);


// initialize the HW
void I2C1_Initialize(void);

// Non Blocking Interface to the I2c
bool i2c1_open(i2c_address_t address);
bool i2c1_close(i2c_address_t address);
bool i2c1_setWriteCompleteCallback(i2c_callback cb,void *p);
bool i2c1_setReadCompleteCallback(i2c_callback cb, void *p);
bool i2c1_setWriteCollisionCallback(i2c_callback cb, void *p);
bool i2c1_setAddressNACKCallback(i2c_callback cb, void *p);
bool i2c1_setDataNACKCallback(i2c_callback cb, void *p);
i2c_error_t i2c1_asyncMasterWrite(const void *dataToWrite, size_t writeSize);
i2c_error_t i2c1_asyncMasterRead(void *bufferForRead, size_t bufferSize);

// things that are callable from a callback
void i2c1_asyncMasterReWrite(const void *dataToWrite, size_t writeSize);
void i2c1_asyncMasterReRead(void *bufferForRead, size_t bufferSize);

// blocking interface to the I2C.
i2c_error_t i2c1_blockingMasterWrite(uint8_t address, const void *dataToWrite, size_t writeSize);
i2c_error_t i2c1_blockingMasterRead(uint8_t address, void *bufferForRead, size_t bufferSize);

void I2C1_ISR(void);

char I2C_Send(char Address,const void *Data,char Num);
char i2csendbyte(char byte);

uint8_t i2c_send_bytes(uint8_t count, const void *data);
uint8_t i2c_receive_bytes(uint8_t count, char *data);
char i2c_receive_byte(char *data);

char i2cgetack(void);
char i2csendack(void);
char i2csendnack(void);

#define i2cdelay(x) DELAY_US(x)
uint8_t i2c_send_start(void);
uint8_t i2c_send_stop(void);
void i2cclock(void);
char i2creadbit(void);

//void i2c_enable();
void i2c_set_speed(uint32_t speed);
void i2c_disable();


#define I2C_FUNCTION_RETCODE_SUCCESS     ((uint8_t) 0x00) //!< Communication with device succeeded.
#define I2C_FUNCTION_RETCODE_COMM_FAIL   ((uint8_t) 0xF0) //!< Communication with device failed.
#define I2C_FUNCTION_RETCODE_TIMEOUT     ((uint8_t) 0xF1) //!< Communication timed out.
#define I2C_FUNCTION_RETCODE_NACK        ((uint8_t) 0xF8) //!< TWI nack

//#define SDA_TRIS  TRISEbits.TRISE1
//#define SCL_TRIS  TRISEbits.TRISE0
//
//#define SDA_PORT   PORTEbits.RE1
//#define SCL_PORT   PORTEbits.RE0
//
//#define SDA_LAT     LATEbits.LATE1
//#define SCL_LAT     LATEbits.LATE0

#endif	/* I2C_ASYNC_H */

