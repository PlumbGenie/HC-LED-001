/**
  EUSART1 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    eusart1.h

  @Summary
    This is the generated header file for the EUSART1 driver using Foundation Services Library

  @Description
    This header file provides APIs for driver for EUSART1.
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

#ifndef _EUSART1_H
#define _EUSART1_H

/**
  Section: Included Files
*/

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif


/**
  Section: Macro Declarations
*/

/**
  Section: EUSART1 APIs
*/

/**
 * @brief Initialization routine that takes inputs from the EUSART1 GUI.
 *
 */
void EUSART1_Initialize(void);

/**
 * @brief Check if the EUSART1 transmitter is empty
 *
 * @return The status of EUSART1 TX empty checking.
 * @retval false the EUSART1 transmitter is not empty
 * @retval true the EUSART1 transmitter is empty
 */
bool EUSART1_is_tx_ready(void);

/**
 * @brief Check if the EUSART1 receiver is not empty
 *
 * @return The status of EUSART1 RX empty checking.
 * @retval false the EUSART1 receiver is empty
 * @retval true the EUSART1 receiver is not empty
 */
bool EUSART1_is_rx_ready(void);

/**
 * @brief Check if EUSART1 data is transmitted
 *
 * @return Receiver ready status
 * @retval false  Data is not completely shifted out of the shift register
 * @retval true   Data completely shifted out if the USART shift register
 */
bool EUSART1_is_tx_done(void);

/**
 * @brief Read one character from EUSART1
 *
 * @return Data read from the EUSART1 module
 */
uint8_t EUSART1_Read(void);

/**
 * @brief Write one character on EUSART1
 *
 * @param[in] data The character to write to the EUSART1 bus
 *
 * @return Nothing
 */
void EUSART1_Write(uint8_t txData);

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif  // _EUSART1_H
/**
 End of File
*/
