/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef COAP_OBSERVE_COMMON_H
#define	COAP_OBSERVE_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include "coap.h"




extern bool isFreshestNotification;

int16_t COAP_ProcessObserveOption(coapReceive_t *ptr);
void COAP_setObserveOptionValue(char *value,uint16_t observeOptionLength);
bool COAP_IsAddObserveOptionFlag();
uint32_t COAP_getObserveOptionValue();
void COAP_NotifyObservers(void);
void COAP_ObserveNotifyTransmissions(void);
void COAP_UpdateObserveOptionFlag(bool flag);

error_msg COAP_RegisterResource(char *resourceAssoicatedWith,coapTransmit_t *ocp);
error_msg COAP_DeRegisterResource(char *resourceAssoicatedWith,coapTransmit_t *ocp);

void COAP_RemoveObservedFromList(int16_t observedVertexIndex, uint32_t observerIPAddress);
bool COAP_IsFreshestNotification(int16_t observingObjIdx);
#endif	/* COAP_OBSERVE_COMMON_H */

