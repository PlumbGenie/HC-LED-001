/**
 I2C driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    i2c_async.c

  Summary:
    I2C Driver implementation.

  Description:
    This file provides the I2C driver implementation.

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
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "i2c_async.h"
#include "i2c_types.h"


#define I2C_IDLE 0
#define I2C_WRITECOLLISION  1
#define I2C_SEND_ADDR 2
#define I2C_START_WRITING 3
#define I2C_START_RECEIVING 4
#define I2C_SEND_DATA 5
#define I2C_RCV_DATA 6
#define I2C_DATA_RCVD 7
#define MAX_STATE 7 // prevent out of bounds on the state variable

inline static uint8_t issueCallback(uint8_t idx,uint8_t thisState);

// The I2C state functions process an I2C state and return the next state to enter.
typedef uint8_t (*stateFunction)(void);
inline uint8_t i2c1_idleFunction(void);
inline uint8_t i2c1_writeCollisionFunction(void);
inline uint8_t i2c1_sendAddrFunction(void);
inline uint8_t i2c1_startWritingFunction(void);
inline uint8_t i2c1_startReceivingFunction(void);
inline uint8_t i2c1_sendDataFunction(void);
inline uint8_t i2c1_rcvDataFunction(void);
inline uint8_t i2c1_dataRcvdFunction(void);

const stateFunction i2c1_functionTable[] = {
    i2c1_idleFunction,
    i2c1_writeCollisionFunction,
    i2c1_sendAddrFunction,
    i2c1_startWritingFunction,
    i2c1_startReceivingFunction,
    i2c1_sendDataFunction,
    i2c1_rcvDataFunction,
    i2c1_dataRcvdFunction
};

// index for each callback in the callback list.
typedef enum
{
    i2c_writeComplete = 0,
    i2c_readComplete,
    i2c_writeCollision,
    i2c_addressNACK,
    i2c_dataNACK
} i2c_callbackIndex;

static bool setCallBack(i2c_callbackIndex idx, i2c_callback cb, void *p);
static i2c_operations_t asyncStop(void *p);

bool i2c1_setWriteCompleteCallback(i2c_callback cb, void *p)
{
    return setCallBack(i2c_writeComplete,cb,p);
}



bool i2c1_setReadCompleteCallback(i2c_callback cb, void *p)
{
    return setCallBack(i2c_readComplete,cb,p);
}

bool i2c1_setWriteCollisionCallback(i2c_callback cb, void *p)
{
    return setCallBack(i2c_writeCollision,cb,p);
}

bool i2c1_setAddressNACKCallback(i2c_callback cb, void *p)
{
    return setCallBack(i2c_addressNACK,cb,p);
}

bool i2c1_setDataNACKCallback(i2c_callback cb, void *p)
{
    return setCallBack(i2c_dataNACK,cb,p);
}

struct
{
    unsigned busy:1;
    unsigned writing:1;
    i2c_address_t address;
    uint8_t *data_ptr;
    uint8_t data_length;
    i2c_callback callbackTable[5];
    void *callbackPayload[5]; // each callback can have a payload
    uint8_t state;
} i2c1_status;

void I2C1_Initialize(void)
{
    i2c1_status.busy = 0;
    i2c1_status.writing = 0;
    i2c1_status.address = 0xFF;
    i2c1_status.state = I2C_SEND_ADDR;
    // set all the call backs to a default of sending stop
    i2c1_status.callbackTable[i2c_addressNACK] = asyncStop;
    i2c1_status.callbackTable[i2c_dataNACK] = asyncStop;
    i2c1_status.callbackTable[i2c_readComplete] = asyncStop;
    i2c1_status.callbackTable[i2c_writeComplete] = asyncStop;
    i2c1_status.callbackTable[i2c_writeCollision] = asyncStop;
    i2c1_status.callbackPayload[i2c_addressNACK] = 0;
    i2c1_status.callbackPayload[i2c_dataNACK] = 0;
    i2c1_status.callbackPayload[i2c_readComplete] = 0;
    i2c1_status.callbackPayload[i2c_writeComplete] = 0;
    i2c1_status.callbackPayload[i2c_writeCollision] = 0;

    // BF RCinprocess_TXcomplete; UA don't update; SMP Sample At Middle; P stop bit_not detected; S startbit_notdetected; R_nW write_noTX; CKE Idle to Active; D_nA lastbyte_address; 
    SSP1STAT = 0x00;
    // SSPEN enabled; WCOL no_collision; SSPOV no_overflow; CKP Idle:Low, Active:High; SSPM FOSC/4_SSPxADD; 
    SSP1CON1 = 0x28;
    // BOEN disabled; AHEN disabled; SBCDE disabled; SDAHT 100ns; DHEN disabled; ACKTIM ackseq; PCIE disabled; SCIE disabled; 
//    SSP1CON3 = 0x00;
    // Baud Rate Generator Value: SSP1ADD 103;   
    SSP1ADD = 0x67;

    /* Byte sent or received */
    // clear the master interrupt flag
    PIR1bits.SSP1IF = 0;
    // enable the master interrupt
    PIE1bits.SSP1IE = 1;
    
    IPR1bits.SSP1IP = 0; //low priority interrupt for I2C
}

// when you call open, you supply a device address.
// if you get the bus, the function returns true
bool i2c1_open(i2c_address_t address)
{
    bool ret = false;
    
    if(!i2c1_status.busy && i2c1_status.address == 0xFF)
    {
        i2c1_status.address = address;
        
        // set all the call backs to a default of sending stop
        i2c1_status.callbackTable[i2c_addressNACK]      = asyncStop;
        i2c1_status.callbackTable[i2c_dataNACK]         = asyncStop;
        i2c1_status.callbackTable[i2c_readComplete]     = asyncStop;
        i2c1_status.callbackTable[i2c_writeComplete]    = asyncStop;
        i2c1_status.callbackTable[i2c_writeCollision]   = asyncStop;
        i2c1_status.callbackPayload[i2c_addressNACK]    = 0;
        i2c1_status.callbackPayload[i2c_dataNACK]       = 0;
        i2c1_status.callbackPayload[i2c_readComplete]   = 0;
        i2c1_status.callbackPayload[i2c_writeComplete]  = 0;
        i2c1_status.callbackPayload[i2c_writeCollision] = 0;

        ret = true;
    }
    return ret;
}

// close the bus.
// if the bus is already closed
bool i2c1_close(i2c_address_t address)
{
    bool ret = false;
    if(!i2c1_status.busy && address == i2c1_status.address) // do you own the bus?
//    if(address == i2c1_status.address) // do you own the bus?
    {
        // close it down
        i2c1_status.address = 0xFF; // 8-bit address is invalid so this is FREE        
        ret = true;
    }
    return ret;
}

static bool setCallBack(i2c_callbackIndex idx, i2c_callback cb,void *p)
{
    bool ret = false;
//    if(!i2c1_status.busy)
//    {
        if(cb)
        {
            i2c1_status.callbackTable[idx] = cb;
            i2c1_status.callbackPayload[idx] = p;
        }
        else // null was passed so insert a STOP
        {
            i2c1_status.callbackTable[idx] = asyncStop;
            i2c1_status.callbackPayload[idx] = NULL;
        }
        ret = true;
//    }
    return ret;
}

static i2c_operations_t asyncStop(void *p)
{
    return i2c_stop;
}

void i2c1_asyncMasterReWrite(void *dataToWrite, size_t writeSize)
{
    i2c1_status.data_ptr = dataToWrite;
    i2c1_status.data_length = writeSize;
    i2c1_status.writing = true;    
}

void i2c1_asyncMasterReRead(void *bufferForRead, size_t bufferSize)
{
    i2c1_status.data_ptr = bufferForRead;
    i2c1_status.data_length = bufferSize;
    i2c1_status.writing = false;
}

i2c_error_t i2c1_asyncMasterWrite(void *dataToWrite, size_t writeSize)
{
    i2c_error_t ret = I2C_BUSY;
    if(!i2c1_status.busy)
    {
        i2c1_status.data_ptr = dataToWrite;
        i2c1_status.data_length = writeSize;
        i2c1_status.busy = true;
        i2c1_status.writing = true;
        ret = I2C_NOERR;
        SSP1CON2bits.SEN = 1;
    }
    return ret;
}

i2c_error_t i2c1_asyncMasterRead(void *bufferForRead, size_t bufferSize)
{
    i2c_error_t ret = I2C_BUSY;
    if(!i2c1_status.busy)
    {
        i2c1_status.data_ptr = bufferForRead;
        i2c1_status.data_length = bufferSize;
        i2c1_status.busy = true;
        i2c1_status.writing = false;
        ret = I2C_NOERR;
        SSP1CON2bits.SEN = 1;
    }    
    return ret;
}

i2c_error_t i2c1_blockingMasterWrite(uint8_t address, void *dataToWrite, size_t writeSize)
{
    i2c_error_t ret = I2C_BUSY;
    
    if(i2c1_open(address))
    {
        i2c1_asyncMasterWrite(dataToWrite, writeSize);
        while(!i2c1_close(address));// sit here until the close succeeds
        ret = I2C_NOERR;
    }
    return ret;
}

i2c_error_t i2c1_blockingMasterRead(uint8_t address, void *bufferForRead, size_t bufferSize)
{
    i2c_error_t ret = I2C_BUSY;
    
    if(i2c1_open(address))
    {        
        i2c1_asyncMasterRead(bufferForRead, bufferSize);
        while(!i2c1_close(address)); // sit here until the close succeeds
        ret = I2C_NOERR;
    }
    return ret;
}

/****************************************************************************/
void I2C1_ISR(void)
{    
    uint8_t nextState,i;
    PIR1bits.SSPIF = 0;
    

//    LED_D4_SetHigh();
    // Check first if there was a collision.
    if (SSP1CON1bits.WCOL)
    {
        nextState =  i2c1_functionTable[I2C_WRITECOLLISION]();
    }
    else
    {
        nextState = i2c1_functionTable[i2c1_status.state]();
    }
    if(nextState <= MAX_STATE)
        i2c1_status.state = nextState;
    else
    {
        //TODO STATE MACHINE OVERFLOW
    }
//    LED_D4_SetLow();
}

void I2C1_BusCollisionISR(void)
{
    PIR2bits.BCLIF = 0; // clear the bus collision.
}

inline uint8_t i2c1_idleFunction(void)
{
    i2c1_status.busy = 0;
    return I2C_SEND_ADDR;
}

inline uint8_t i2c1_writeCollisionFunction(void)
{
    SSP1CON1bits.WCOL = 0; // clear the write collision
    return issueCallback(i2c_writeCollision,I2C_IDLE);
}

inline uint8_t i2c1_sendAddrFunction(void)
{
    uint8_t ret = I2C_START_WRITING;
    uint8_t address = i2c1_status.address << 1;
    if(!i2c1_status.writing)
    {
        ret = I2C_START_RECEIVING;
        address |= 0x01;
    }
    SSP1BUF = address;
    return ret;
}

inline uint8_t i2c1_startWritingFunction(void)
{
    uint8_t ret = I2C_SEND_DATA;

    if(SSP1CON2bits.ACKSTAT==0) // received an ACK... start sending data
    {
        SSP1BUF = *i2c1_status.data_ptr++;
        i2c1_status.data_length--;
    }
    else // address was NACKED...Abort
    {
        ret = issueCallback(i2c_addressNACK,ret);
    }
    return ret;
}

inline uint8_t i2c1_startReceivingFunction(void)
{
    uint8_t ret = I2C_RCV_DATA;

    if(SSP1CON2bits.ACKSTAT==0) // received an ACK... start sending data
    {
        SSP1CON2bits.RCEN = 1; // start a receive
    }
    else // address was NACKED...Abort
    {
        ret = issueCallback(i2c_addressNACK,ret);
    }
    return ret;
}

inline uint8_t i2c1_sendDataFunction(void)
{
    uint8_t ret = I2C_SEND_DATA;

    if(SSP1CON2bits.ACKSTAT==0) // received an ACK... start sending data
    {
        if(i2c1_status.data_length)
        {
            SSP1BUF = *i2c1_status.data_ptr++;
            i2c1_status.data_length--;
        }
        else
        {
            ret = issueCallback(i2c_writeComplete,ret);
        }
    }
    else // address was NACKED...Abort
    {
        ret = issueCallback(i2c_dataNACK,ret);
    }
    return ret;
}

inline uint8_t i2c1_rcvDataFunction(void)
{
    uint8_t ret = I2C_DATA_RCVD;
    *i2c1_status.data_ptr++ = SSP1BUF;
    i2c1_status.data_length--;
    if(i2c1_status.data_length)
    {
        SSP1CON2bits.ACKDT = 0; // ACK because we can still receive data
    }
    else
    {
        SSP1CON2bits.ACKDT = 1; // NACK because we are done.
    }
    SSP1CON2bits.ACKEN = 1; // start the ACK/NACK
    return ret;
}

inline uint8_t i2c1_dataRcvdFunction(void)
{
    uint8_t ret = I2C_RCV_DATA;
    if(i2c1_status.data_length)
        SSP1CON2bits.RCEN = 1; // receive a byte
    else
        ret = issueCallback(i2c_readComplete,ret);    
    return ret;
}

inline static uint8_t issueCallback(uint8_t idx, uint8_t thisState)
{
    uint8_t ret;
    switch(i2c1_status.callbackTable[idx](i2c1_status.callbackPayload[idx]))
    {
        default:
        case i2c_stop:
            ret = I2C_IDLE;
            // then send a stop
            SSP1CON2bits.PEN = 1;
            break;
        case i2c_restart:
            ret = I2C_SEND_ADDR;
            // then send a restart
            SSP1CON2bits.RSEN = 1; // issue a restart
            break;
        case i2c_continue:
            PIR1bits.SSPIF = 1;
            ret = thisState; // stay in the state supplied by the calling function.
            break;
        case i2c_resetLink:
            // TODO restart the I2C Peripheral, cleanup the SDA/SCL line, 
            SSP1CON2bits.PEN = 1;
            ret = I2C_IDLE;
            break;
    }
    return ret;
}
