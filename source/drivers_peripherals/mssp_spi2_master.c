/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    mssp_spi_master.c

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

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
#include "hardware.h"
#include "mssp_spi2_master.h"


//void spi_init(void)
//{
//    SSP1STATbits.CKE = 1;
//    SSP1STATbits.SMP = 0;    
//
//    SSP1CON1 = 0x00;// SPI Master mode, Clock = FOSC/4
//    SSP1CON1bits.SSPEN = 1;
//}
//
//char SPI_ExchangeByte(char v)
//{
//    SSP1BUF = v;
//    while (SSP1STATbits.BF == 0);
//    return SSPBUF;
//}

void spi2_init(void) {
    SSP2STATbits.CKE = 1;
    SSP2STATbits.SMP = 1;

    SSP2CON1 = 0x02; // SPI Master mode, Clock = FOSC/64
    SSP2CON1bits.SSPEN= 1;
}

char SPI2_ExchangeByte(char v) {
    SSP2BUF = v;
//    while (SSP2STATbits.BF == 0);
//    while(!PIR3bits.SSP2IF);
    DELAY_US(15);
    PIR3bits.SSP2IF=0;
    NOP();
    NOP();
    NOP();
    NOP();
    return SSP2BUF;
}
