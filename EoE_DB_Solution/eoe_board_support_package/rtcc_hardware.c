/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    file_template_source.c

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

 */

/*

?  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "rtcc.h"
#include "hardware.h"
#include "led_board.h"
#include "tmr1.h"
#include "mac_address.h"

/******************************************************************************/
uint8_t rtccDelay_5ms = 5;

static void eepr_write_latch_enable(void);
static void eepr_write_latch_disable(void);
static uint8_t read_status_register(void);
volatile time_t deviceTime;
volatile uint16_t seconds_counter;
volatile uint8_t PoESACount;
volatile BIT dirtyTime;
DateTime rtcc_datetime;

/******************************************************************************/

void rtcc_init(void)
{
    uint8_t reg = 0;
    CS_RTCC_HIGH();
    
    // read the registers we will modify later
    rtcc_datetime.day = rtcc_read(RTCC_DAY);
    rtcc_datetime.sec = rtcc_read(RTCC_SECONDS);
    
    //Configure Control Register - SQWE=1, ALM0 = 00 {No Alarms Activated},
    //                             RS2, RS1, RS0 = 000 {1 HZ}
    rtcc_write(CONTROL_REG, ALM_NO + SQWE + MFP_01H);   
    NO_OPERATION();
    NO_OPERATION();
    
    // Start the external crystal and check OSCON to know when it is running
    rtcc_write(RTCC_SECONDS, rtcc_datetime.sec | ST_SET);
    while(!reg)
    {
        reg = rtcc_read(RTCC_DAY);
        reg &= OSCON;
    }
    
    // Configure external battery enable BIT and clear the VBAT flag
    rtcc_write(RTCC_DAY, rtcc_datetime.day | (VBATEN & VBAT_CLR));
    NO_OPERATION();
    NO_OPERATION();
    
    // Read time from rtcc
    deviceTime = rtcc_get();  
    
    INT1E=1;
}

void rtcc_time_init(void)
{
    if((rtcc_datetime.day &OSCON)== OSCON)
    {
        ;
    }
    else
    {
        rtcc_write(RTCC_YEAR,16);
        rtcc_write(RTCC_MONTH,1);
        rtcc_write(RTCC_DATE,12);
        rtcc_write(RTCC_HOUR,10);
        rtcc_write(RTCC_MINUTES,0);
        rtcc_write(RTCC_SECONDS,10|ST_SET);       
    }
}

uint8_t rtcc_read(uint8_t addr)
{
    uint8_t ret;
    
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(RTCC_READ);     //RTCC Read Instruction
    RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from
    ret = RTCC_SPI_READ();
     
    CS_RTCC_HIGH();
  
    return ret;    
}

void rtcc_write(uint8_t addr, uint8_t data)
{    
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(RTCC_WRITE);     //RTCC Write Instruction
    RTCC_SPI_WRITE(addr);          //Write RTCC address to write data to
    RTCC_SPI_WRITE(data);          //Write data
    CS_RTCC_HIGH();   
}

void rtcc_read_block(uint8_t addr, void *data, uint8_t len)
{    
    register char *p = (char *) data;
    
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(RTCC_READ);     //RTCC Read Instruction
    RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from
    while(len--)
    {
       *p++ = RTCC_SPI_READ();
    }
    CS_RTCC_HIGH();    
   
}

void rtcc_set(time_t t)
{
    bool gie_val;
    struct tm *tm_t;
    
    gie_val = GIE;
    GIE = 0;    
   
    tm_t = localtime(&t);
    
    rtcc_write(RTCC_YEAR,tm_t->tm_year);
    rtcc_write(RTCC_MONTH,tm_t->tm_mon);
    rtcc_write(RTCC_DATE,tm_t->tm_mday);
    rtcc_write(RTCC_HOUR,tm_t->tm_hour);
    rtcc_write(RTCC_MINUTES,tm_t->tm_min);
    rtcc_write(RTCC_SECONDS,(tm_t->tm_sec)|ST_SET);     
    
    GIE = gie_val;
    deviceTime = rtcc_get();
    dirtyTime = false;
    NO_OPERATION();
    NO_OPERATION();
}

time_t rtcc_get(void)
{
    bool gie_val;
    struct tm tm_t;   
    
    gie_val = GIE;
    GIE = 0;   
    
    tm_t.tm_year = rtcc_read(RTCC_YEAR);
    tm_t.tm_mon = rtcc_read(RTCC_MONTH);
    tm_t.tm_mday = rtcc_read(RTCC_DATE);
    tm_t.tm_hour = rtcc_read(RTCC_HOUR);
    tm_t.tm_min =  rtcc_read(RTCC_MINUTES); 
    tm_t.tm_sec = rtcc_read(RTCC_SECONDS);   
 
    GIE = gie_val;
    dirtyTime = true;
    return  mktime(&tm_t);
}


//void rtcc_handler()
//{
//    bool ret = false;
//    NO_OPERATION();
//    NO_OPERATION();
//    if(INT1IF == 1)//TODO - Check for the Interrupt edge
//    {
//         INT1IF = 0;
//         ret = true;
////        deviceTime++;
////        LATGBITs.LATG6^=1;
//         
//    }
//    return ret;
//}

BIT rtcc_isDirty(void)
{
    return dirtyTime;
}


inline void maintain_PoESA(void)
{
    PoESACount--;
    if(PoESACount ==0)
    {
        if (POE_STAY_ALIVE_LAT) PoESACount = PoESA_OFF_TIME;
        else PoESACount = PoESA_ON_TIME;
        POE_STAY_ALIVE_LAT ^= 1;
    }
}

time_t time(time_t *t)
{
    bool   gie_val;
    time_t  the_time;
    
    gie_val = GIE;
    GIE = 0;
    the_time = deviceTime;
    GIE = gie_val;

    if(t)
    {
        *t = the_time;
    }

    return (the_time);
}



/**********************TMR1 and RTCC Interrupt Handler************************/

void rtcc_handler(void)
{ 
//    if(INT1E && INT1IF)
//    {
//        INT1IF = 0;
//        TMR1IF = 0;
//        TMR1ON = 0;
//       
//        TMR1 = TMR1_RELOAD_BACKUP;//tmr1 reload
//        seconds_counter = TMR1_TICKS_PER_SECOND;
//        TMR1ON = 1;
//        LED1_LAT ^= 1;
//        deviceTime++;
//    }
    if(TMR1IE && TMR1IF)
    {
        TMR1IF = 0;
        TMR1ON = 0;
        TMR1 = TMR1_RELOAD;//tmr1 reload
        TMR1ON = 1;
        seconds_counter--;
        if(seconds_counter == 0)
        { 
            seconds_counter = TMR1_TICKS_PER_SECOND;
            deviceTime++;
            led_update();
        }
    }  
    maintain_PoESA();
}

/******************************************************************************/

static void eepr_write_latch_enable(void)
{
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(EEWREN);        //Write Enable Instruction
    CS_RTCC_HIGH();                //CS High
}

static void eepr_write_latch_disable(void)
{
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(EEWRDI);        //Write Disable Instruction
    CS_RTCC_HIGH();                //CS High
}

static uint8_t read_status_register(void)
{
    uint8_t ret;
    
    CS_RTCC_LOW();
    RTCC_SPI_WRITE(SRREAD);                // Read Status Register
    ret = RTCC_SPI_READ();
    CS_RTCC_HIGH();
    
    return ret;
}

uint8_t eepr_read_mcp79512(uint8_t addr)
{
    uint8_t ret;
    uint8_t stat_reg; 
    eepr_write_latch_disable();      //Disable write latch
    
     do
    {
        stat_reg = read_status_register();  //Read Status Register
    }while(((stat_reg & 0x03) != 0x00));
    
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(EEREAD);        //RTCC Read Instruction
    RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from
    ret = RTCC_SPI_READ();         //Read Data
    CS_RTCC_HIGH();                //CS High
    
    return ret;    
}

void eepr_read_block_mcp79512(uint8_t addr, void *data, uint8_t len)
{  
    uint8_t stat_reg; 
    register char *p = (char *) data;
   
    eepr_write_latch_disable();      //Disable write latch   
    
     do
    {
        stat_reg = read_status_register();  //Read Status Register
    }while(((stat_reg & 0x03) != 0x00));
    
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(EEREAD);        //RTCC Read Instruction
    RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from
    while(len--)
    {
        *p++ = RTCC_SPI_READ();         //Read Data
        addr++;
        if(addr%EEPR_PAGE_SIZE)
        {
            CS_RTCC_HIGH();                //CS High
            eepr_write_latch_disable();      //Disable write latch
            CS_RTCC_LOW();                 //CS low
            RTCC_SPI_WRITE(EEREAD);        //RTCC Read Instruction
            RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from
        }
    }  
    CS_RTCC_HIGH();                //CS High    
}

void eepr_write_mcp79512(uint8_t addr, uint8_t data)
{ 
    uint8_t stat_reg; 
    do{
        stat_reg = read_status_register();  //Read Status Register to check for bus not busy
    }while((stat_reg & 0x01) != 0x00); 

    eepr_write_latch_enable();      //Enable write latch
    
    do{
        stat_reg = read_status_register();  //Read Status Register
    }while((stat_reg & 0x03) != 0x02);
    
    CS_RTCC_LOW();  
    RTCC_SPI_WRITE(EEWRITE);        //RTCC Write Instruction
    RTCC_SPI_WRITE(addr);           //Write RTCC address to write data to
    RTCC_SPI_WRITE(data);           //Write data
    CS_RTCC_HIGH();                 //CS High
    
   eepr_write_latch_disable();      //Disable write latch
}

void eepr_write_block_mcp79512(uint8_t addr, void *data, uint8_t len)
{ 
    uint8_t stat_reg; 
    register char *p = (char *) data;
    
     do{
        stat_reg = read_status_register();  //Read Status Register to check for bus not busy
    }while((stat_reg &0x01) != 0x00); 
    
    eepr_write_latch_enable();      //Enable write latch
    
    do{
        stat_reg = read_status_register();  //Read Status Register
    }while((stat_reg & 0x03) != 0x02);    
   
    CS_RTCC_LOW();                 //CS low
    RTCC_SPI_WRITE(EEWRITE);     //RTCC Write Instruction
    RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from
    while(len--)
    {
        RTCC_SPI_WRITE(*p++);        
        addr++;
        if(addr%EEPR_PAGE_SIZE==0)
        {
            CS_RTCC_HIGH();              
            DELAY_MS(rtccDelay_5ms);
            eepr_write_latch_enable();      //Enable write latch
            CS_RTCC_LOW();                 //CS low
            RTCC_SPI_WRITE(EEWRITE);     //RTCC Read Instruction
            RTCC_SPI_WRITE(addr);          //Write RTCC address to read data from      
        }       
    }
    CS_RTCC_HIGH();    
    eepr_write_latch_disable();      //Disable write latch      
}

/******************************************************************************/

//void e48_read_mac(uint8_t *mac)
//{
//    #define xor_mask        0xDEADBEAT
//    #define mac_64_bytes    8
//    #define  mac_48_bytes   6
//    mac64Address_t mac64add;
//    
//    
//    CS_RTCC_LOW();                 //CS low
//    RTCC_SPI_WRITE(IDREAD);        //Unique ID Read Instruction
//    RTCC_SPI_WRITE(MAC_ADDR_48);          //Write Unique ID address to read data from
//
//    while(mac_48_bytes--)
//    {
//        *mac++ = RTCC_SPI_READ();         //Read Data
//    }
//    CS_RTCC_HIGH();                //CS High
//}
//    
//void e48_read_mac(uint8_t *mac)
//{
// 
//    #define mac_64_bytes    8
//    #define  mac_48_bytes   6
//    mac64Address_t mac64add;
//    uint8_t x;
//    
//    
//    CS_RTCC_LOW();                 //CS low
//    RTCC_SPI_WRITE(IDREAD);        //Unique ID Read Instruction
//    RTCC_SPI_WRITE(MAC_ADDR_48);          //Write Unique ID address to read data from
//
//    for(x=0; x<=mac_48_bytes; x++)
//    {
//        mac64add.mac_array[x] = RTCC_SPI_READ();         //Read Data
//    }
//    CS_RTCC_HIGH();  
//    
//    
////    printf("Original macAddr:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n",mac64add.mac_array[0],mac64add.mac_array[1],mac64add.mac_array[2],mac64add.mac_array[3],mac64add.mac_array[4],mac64add.mac_array[5]);
//    
//    if ((uint8_t)mac64add.mac_array[0]!=0)//errata: wrong part
//    {
//        printf("WARNING:EUI 64 Detected, Address may not be unique\r\n");
//        CS_RTCC_LOW();                 //CS low
//        RTCC_SPI_WRITE(IDREAD);        //Unique ID Read Instruction
//        RTCC_SPI_WRITE(MAC_ADDR_64);          //Write Unique ID address to read data from
//
//        for(x=0; x<=mac_64_bytes; x++)
//        {
//            mac64add.mac_array[x] = RTCC_SPI_READ();         //Read Data
//        }
//        CS_RTCC_HIGH();                //CS High
//        
//        mac64add.s.byte4 ^= 0xB;
//        mac64add.s.byte5 ^= 0xD;
//        mac64add.s.byte6 ^= 0xE;
//        mac64add.s.byte7 ^= 0xA;
//        mac64add.s.byte8 ^= 0xD;
//        
//        mac[0] = mac64add.mac_array[0];
//        mac[1] = mac64add.mac_array[1];
//        mac[2] = mac64add.mac_array[2];
//        mac[3] = mac64add.mac_array[3]^mac64add.mac_array[4]^mac64add.mac_array[5];
//        mac[4] = mac64add.mac_array[4]^mac64add.mac_array[5]^mac64add.mac_array[6];
//        mac[5] = mac64add.mac_array[6]^mac64add.mac_array[7]^mac64add.mac_array[8];
////        printf("Fabricated macAddr:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
//   }
//   else
//   {
//    for(x=0; x<=mac_48_bytes; x++)
//        {
//            mac[x] = mac64add.mac_array[x];
//        }
//   }
//}
    
 
    
    

    
