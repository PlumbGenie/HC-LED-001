/**
  CoAP Protocol Header file

  Company:
    Microchip Technology Inc.

  File Name:
    coap.h

  Summary:
    Header file for CoAP protocol implementation.

  Description:
    This header file provides the API for the CoAP protocol.

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

#ifndef COAP_OPTIONS_H
#define COAP_OPTIONS_H



extern bool coapObserveOptionRecvd;

/**Create a new Option table for each endpoint (if options are different)
 * Make sure the options are sorted and placed in the table.
 * Options sorting is not implemented -  have to be implemented if required
 * If possible have to move this table to a new file - user configurable file
 */
void COAP_AddWellKnownURIPathOptions(void);
void COAP_AddBlock2Options(char *str);
void COAP_AddAcceptOption(content_format_t acceptFormat);
void COAP_AddContentFormatOption(content_format_t contentFormat);
void COAP_AddURIPathOption(const char  *uriPathStr);
void COAP_AddURIQueryOption(const char *uriQueryStr);

void COAP_ResetLastOptionNumber(void);
uint8_t COAP_GetNumberOfOptionsAdded(void);
uint8_t COAP_GetLengthOfIntegerOptionValue(uint32_t val);

#endif	/* COAP_OPTIONS_H */

