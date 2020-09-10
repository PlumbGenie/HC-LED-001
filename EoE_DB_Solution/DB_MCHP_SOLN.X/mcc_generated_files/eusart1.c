/**
  EUSART1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    eusart1.c

  @Summary
    This is the generated driver implementation file for the EUSART1 driver using Foundation Services Library

  @Description
    This header file provides implementations for driver APIs for EUSART1.
    Generation Information :
        Product Revision  :  Foundation Services Library - 0.1.21
        Device            :  PIC18F67J60
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "eusart1.h"

/**
  Section: EUSART1 APIs
*/

void EUSART1_Initialize(void)
{
    // Set the EUSART1 module to the options selected in the user interface.

    // ABDOVF no_overflow; SCKP async_noninverted_sync_fallingedge; BRG16 16bit_generator; WUE disabled; ABDEN disabled; RXDTP not_inverted; 
    BAUDCON1 = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
    RCSTA1 = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave_mode; 
    TXSTA1 = 0x24;

    // Baud Rate = 115740.742; 
    SPBRG1 = 0x59;

    // Baud Rate = 115740.742; 
    SPBRGH1 = 0x00;

}


bool EUSART1_is_tx_ready(void)
{
    return (bool)(PIR1bits.TX1IF && TXSTA1bits.TXEN);    //jira:CAE_MCU8-5647
}

bool EUSART1_is_rx_ready(void)
{
    return PIR1bits.RC1IF;
}

bool EUSART1_is_tx_done(void)
{
    return TXSTA1bits.TRMT;
}

uint8_t EUSART1_Read(void)
{

    while(!PIR1bits.RC1IF)
    {
    }

    
    if(1 == RCSTA1bits.OERR)
    {
        // EUSART1 error - restart

        RCSTA1bits.SPEN = 0; 
        RCSTA1bits.SPEN = 1; 
    }

    return RCREG1;
}

void EUSART1_Write(uint8_t txData)
{
    while(0 == PIR1bits.TX1IF)
    {
    }

    TXREG1 = txData;    // Write the data byte to the USART.
}

char getch(void)
{
    return EUSART1_Read();
}

void putch(char txData)
{
    EUSART1_Write(txData);
}
/**
  End of File
*/
