/**
 PWM API Header File

  Company:
    Microchip Technology Inc.

  File Name:
   pwm.h

  Summary:
    Header file for the PWM implementation.

  Description:
    This header file provides APIs for the PWM implementation

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


/* 
 * File:   pwm.h
 * Author: c14210
 *
 * Created on October 23, 2014, 11:50 AM
 */

#ifndef PWM_H
#define	PWM_H

#include <stdint.h>

#define V48EN       LATFbits.LATF6
#define V48EN_T     TRISFbits.TRISF6

#define LIGHT_ON 1000U
#define LIGHT_OFF 0

extern uint16_t target_power_level_pwm[2];
extern uint16_t current_power_level_pwm[2];

void pwm_init(void);
void pwm1_set(uint16_t p);
void pwm2_set(uint16_t p);
void pwm1_dim(void);
void pwm2_dim(void);
void pwm_setLimit(uint16_t limit);

//void pwm_isr(void);

#endif	/* PWM_H */

