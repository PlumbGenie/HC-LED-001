//*************************************************************************
//                           m_i2c_1.c
//                         Version 1.0.0
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
// Version 1.0.0 - 2 June 1999
//  Single routine to write data to i2c
//*************************************************************************
#include <xc.h>
#include <stdint.h>

//#include "m_i2c_1.h"
#include "i2c_async.h"
#include "hardware.h"

//******** REQUIRED DEFINES ***********
// #define BITNUM(adr, bit)       ((unsigned)(&adr)*8+(bit))
// static bit SCL @ BITNUM(PORTA,0)  //-- The SCL output pin
// #define SCL_TRIS       //-- The SCL Direction Register Bit
// #define SDA            //-- The SDA output pin
// #define SDA_TRIS       //-- The SDA Direction Register Bit
// #define XTAL_FREQ 8MHZ //-- Define the crystal frequency

#define I2CLOW  0         //-- Puts pin into output/low mode
#define I2CHIGH 1         //-- Puts pin into Input/high mode

#ifndef MHZ
 #define	MHZ	*1000			/* number of kHz in a MHz */
#endif
#ifndef KHZ
 #define	KHZ	*1			/* number of kHz in a kHz */
#endif


//********* I2C Bus Timing - uS ************

//#define I2CSTARTDELAY 1
//#define I2CSTOPDELAY  1
//#define I2CDATASETTLE 2
//#define I2CCLOCKHIGH  1
//#define I2CHALFCLOCK  1
//#define I2CCLOCKLOW   1
//#define I2CACKWAITMIN 1

//Working for GestIC and PIC1713
//#define I2CSTARTDELAY 4
//#define I2CSTOPDELAY  12
//#define I2CDATASETTLE 25
//#define I2CCLOCKHIGH  24
//#define I2CHALFCLOCK  12
//#define I2CCLOCKLOW   24
//#define I2CACKWAITMIN 12

#define I2CSTARTDELAY 4
#define I2CSTOPDELAY  8
#define I2CDATASETTLE 12
#define I2CCLOCKHIGH  8
#define I2CHALFCLOCK  8
#define I2CCLOCKLOW   8
#define I2CACKWAITMIN 8

#define ACK  0
#define NACK 1

//********************* FUNCTIONS ************************

void I2C1_Initialize()
{
    SDA_TRIS = 1;
    SCL_TRIS  = 1;
    SDA_LAT   = 0;
    SCL_LAT  = 0;    
}

//*************************************************************************
//                          I2C_Send
//
// Inputs:
//         char Address  - Address to write data to
//         char *Data    - Pointer to buffer
//         char Num      - Number of bytes to send
//*************************************************************************
char I2C_Send(char Address,const void *Data,char Num)
{
    const char *d = Data;
    i2c_send_start();
    
    //-- Send Address
    i2csendbyte(Address & 0xFE);   //-- Lowest bit = 0 => WRITE
    if(!i2cgetack())
    {
        i2c_send_stop();
        return(0);
    }
    while(Num--)
    {
        i2csendbyte(*d);
        if(!i2cgetack())
        {
            i2c_send_stop();
            return(0);
        }
        d++;
    }
    i2c_send_stop();
    return(1);
}
//************** END OF I2C_Send

//*************************************************************************
//                        i2csendbyte
//*************************************************************************
char i2csendbyte(char Byte)
{
    char count;
    SDA_PORT=I2CLOW;
    SCL_PORT=I2CLOW;

    i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time

    for(count=8;count>0;count--)   //-- Send 8 bits of data
    {
     if( (Byte & 0x80)== 0)        //-- Get the Bit
     {
      SDA_PORT=I2CLOW;                  //-- Ensure Port pin is low
      SDA_TRIS=I2CLOW;             //-- Lower pin if bit = 0
     }
     else
     {
      SDA_TRIS=I2CHIGH;            //-- Release pin if bit = 1
     }
     Byte=Byte<<1;                 //-- Shift next bit into position
     i2cclock();                   //-- Pulse the clock
    }
    SDA_TRIS=I2CHIGH;              //-- Release data pin for ACK
    return i2cgetack();
}
//************** END OF i2csendbyte

//*************************************************************************
//                        i2c_send_bytes
//*************************************************************************
uint8_t i2c_send_bytes(uint8_t len, const void *Byte)
{
 const char *d = Byte;

 uint8_t ret;
 i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
 while(len)
 {
    ret = i2csendbyte(*d++);
    if(ret == NACK) break;
    len--;
 }
 if(ret==ACK)
     return I2C_FUNCTION_RETCODE_SUCCESS;
 else
     return I2C_FUNCTION_RETCODE_NACK;
}
//************** END OF i2c_send_bytes

//*************************************************************************
//                      char i2c_receive_bytes(void)
//
//  Reads in a byte from the I2C Port
//*************************************************************************
char i2c_receive_byte(char *Byte)
{
 char count;
 SDA_PORT=I2CLOW;
 SCL_PORT=I2CLOW;

 i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time

 for(count=8;count>0;count--)   //-- Read 8 bits of data
 {
  *Byte=*Byte <<1;
  SDA_TRIS=I2CHIGH;            //-- Release pin so data can be recieved
  if(i2creadbit())
  {
   *Byte |= 1;
  }
 }
 i2csendnack();
 return I2C_FUNCTION_RETCODE_SUCCESS;
}


//*************************************************************************
//                        i2csendbyte
//*************************************************************************
uint8_t i2c_receive_bytes(uint8_t len, char *Byte)
{
 SDA_PORT=I2CLOW;
 SCL_PORT=I2CLOW;

 i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
 while(len--)
 {
    i2c_receive_byte(Byte++);    
 }
 return I2C_FUNCTION_RETCODE_SUCCESS;
}
//************** END OF i2c_send_bytes


//*************************************************************************
//                          i2creadbit
//*************************************************************************
char i2creadbit(void)
{
 char Data=0;
 i2cdelay(I2CDATASETTLE);       //-- Minimum Clock Low Time
 SCL_TRIS=I2CHIGH;              //-- Release clock
 i2cdelay(I2CHALFCLOCK);        //-- 1/2 Minimum Clock High Time
 if(SDA_PORT !=0 ) Data=1;           //-- READ in the data bit
 i2cdelay(I2CHALFCLOCK);        //-- 1/2 Minimum Clock High Time
 SCL_TRIS=I2CLOW;               //-- Lower the clock
 i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
 return(Data);
}
//************** END OF i2cclock
//*************************************************************************
//                           i2cstart
//*************************************************************************
uint8_t i2c_send_start(void)
{
 //-- Ensure pins are in high impedance mode --
 SDA_TRIS=I2CHIGH;
 SCL_TRIS=I2CHIGH;
 SCL_LAT=I2CLOW;
 SDA_LAT=I2CLOW;
 i2cdelay(I2CSTARTDELAY);

 //-- Generate the start condition
 SDA_TRIS=I2CLOW;
 SDA_LAT=I2CLOW;
 i2cdelay(I2CSTARTDELAY);
 SCL_TRIS=I2CLOW;
 i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time

 return I2C_FUNCTION_RETCODE_SUCCESS;
}
//************** END OF i2cstart

//*************************************************************************
//                           i2cstop
//*************************************************************************
uint8_t i2c_send_stop(void)
{
  //-- Generate Stop Condition --
 SDA_TRIS=I2CLOW;
 SCL_TRIS=I2CHIGH;
 i2cdelay(I2CSTOPDELAY);
 SDA_TRIS=I2CHIGH;
 
 return I2C_FUNCTION_RETCODE_SUCCESS;
}
//************** END OF i2cstop

//*************************************************************************
//                          i2cclock
//*************************************************************************
void i2cclock(void)
{
 i2cdelay(I2CDATASETTLE);       //-- Minimum Clock Low Time
 SCL_TRIS=I2CHIGH;              //-- Release clock
 i2cdelay(I2CCLOCKHIGH);        //-- Minimum Clock High Time
 SCL_TRIS=I2CLOW;               //-- Lower the clock
 i2cdelay(I2CCLOCKLOW);         //-- Minimum Clock Low Time
}
//************** END OF i2cclock


//*************************************************************************
//                           i2cgetack
//*************************************************************************
char i2cgetack(void)
{
 SCL_TRIS=I2CLOW;                 //-- Ensure clock is low
 SDA_TRIS=I2CHIGH;                //-- Release the Data pin so slave can ACK
 SCL_TRIS=I2CHIGH;                //-- raise the clock pin
 i2cdelay(I2CHALFCLOCK);          //-- wait for 1/2 the clock pulse
 if(SDA_PORT)                          //-- sample the ACK signal
 {
    i2cdelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
    SCL_TRIS=I2CLOW;
    return NACK;                      //-- No ACK so return BAD
 }
 i2cdelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
 SCL_TRIS=I2CLOW;                 //-- Finish the clock pulse
 i2cdelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
 i2cdelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time

 return ACK;
}
//************** END OF i2cgetack

//*************************************************************************
//                           i2csendack
//*************************************************************************
char i2csendack(void)
{
    SDA_LAT=I2CLOW;
    SCL_LAT=I2CLOW;
    SCL_TRIS=I2CLOW;                 //-- Ensure clock is low
    SDA_TRIS=I2CHIGH;                //-- Release the Data pin so slave can ACK
    SCL_TRIS=I2CHIGH;                //-- raise the clock pin
    i2cdelay(I2CHALFCLOCK);          //-- wait for 1/2 the clock pulse
    if(SDA_PORT)                          //-- sample the ACK signal
    {
      return(0);                      //-- No ACK so return BAD
    }
    i2cdelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
    SCL_TRIS=I2CLOW;                 //-- Finish the clock pulse
    i2cdelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    i2cdelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    return(1);
}
//************** END OF i2cgetack

char i2csendnack(void)
{    
    SDA_TRIS=I2CLOW;                //-- Release the Data pin so slave can ACK
    SDA_PORT = I2CLOW;
    SCL_TRIS=I2CHIGH;                //-- raise the clock pin
    i2cdelay(I2CHALFCLOCK);          //-- wait for 1/2 the clock pulse
    if(SDA_PORT)                          //-- sample the ACK signal
    {
      i2cdelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
      SCL_TRIS=I2CHIGH;  
      return NACK;                      //-- No ACK so return BAD
    }
    i2cdelay(I2CHALFCLOCK);          //-- Else wait for rest of clock
    SCL_TRIS=I2CLOW;                 //-- Finish the clock pulse
    i2cdelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    i2cdelay(I2CCLOCKLOW);           //-- Minimum Clock Low Time
    return ACK;
}

i2c_error_t i2c1_blockingMasterWrite(uint8_t address, const void *dataToWrite, size_t writeSize)
{
   uint8_t sla = (address << 1) | 0; //write Address
	uint8_t ret_code = i2c_send_start(); 
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return ret_code;
		
	ret_code = i2c_send_bytes(1, &sla);
	
	if (ret_code == I2C_FUNCTION_RETCODE_SUCCESS)
    ret_code = i2c_send_bytes(writeSize, dataToWrite);
    
    i2c_send_stop();
   
    return ret_code;
}

i2c_error_t i2c1_blockingMasterRead(uint8_t address, void *bufferForRead, size_t bufferSize)
{
    
	uint8_t sla = (address << 1) | 1; //Read Address
	uint8_t ret_code = i2c_send_start(); 
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return ret_code;
		
	ret_code = i2c_send_bytes(1, &sla);
	
	if (ret_code == I2C_FUNCTION_RETCODE_SUCCESS)    
        ret_code = i2c_receive_bytes(bufferSize, bufferForRead);
    
    i2c_send_stop();
    
    return ret_code;
}

void I2C1_ISR()
{
    
}

