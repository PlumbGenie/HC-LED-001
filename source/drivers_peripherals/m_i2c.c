//*************************************************************************
//                           m_i2c.c
//                         Version 2.0
//
// Mikes simple software driven I2C routine - hopefully done for compactness.
// Master Only mode - output only in this version
//
// Author: Michael Pearce
//         Chemistry Department, University of Canterbury
//
// Started: 2 June 1999
//
//*************************************************************************
// Version 2.0 - 1 September 2016
//  Some API Changes, clean ups and update to XC8 plus PIC cores with LAT's
//
// Version 1.0.0 - 2 June 1999
//  Single routine to write data to i2c
//*************************************************************************
#include <xc.h>
#include <stdint.h>
#include "m_i2c.h"
#include "hardware.h"


//******** REQUIRED DEFINES ***********


#define I2CLOW  0         //-- Puts pin into output/low mode
#define I2CHIGH 1         //-- Puts pin into Input/high mode



//********* I2C Bus Timing - uS ************

#define I2CSTARTDELAY 1
#define I2CSTOPDELAY  1
#define I2CDATASETTLE 2
#define I2CCLOCKHIGH  1
#define I2CHALFCLOCK  1
#define I2CCLOCKLOW   1
#define I2CACKWAITMIN 1

//Working for GestIC and PIC1713
//#define I2CSTARTDELAY 4
//#define I2CSTOPDELAY  12
//#define I2CDATASETTLE 25
//#define I2CCLOCKHIGH  24
//#define I2CHALFCLOCK  12
//#define I2CCLOCKLOW   24
//#define I2CACKWAITMIN 12

//#define I2CSTARTDELAY 4
//#define I2CSTOPDELAY  8
//#define I2CDATASETTLE 1
//#define I2CCLOCKHIGH  6
//#define I2CHALFCLOCK  3
//#define I2CCLOCKLOW   6
//#define I2CACKWAITMIN 8

#define ACK  0
#define NACK 1


//********************* FUNCTIONS ************************

void i2cInit()
{
    SDA_TRIS  = 1;
    SCL_TRIS  = 1;
    SDA_LAT   = 0;
    SCL_LAT   = 0;    
}

//*************************************************************************
//                          I2C_Write
//
// Inputs:
//         char Address  - Address to write data to
//         char *Data    - Pointer to buffer
//         char Num      - Number of bytes to send
//*************************************************************************
uint8_t i2cWrite(uint8_t Address,uint8_t *Data,uint8_t Num)
{
    uint8_t ret;                            //jira: CAE_MCU8-5958
    i2cSendStart();
    
    Address <<=1;
    
    //-- Send Address - WRITE Mode 
    ret = i2cWriteByte(Address & 0xFEu);   //-- Lowest bit = 0 => WRITE    //jira: CAE_MCU8-5647, CAE_MCU8-5958
    
    if(!ret)                               //jira: CAE_MCU8-5958
    {
        i2cSendStop();
        return(0);
    }
    while(Num--)
    {
        ret = i2cWriteByte(*Data);        //jira: CAE_MCU8-5958
        Data++;
        
        if(!ret)
        {
            i2cSendStop();
            return(0);
        }
       
    }
    i2cSendStop();
    return(1);
}
//************** END OF I2C_Write


//*************************************************************************
//                          I2C_Read
//
// Inputs:
//         char Address  - Address to write data to
//         char *Data    - Pointer to buffer
//         char Num      - Number of bytes to send
//*************************************************************************
uint8_t i2cRead(uint8_t Address,uint8_t *Data,uint8_t Num)
{
    uint8_t ret;
    
    Address <<=1; // Shift to positon
    Address |=1;  // Add Read Bit
    
    i2cSendStart();
    ret = i2cWriteByte(Address);
	
	if (ret == ACK)
    {
        ret = i2cReadBytes(Num, Data);
    }
        
    i2cSendStop();
    
    return ret;
}

//************** END OF I2C_Read

//*************************************************************************
//                        i2csendbyte
//*************************************************************************
uint8_t i2cWriteByte(uint8_t Byte)
{
    uint8_t count;

    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time (between bytes))
    
    for(count=8;count!=0;count--)   //-- Send 8 bits of data
    {
        if( (Byte & 0x80u) )        //-- Get the Bit   //jira: CAE_MCU8-5647
        {
            SDA_TRIS=I2CHIGH;            //-- Release pin if bit = 1
        }
        else
        {
            SDA_TRIS=I2CLOW;             //-- Lower pin if bit = 0
        }
        i2cClock();                   //-- Pulse the clock
        Byte=(uint8_t)(Byte <<1u);                 //-- Shift next bit into position  //jira: CAE_MCU8-5647
    }
    
    SDA_TRIS=I2CHIGH;              //-- Release data pin for ACK
    return i2cGetAck();
}
//************** END OF i2csendbyte

//*************************************************************************
//                        i2c_send_bytes
//*************************************************************************
uint8_t i2cWriteBytes(uint8_t len, const void *Byte)
{
    uint8_t *d = Byte;
    uint8_t ret;
    
    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
    while(len)
    {
        ret = i2cWriteByte(*d++);
        if(ret == NACK) break;
        len--;
    }
    return ret;

}
//************** END OF i2c_send_bytes

//*************************************************************************
//                      char i2c_receive_byte(void)
//
//  Reads in a byte from the I2C Port
//*************************************************************************
uint8_t i2cReadByte(uint8_t *Byte, uint8_t AckNack)
{
    uint8_t count;
         
    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
    SDA_TRIS=I2CHIGH;              //-- Release pin so data can be received
    
    for(count=8;count>0;count--)   //-- Read 8 bits of data
    {
        *Byte=(uint8_t)(*Byte <<1u);   //jira: CAE_MCU8-5647
        if(i2cReadBit())
        {
            *Byte |= 1u;    //jira: CAE_MCU8-5647
        }
    }
  //  count =  i2cGetAck(); //i2cSendNack();
    if(AckNack==ACK)
    {
        count = i2cSendAck();
    }
    else
    {
        count= i2cSendNack();
    }
    return count;
}


//*************************************************************************
//                        i2csendbyte
//*************************************************************************
uint8_t i2cReadBytes(uint8_t len, uint8_t *Byte)
{
    uint8_t ret,acknack=ACK;

    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
    while(len --)
    {
        if(len==0)acknack=NACK;
        ret=i2cReadByte(Byte++,acknack);    
    }
 
 return ret;
}
//************** END OF i2c_send_bytes


//*************************************************************************
//                          i2creadbit
//*************************************************************************
uint8_t i2cReadBit(void)
{
    uint8_t Data=0, timeout=0;

    i2cDelay(I2CDATASETTLE);       //-- Minimum Clock Low Time
    SCL_TRIS=I2CHIGH;              //-- Release clock
    while(SCL_PORT==0)             //-- Handle Clock Stretch
    {
        i2cDelay(I2CHALFCLOCK); 
        if(++timeout >100) break;
    }
    i2cDelay(I2CHALFCLOCK);        //-- 1/2 Minimum Clock High Time
    if(SDA_PORT !=0 ) Data=1;      //-- READ in the data bit
    i2cDelay(I2CHALFCLOCK);        //-- 1/2 Minimum Clock High Time
    SCL_TRIS=I2CLOW;               //-- Lower the clock
    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
    return(Data);
}
//************** END OF i2cclock
//*************************************************************************
//                           i2cstart
//*************************************************************************
uint8_t i2cSendStart(void)
{
    //-- Ensure pins are in high impedance mode and LATS are Low --
    SCL_LAT=I2CLOW;
    SDA_LAT=I2CLOW;
    SDA_TRIS=I2CHIGH;
    SCL_TRIS=I2CHIGH;

    i2cDelay(I2CSTARTDELAY);

    //-- Generate the start condition
    SDA_TRIS=I2CLOW;
    i2cDelay(I2CSTARTDELAY);

    SCL_TRIS=I2CLOW;
    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time

    return 1;
}
//************** END OF i2cstart

//*************************************************************************
//                           i2cstop
//*************************************************************************
uint8_t i2cSendStop(void)
{
    
    i2cDelay(I2CSTOPDELAY); // Gap before sending stop

     //-- Generate Stop Condition --
    SDA_TRIS=I2CLOW;
    i2cDelay(I2CSTOPDELAY);
    SCL_TRIS=I2CHIGH;
    i2cDelay(I2CSTOPDELAY);
    SDA_TRIS=I2CHIGH;

    return 1;
}
//************** END OF i2cstop

//*************************************************************************
//                          i2cclock
//*************************************************************************
void i2cClock(void)
{
    uint8_t timeout=0;
    
    i2cDelay(I2CDATASETTLE);       //-- Minimum Clock Low Time
    SCL_TRIS=I2CHIGH;              //-- Release clock
    
    while(SCL_PORT==I2CLOW)        //-- Handle Clock stretching
    {
        i2cDelay(I2CCLOCKHIGH);
        timeout++;
        if(timeout > 100) break;   //-- Exit if more than 100 clock high times
    }
    
    i2cDelay(I2CCLOCKHIGH);        //-- Minimum Clock High Time
    
    
    SCL_TRIS=I2CLOW;               //-- Lower the clock
    i2cDelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
}
//************** END OF i2cclock


//*************************************************************************
//                           i2cgetack
//*************************************************************************
uint8_t i2cGetAck(void)
{
    SCL_TRIS=I2CLOW;                 //-- Ensure clock is low
    SDA_TRIS=I2CHIGH;                //-- Release the Data pin so slave can ACK
    SCL_TRIS=I2CHIGH;                //-- raise the clock pin
    i2cDelay(I2CHALFCLOCK);          //-- wait for 1/2 the clock pulse
    if(SDA_PORT)                     //-- sample the ACK signal
    {   
        i2cDelay(I2CHALFCLOCK);      //-- Else wait for rest of clock
        SCL_TRIS=I2CLOW;
        return NACK;                  //-- No ACK so return BAD
    }
    i2cDelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
    SCL_TRIS=I2CLOW;                 //-- Finish the clock pulse
    i2cDelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    i2cDelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time

    
    return ACK;
}
//************** END OF i2cgetack

//*************************************************************************
//                           i2csendack
//*************************************************************************
uint8_t i2cSendAck(void)
{
   
    SCL_TRIS=I2CLOW;                 //-- Ensure clock is low
    SDA_TRIS=I2CLOW;                //-- Lower the Data pin to ACk data from slave
    SCL_TRIS=I2CHIGH;                //-- raise the clock pin
    i2cDelay(I2CHALFCLOCK);          //-- wait for 1/2 the clock pulse
    i2cDelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
    SCL_TRIS=I2CLOW;                 //-- Finish the clock pulse
    i2cDelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    i2cDelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    return(ACK);
}
//************** END OF i2cgetack

uint8_t i2cSendNack(void)
{    
    uint8_t retval;
    
    SDA_TRIS=I2CHIGH;               //--  Release Data pin as NACK
    SCL_TRIS=I2CHIGH;               //-- raise the clock pin
    i2cDelay(I2CHALFCLOCK);         //-- wait for 1/2 the clock pulse
    if(SDA_PORT)                    //-- sample the ACK signal
    {
        retval=NACK;
    }
    else
    {
        retval=ACK;
    }
    i2cDelay(I2CHALFCLOCK);         //-- Else wait for rest of clock
    SCL_TRIS=I2CLOW;                //-- Finish the clock pulse
    i2cDelay(I2CCLOCKLOW);          //-- Minimum Clock Low Time
    return retval;
}




