/**
 MAC EEPROM driver implementation

  Company:
    Microchip Technology Inc.

  File Name:
    mac_eeprom.c

  Summary:
    MAC EEPROM implementation.

  Description:
    This file provides the MAC EEPROM driver implementation.

 */

/*

�  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software
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
/*
 * File:   mac_eeprom.c
 * Author: C16813
 *
 * Created on June 30, 2016, 3:23 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "mac_eeprom.h"
#include "spi1.h"
#include "mac_address.h"

#define mac_64_bytes    8
#define  mac_48_bytes   6


uint8_t MACeepromDelay_10ms = 10;
/************************************************************************/
/* PROCEDURE: init														*/
/*																		*/
/* This procedure initializes the SCK to low. Must be called prior to 	*/
/* setting up mode 0.													*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		SCK																*/
/************************************************************************/
void macEeprInit()
{
   
    RTCC_CS_SetHigh();
    printf("MAC EEPROM Initialized.\r\n");
    
}

/************************************************************************/
/* PROCEDURE: Send_Byte													*/
/*																		*/
/* This procedure outputs a byte shifting out 1-bit per clock rising	*/
/* edge on the the SI pin(LSB 1st).										*/
/*																		*/
/* Input:																*/
/*		out																*/
/*																		*/
/* Output:																*/
/*		SI																*/
/************************************************************************/
void macEeprSendByte(uint8_t out)
{
    SPI1_Exchange8bit((char)out);   
}

/************************************************************************/
/* PROCEDURE: Get_Byte													*/
/*																		*/
/* This procedure inputs a byte shifting in 1-bit per clock falling		*/
/* edge on the SO pin(LSB 1st).											*/
/*																		*/
/* Input:																*/
/*		SO																*/
/*																		*/
/* Output:																*/
/*		None															*/
/************************************************************************/
uint8_t macEeprGetByte(void)
{
    uint8_t byte = 0;
    byte = SPI1_Exchange8bit((char)0);
    return byte;
}


/************************************************************************/
/* PROCEDURE: CE_High													*/
/*																		*/
/* This procedure set CE = High.										*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		CE																*/
/*																		*/
/************************************************************************/
void macEeprCE_High(void)
{
	RTCC_CS_SetHigh();
}

/************************************************************************/
/* PROCEDURE: CE_Low													*/
/*																		*/
/* This procedure drives the CE of the device to low.  					*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Output:																*/
/*		CE																*/
/*																		*/
/************************************************************************/
void macEeprCE_Low()
{
	RTCC_CS_SetLow();
}


/************************************************************************/
/* PROCEDURE: Read_Status_Register										*/
/*																		*/
/* This procedure read the status register and returns the byte.		*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Returns:																*/
/*		byte															*/
/************************************************************************/
uint8_t macEeprReadStatusRegister()
{
	uint8_t byte = 0;
	macEeprCE_Low();				/* enable device */
	macEeprSendByte(0x05);          /* send RDSR command */
	byte = macEeprGetByte();        /* receive byte */
	macEeprCE_High();			    /* disable device */
	return byte;
}

/************************************************************************/
/* PROCEDURE: WRSR														*/
/*																		*/
/* This procedure writes a byte to the Status Register.					*/
/*																		*/
/* Input:																*/
/*		byte															*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void macEeprWRSR(uint8_t byte)
{
	macEeprCE_Low();				/* enable device */
	macEeprSendByte(0x01);          /* select write to status register */
	macEeprSendByte(byte);          /* data that will be written to status 
                                     * register */
	macEeprCE_High();               /* disable the device */
}

/************************************************************************/
/* PROCEDURE: WRENABLE													*/
/*																		*/
/* This procedure enables the Write Enable Latch.  It can also be used 	*/
/* to Enables Write Status Register.									*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void macEeprWRENABLE()
{
	macEeprCE_Low();             /* enable device */
	macEeprSendByte(0x06);       /* send WREN command */
	macEeprCE_High();            /* disable device */
    NO_OPERATION();
    NO_OPERATION();
}

/************************************************************************/
/* PROCEDURE: WRDI														*/
/*																		*/
/* This procedure disables the Write Enable Latch.						*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void macEeprWRDI()
{
	macEeprCE_Low();				/* enable device */
	macEeprSendByte(0x04);          /* send WRDI command */
	macEeprCE_High();               /* disable device */
}


/************************************************************************/
/* PROCEDURE:	Read													*/
/*																		*/
/* This procedure reads one address of the device.  It will return the 	*/
/* byte read in variable byte.											*/
/*																		*/
/*																		*/
/*																		*/
/* Input:																*/
/*		Dst:	Destination Address 000000H - FFH					*/
/*      																*/
/*																		*/
/* Returns:																*/
/*		byte															*/
/*																		*/
/************************************************************************/
uint8_t macEeprRead(uint8_t Dst)
{
	uint8_t byte = 0, stat_reg;
    
    macEeprWRDI();                                        /* write disable */
    do{
        stat_reg = macEeprReadStatusRegister();           /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);
            
	macEeprCE_Low();                                      /* enable device */
    
	macEeprSendByte(0x03);                                /* read command */
	macEeprSendByte(Dst & 0xFFu);                          /* send address byte */  //jira: CAE_MCU8-5647
	byte = macEeprGetByte();
    
	macEeprCE_High();                                     /* disable device */
    
    macEeprWRENABLE();                                    /* write enable */
    do{
        stat_reg = macEeprReadStatusRegister();           /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x02) != 0x02);
    
	return byte;                                          /* return one byte read */
}

/************************************************************************/
/* PROCEDURE:	Read_Cont												*/
/*																		*/
/* This procedure reads multiple addresses of the device and stores		*/
/* data into 256 byte buffer. Maximum byte that can be read is 256 bytes*/
/*																		*/
/* Input:																*/
/*		Dst:		Destination Address 000000H - FFH   				*/
/*      no_bytes	Number of bytes to read	(max = 256)					*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/*																		*/
/************************************************************************/
void macEeprReadBlock(uint8_t Dst, void *data, uint8_t no_bytes)
{
    uint8_t stat_reg;
    register char *p = (char *) data;
    
    macEeprWRDI();                                         /* write disable */
    do{
        stat_reg = macEeprReadStatusRegister();            /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);
    
	macEeprCE_Low();                                      /* enable device */
	macEeprSendByte(0x03);                                /* read command */
	macEeprSendByte(Dst & 0xFFu);                          /* send address byte */    //jira: CAE_MCU8-5647
	
    while(no_bytes--)
    {
        *p++ = macEeprGetByte();                          /* Read the data */            
            Dst++;    
            if(Dst%16==0)
            {
                 macEeprCE_High();                
                 macEeprWRDI();                           /* Reset the write enable latch */   
                  do
                  {
                      stat_reg = macEeprReadStatusRegister();          /* Read Status Register */   
                  }while(((stat_reg & 0x03) != 0x00));
                  
                  
                  macEeprCE_Low();                                     /* CS low */           
                  macEeprSendByte(0x03);                               /* Write Read instruction  */   
                  macEeprSendByte(Dst & 0xFFu);                         /* send address byte */    //jira: CAE_MCU8-5647
            }   
    }
        
	macEeprCE_High();                                     /* disable device */
    
    macEeprWRENABLE();                                    /* write enable */
    do{
        stat_reg = macEeprReadStatusRegister();           /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x02) != 0x02);

}

/************************************************************************/
/* PROCEDURE:	Byte_Program											*/
/*																		*/
/* This procedure programs one address of the device.					*/
/* Assumption:  Address being programmed is already erased and is NOT	*/
/* block protected.														*/
/*																		*/
/*																		*/
/*																		*/
/* Input:																*/
/*		Dst:		Destination Address 000000H - FFH       			*/
/*		byte:		byte to be programmed								*/
/*      																*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/*																		*/
/************************************************************************/
void macEeprWrite(uint8_t Dst, uint8_t byte)
{
    uint8_t stat_reg;
    macEeprWRENABLE();                                       /* write enable */
    do{
        stat_reg = macEeprReadStatusRegister();              /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x02) != 0x02);
    
	macEeprCE_Low();                                         /* enable device */
	macEeprSendByte(0x02);                                   /* send Byte Program command */
	macEeprSendByte(Dst & 0xFFu);                             /* send address byte */        //jira: CAE_MCU8-5647
	macEeprSendByte(byte);                                   /* send byte to be programmed */
    macEeprWaitBusy();
    macEeprCE_High();                                        /* disable device */
    
    macEeprWRDI();                                           /* write disable */
    do{
        stat_reg = macEeprReadStatusRegister();              /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	
}

/************************************************************************/
/* PROCEDURE:	ATstorageom_writeBlock                                  */
/*                                                                      */
/* This procedure does page programming.  The destination               */
/* address should be provided.                                          */
/* Assumption:  Address being programmed is already erased and is NOT	*/
/*		block protected.                                                */
/* Input:                                                               */
/*		Dst:		Destination Address 000000H - FFH                   */
/************************************************************************/

void macEeprWriteBlock(uint8_t Dst, void *data, uint8_t len)
{
    uint8_t stat_reg;
	unsigned int i;
    register char *p = (char *) data;
    macEeprWRENABLE();
    do{
        stat_reg = macEeprReadStatusRegister();               /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x02) != 0x02);
    
    macEeprCE_Low();                                         /* enable device */
    macEeprSendByte(0x02);                                   /* send Byte Program command */
    macEeprSendByte(Dst & 0xFFu);                             /* send 3 address bytes */       //jira: CAE_MCU8-5647

        
	for (i=0;i<len;i++)
	{        
        macEeprSendByte(*p++);                               /* send byte to be programmed */

        Dst++;
        if(Dst%16==0)
        {
            macEeprCE_High();
            DELAY_MS(MACeepromDelay_10ms);
            DELAY_MS(MACeepromDelay_10ms);

            macEeprWRENABLE();
            do{
                stat_reg = macEeprReadStatusRegister();       /* read status register */
//                printf("stat_reg = %d \r\n", stat_reg);
            }while((stat_reg &0x02) != 0x02);
            
            macEeprCE_Low();    
            macEeprSendByte(0x02);                          /* send Byte Program command */
            macEeprSendByte(Dst & 0xFFu);                    /* send 3 address bytes */          //jira: CAE_MCU8-5647
        }

	}    
    macEeprCE_High();                                       /* disable device */
    
    macEeprWRDI();                                          /* write disable */
    do{
        stat_reg = macEeprReadStatusRegister();             /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	
	
}

/************************************************************************/
/* PROCEDURE: Chip_Erase                                                */
/*                                                                      */
/* This procedure erases the entire Chip.                               */
/************************************************************************/
void macEeprErase()
{
    uint8_t Dst = 0x72;                       
//    uint16_t len = 32768;
    uint8_t len = 135;                           /* 87H bytes */
    uint8_t stat_reg;      
//    printf("Erasing EEPROM from address %lu \r\n", Dst);
    macEeprWRENABLE();
    do{
        stat_reg = macEeprReadStatusRegister();               /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x02) != 0x02);
    
    macEeprCE_Low();                                         /* enable device */
    macEeprSendByte(0x02);                                   /* send Byte Program command */
    macEeprSendByte(Dst & 0xFFu);                             /* send address byte */        //jira: CAE_MCU8-5647

    while(len--)
    {
        macEeprSendByte(0xFF);                                  /* send byte to be programmed */
        Dst++;    
        if(Dst%16==0)                                         /* Next page 1st byte write */
        {
            macEeprCE_High();
            DELAY_MS(MACeepromDelay_10ms);
            DELAY_MS(MACeepromDelay_10ms);
            macEeprWRENABLE();
            do{
                stat_reg = macEeprReadStatusRegister();       /* read status register */
//                printf("stat_reg = %d \r\n", stat_reg);
            }while((stat_reg &0x02) != 0x02);
             
             macEeprCE_Low();                                 /* enable device */
             macEeprSendByte(0x02);                           /* send Byte Program command */
             macEeprSendByte(Dst & 0xFFu);                     /* send address byte */       //jira: CAE_MCU8-5647
             
        }
    }
    macEeprCE_High();
    
    macEeprWRDI();                                          /* write disable */
    do{
        stat_reg = macEeprReadStatusRegister();             /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	     
//    printf("Erasing macEepr complete \r\n");
    
}

/************************************************************************/
/* PROCEDURE: Wait_Busy													*/
/*																		*/
/* This procedure waits until device is no longer busy (can be used by	*/
/* Byte-Program, Sector-Erase, Block-Erase, Chip-Erase).				*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void macEeprWaitBusy()
{
	while ((macEeprReadStatusRegister()& 0x01) == 0x01)	// waste time until not busy
		macEeprReadStatusRegister();
//    DELAY_MS(26);
}

/************************************************************************/
/* PROCEDURE: WREN_Check												*/
/*																		*/
/* This procedure checks to see if WEL bit set before program/erase.	*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void macEeprWREN_check()
{
	uint8_t byte;
	byte = macEeprReadStatusRegister();	/* read the status register */
	if (byte != 0x02)                       /* verify that WEL bit is set */
	{
		while(1)
        {
//            printf("Write Enable bit not set\r\n");
        }
    }
}

/************************************************************************/
/* PROCEDURE: Verify													*/
/*																		*/
/* This procedure checks to see if the correct byte has be read.		*/
/*																		*/
/* Input:																*/
/*		byte:		byte read											*/
/*		cor_byte:	correct_byte that should be read					*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void macEeprverify(uint8_t byte, uint8_t cor_byte)
{
	if (byte != cor_byte)
	{
		while(1)
        {
//            printf("Correct byte not read\r\n");
        }
	}
}


/************************************************************************/
/* PROCEDURE: e48_read_mac                                          	*/
/*																		*/
/* This procedure reads the MAC address.                        		*/
/*																		*/
/* Input:																*/
/*		mac:		pointer to the array to store MAC address			*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
void e48_read_mac(uint8_t *mac)
{
    mac48Address_t mac48add;
    uint8_t x;
    
    
    RTCC_CS_SetLow();                               /* CS low */      
    MAC_EEPROM_SPI_WRITE(MAC_EE_READ);                         /* read command */
    MAC_EEPROM_SPI_WRITE(MAC_ADDR_48);                         /* Write Unique ID address to read data from */ 


    for(x=0; x<mac_48_bytes; x++)         //jira: CAE_MCU8-5898
    {
//        printf("x=%d\r\n",x);
        mac48add.mac_array[x] = MAC_EEPROM_SPI_READ();         /* Read Data */         
    }

    RTCC_CS_SetHigh();  
    
    
//    printf("macAddr:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n",mac48add.mac_array[0],mac48add.mac_array[1],mac48add.mac_array[2],mac48add.mac_array[3],mac48add.mac_array[4],mac48add.mac_array[5]);
    
    for(x=0; x<mac_48_bytes; x++)       //jira: CAE_MCU8-5898
    {
        mac[x] = mac48add.mac_array[x];
    }
    
}







