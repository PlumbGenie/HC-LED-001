/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    eeprom.c

  Summary:
    EEPROM driver for AT25M02

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

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

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "eeprom.h"
#include "spi1.h"
#ifdef __XC8
#include "hardware.h"
#else
#include "hardware_unix.h"
#endif

const uint8_t eepromDelay_10ms = 10;

static void    storageWP_Low(void);
static uint8_t storageReadStatusRegister(void);
//static void    storageEWSR(void);                        //jira: CAE_MCU8-5647
//static void    storageWRSR(uint8_t byte);                //jira: CAE_MCU8-5647
static void    storageWRENABLE(void);
static void    storageWRDI(void);
static void    storageWREN_Check(void);

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
void storageInit()
{
    STORAGE_CE_SetHigh();    
}
#define storagesendByte(out)    SPI1_Exchange8bit(out)
#define storagegetByte()        SPI1_Exchange8bit(0)

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
static uint8_t storageReadStatusRegister()
{
	uint8_t byte = 0;
	STORAGE_CE_SetLow();
	storagesendByte(0x05);	    /* send RDSR command */
	byte = storagegetByte();      /* receive byte */
	STORAGE_CE_SetHigh();
	return byte;
}

/************************************************************************/
/* PROCEDURE: EWSR														*/
/*																		*/
/* This procedure Enables Write Status Register.  						*/
/*																		*/
/* Input:																*/
/*		None															*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/************************************************************************/
//jira: CAE_MCU8-5647
//static void storageEWSR()
//{
//	STORAGE_CE_SetLow();
//	storagesendByte(0x06);		/* enable writing to the status register */
//	STORAGE_CE_SetHigh();
//}

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
//jira: CAE_MCU8-5647
//static void storageWRSR(uint8_t byte)
//{
//	STORAGE_CE_SetLow();				/* enable device */
//	storagesendByte(0x01);		/* select write to status register */
//	storagesendByte(byte);		/* data that will be written to status 
//                                 * register */
//	STORAGE_CE_SetHigh();            /* disable the device */
//}

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
static void storageWRENABLE()
{
	STORAGE_CE_SetLow();             /* enable device */
	storagesendByte(0x06);       /* send WREN command */
	STORAGE_CE_SetHigh();            /* disable device */
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
static void storageWRDI()
{
	STORAGE_CE_SetLow();				/* enable device */
	storagesendByte(0x04);		/* send WRDI command */
	STORAGE_CE_SetHigh();             /* disable device */
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
/*		Dst:	Destination Address 000000H - 3FFFFH					*/
/*      																*/
/*																		*/
/* Returns:																*/
/*		byte															*/
/*																		*/
/************************************************************************/
uint8_t storageRead(uint32_t Dst)
{
	uint8_t byte = 0, stat_reg;
    
    storageWRDI();                                        /* write disable */
    do{
        stat_reg = storageReadStatusRegister();           /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);
            
	STORAGE_CE_SetLow();                                      /* enable device */
    
	storagesendByte(0x03);                                /* read command */
	storagesendByte(((Dst & 0xFFFFFF) >> 16));            /* send 3 address bytes */
	storagesendByte(((Dst & 0xFFFF) >> 8));
	storagesendByte(Dst & 0xFF);
	byte = storagegetByte();
    
	STORAGE_CE_SetHigh();                                     /* disable device */
    
    storageWRENABLE();                                    /* write enable */
    do{
        stat_reg = storageReadStatusRegister();           /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x82) != 0x02);
    
	return byte;                                        /* return one byte read */
}

/************************************************************************/
/* PROCEDURE:	Read_Cont												*/
/*																		*/
/* This procedure reads multiple addresses of the device and stores		*/
/* data into 256 byte buffer. Maximum byte that can be read is 256 bytes*/
/*																		*/
/* Input:																*/
/*		Dst:		Destination Address 000000H - 3FFFFH				*/
/*      no_bytes	Number of bytes to read	(max = 256)					*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/*																		*/
/************************************************************************/
void storageReadBlock(uint32_t Dst, void *data, uint16_t no_bytes)
{
    uint8_t stat_reg;
    register char *p = (char *) data;
	//unsigned long i = 0;
    
    storageWRDI();                                         /* write disable */
    do{
        stat_reg = storageReadStatusRegister();            /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);
    
	STORAGE_CE_SetLow();                                      /* enable device */
	storagesendByte(0x03);                                /* read command */
	storagesendByte(((Dst & 0xFFFFFF) >> 16));            /* send 3 address bytes */
	storagesendByte(((Dst & 0xFFFF) >> 8));
	storagesendByte(Dst & 0xFF);
	
    while(no_bytes--)
    {
        *p++ = storagegetByte();         //Read the data           
            Dst++;    
            if(Dst%256==0)
            {
                 STORAGE_CE_SetHigh();                
                 storageWRDI();                 //Reset the write enable latch
                  do
                  {
                      stat_reg = storageReadStatusRegister();  //Read Status Register
                  }while(((stat_reg & 0x03) != 0x00));
                  
                  
                  STORAGE_CE_SetLow();                   //CS low
                  storagesendByte(0x03);            //Write Read instruction
                  storagesendByte(((Dst & 0xFFFFFF) >> 16));               /* send 3 address bytes */
                  storagesendByte(((Dst & 0xFFFF) >> 8));
                  storagesendByte(Dst & 0xFF);      
            }   
    }
    
//	for (i = 0; i < no_bytes; i++)                      /* read until no_bytes is reached */
//	{
//        p[i] = storagegetByte();
//	}
    
	STORAGE_CE_SetHigh();                                     /* disable device */
    
    storageWRENABLE();                                    /* write enable */
//    SST26_Wait_Busy();
    do{
        stat_reg = storageReadStatusRegister();           /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x82) != 0x02);

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
/*		Dst:		Destination Address 000000H - 3FFFFH				*/
/*		byte:		byte to be programmed								*/
/*      																*/
/*																		*/
/* Returns:																*/
/*		Nothing															*/
/*																		*/
/************************************************************************/
void storageWrite(uint32_t Dst, uint8_t byte)
{
    uint8_t stat_reg;
    storageWRENABLE();                                       /* write enable */
    do{
        stat_reg = storageReadStatusRegister();              /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x82) != 0x02);
    
	STORAGE_CE_SetLow();                                         /* enable device */
	storagesendByte(0x02);                                   /* send Byte Program command */
	storagesendByte(((Dst & 0xFFFFFF) >> 16));               /* send 3 address bytes */
	storagesendByte(((Dst & 0xFFFF) >> 8));
	storagesendByte(Dst & 0xFF);
	storagesendByte(byte);                                   /* send byte to be programmed */
    storagewaitBusy();
    STORAGE_CE_SetHigh();                                        /* disable device */
    
    storageWRDI();                                           /* write disable */
    do{
        stat_reg = storageReadStatusRegister();              /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	
}

/************************************************************************/
/* PROCEDURE:	ATstorageom_writeBlock                                           */
/*                                                                      */
/* This procedure does page programming.  The destination               */
/* address should be provided.                                          */
/* Assumption:  Address being programmed is already erased and is NOT	*/
/*		block protected.                                                */
/* Input:                                                               */
/*		Dst:		Destination Address 000000H - 3FFFFH               */
/************************************************************************/

void storageWriteBlock(uint32_t Dst, void *data, uint16_t len)
{
    uint8_t stat_reg;
	unsigned int i;
    register char *p = (char *) data;
    storageWRENABLE();
    do{
        stat_reg = storageReadStatusRegister();               /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x82) != 0x02);
    
    STORAGE_CE_SetLow();                                         /* enable device */
    storagesendByte(0x02);                                   /* send Byte Program command */
    storagesendByte(((Dst & 0xFFFFFF) >> 16));               /* send 3 address bytes */
    storagesendByte(((Dst & 0xFFFF) >> 8));
    storagesendByte(Dst & 0xFF);

        
	for (i=0;i<len;i++)
	{        
        storagesendByte(*p++);                               /* send byte to be programmed */

        Dst++;
        if(Dst%256==0)
        {
            STORAGE_CE_SetHigh();
            DELAY_MS(eepromDelay_10ms);
            DELAY_MS(eepromDelay_10ms);

            storageWRENABLE();
            do{
                stat_reg = storageReadStatusRegister();       /* read status register */
            }while((stat_reg &0x82) != 0x02);
            
            STORAGE_CE_SetLow();    
            storagesendByte(0x02);                          /* send Byte Program command */
            storagesendByte(((Dst & 0xFFFFFF) >> 16));      /* send 3 address bytes */
            storagesendByte(((Dst & 0xFFFF) >> 8));
            storagesendByte(Dst & 0xFF);
        }

	}    
    STORAGE_CE_SetHigh();                                       /* disable device */
    
    storageWRDI();                                          /* write disable */
    do{
        stat_reg = storageReadStatusRegister();             /* read status register */
    }while((stat_reg &0x03) != 0x00);	
	
}

/************************************************************************/
/* PROCEDURE: Chip_Erase                                                */
/*                                                                      */
/* This procedure erases the entire Chip.                               */
/************************************************************************/
void storageBootImageErase()
{
    uint32_t Dst = 0x00020000;                       
//    uint16_t len = 32768;
    uint32_t len = 131071;                           /* 3FFFH bytes */
    uint8_t stat_reg;      
    storageWRENABLE();
    do{
        stat_reg = storageReadStatusRegister();               /* read status register */
    }while((stat_reg &0x82) != 0x02);
    
    STORAGE_CE_SetLow();                                         /* enable device */
    storagesendByte(0x02);                                   /* send Byte Program command */
    storagesendByte(((Dst & 0xFFFFFF) >> 16));               /* send 3 address bytes */
    storagesendByte(((Dst & 0xFFFF) >> 8));
    storagesendByte(Dst & 0xFF);

    while(len--)
    {
        storagesendByte(0xFF);                                  /* send byte to be programmed */
        Dst++;    
        if(Dst%256==0)                                         /* Next page 1st byte write */
        {
            STORAGE_CE_SetHigh();
            DELAY_MS(eepromDelay_10ms);
            DELAY_MS(eepromDelay_10ms);
            storageWRENABLE();
            do{
                stat_reg = storageReadStatusRegister();       /* read status register */
            }while((stat_reg &0x82) != 0x02);
             
             STORAGE_CE_SetLow();                                 /* enable device */
             storagesendByte(0x02);                           /* send Byte Program command */
             storagesendByte(((Dst & 0xFFFFFF) >> 16));       /* send 3 address bytes */
             storagesendByte(((Dst & 0xFFFF) >> 8));
             storagesendByte(Dst & 0xFF);
             
        }
    }
    STORAGE_CE_SetHigh();
    
    storageWRDI();                                          /* write disable */
    do{
        stat_reg = storageReadStatusRegister();             /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	     
//    printf("Erasing storage complete \r\n");
    
}

/************************************************************************/
/* PROCEDURE: Chip_Erase                                                */
/*                                                                      */
/* This procedure erases the entire Chip.                               */
/************************************************************************/
void storageDatabaseErase()
{
    uint32_t Dst = 0x00000000;                       
//    uint16_t len = 32768;
    uint32_t len = STORAGE_ERASE_SIZE;                           /* 3FFFFH bytes */
    uint8_t stat_reg;      
//    printf("Erasing EEPROM from address %lu \r\n", Dst);
    storageWRENABLE();
    do{
        stat_reg = storageReadStatusRegister();               /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x82) != 0x02);
    
    STORAGE_CE_SetLow();                                         /* enable device */
    storagesendByte(0x02);                                   /* send Byte Program command */
    storagesendByte(((Dst & 0xFFFFFF) >> 16));               /* send 3 address bytes */
    storagesendByte(((Dst & 0xFFFF) >> 8));
    storagesendByte(Dst & 0xFF);

    while(len--)
    {
        storagesendByte(0xFF);                                  /* send byte to be programmed */
        Dst++;    
        if(Dst%256==0)                                         /* Next page 1st byte write */
        {
            STORAGE_CE_SetHigh();
            DELAY_MS(eepromDelay_10ms);
            DELAY_MS(eepromDelay_10ms);
            storageWRENABLE();
            do{
                stat_reg = storageReadStatusRegister();       /* read status register */
//                printf("stat_reg = %d \r\n", stat_reg);
            }while((stat_reg &0x82) != 0x02);
             
             STORAGE_CE_SetLow();                                 /* enable device */
             storagesendByte(0x02);                           /* send Byte Program command */
             storagesendByte(((Dst & 0xFFFFFF) >> 16));       /* send 3 address bytes */
             storagesendByte(((Dst & 0xFFFF) >> 8));
             storagesendByte(Dst & 0xFF);
             
        }
    }
    STORAGE_CE_SetHigh();
    
    storageWRDI();                                          /* write disable */
    do{
        stat_reg = storageReadStatusRegister();             /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	     
//    printf("Erasing storage complete \r\n");
    
}

/************************************************************************/
/* PROCEDURE: Chip_Erase                                                */
/*                                                                      */
/* This procedure erases the entire Chip.                               */
/************************************************************************/
void storageDtlsHandshakeErase()
{
    uint32_t Dst = DTLS_START_ADDRESS;                       
    uint32_t len = DTLS_ERASE_SIZE;                          
    uint8_t stat_reg;      
    storageWRENABLE();
    do{
        stat_reg = storageReadStatusRegister();               /* read status register */
    }while((stat_reg &0x82) != 0x02);
    
    STORAGE_CE_SetLow();                                         /* enable device */
    storagesendByte(0x02);                                   /* send Byte Program command */
    storagesendByte(((Dst & 0xFFFFFF) >> 16));               /* send 3 address bytes */
    storagesendByte(((Dst & 0xFFFF) >> 8));
    storagesendByte(Dst & 0xFF);

    while(len--)
    {
        storagesendByte(0xFF);                                  /* send byte to be programmed */
        Dst++;    
        if(Dst%256==0)                                         /* Next page 1st byte write */
        {
            STORAGE_CE_SetHigh();
            DELAY_MS(10);
            DELAY_MS(10);
            storageWRENABLE();
            do{
                stat_reg = storageReadStatusRegister();       /* read status register */
            }while((stat_reg &0x82) != 0x02);
             
             STORAGE_CE_SetLow();                                 /* enable device */
             storagesendByte(0x02);                           /* send Byte Program command */
             storagesendByte(((Dst & 0xFFFFFF) >> 16));       /* send 3 address bytes */
             storagesendByte(((Dst & 0xFFFF) >> 8));
             storagesendByte(Dst & 0xFF);
             
        }
    }
    STORAGE_CE_SetHigh();
    
    storageWRDI();                                          /* write disable */
    do{
        stat_reg = storageReadStatusRegister();             /* read status register */
//        printf("stat_reg = %d \r\n", stat_reg);
    }while((stat_reg &0x03) != 0x00);	     
//    printf("Erasing storage complete \r\n");
    
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
void storagewaitBusy()
{
	while ((storageReadStatusRegister()& 0x01) == 0x01)	// waste time until not busy
		storageReadStatusRegister();
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
void storageWREN_check()
{
	uint8_t byte;
	byte = storageReadStatusRegister();	/* read the status register */
	if (byte != 0x02)                       /* verify that WEL bit is set */
	{
		while(1)
        {
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
void storageverify(uint8_t byte, uint8_t cor_byte)
{
	if (byte != cor_byte)
	{
		while(1)
        {
        }
	}
}

/************************************************************************/
/* PROCEDURE: ATstorageom_checksum                                             */
/*																		*/
/* This procedure calculates checksum over the specified address range.	*/
/*																		*/
/* Input:																*/
/*		Dst_Start:		Start Address									*/
/*      Dst_End  :      End Address                                     */
/*																		*/
/* Returns:																*/
/*		checksum                                                        */
/************************************************************************/

uint16_t storageChecksum (uint32_t Dst_Start,uint32_t Dst_End)
{
    uint8_t byte = 0;
    uint16_t checksum = 0;
    
    while(Dst_Start <= Dst_End)
    {
        byte = storageRead(Dst_Start);
        checksum += byte;
        Dst_Start += sizeof(byte);
    }    
    return checksum;
}

