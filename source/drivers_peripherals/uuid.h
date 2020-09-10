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
 * File: uuid.h   
 * Comments: From RFC 4122
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef UUID_H
#define	UUID_H

#define SOURCEPORT_UUID     65530u   //jira: CAE_MCU8-5647
#define DESTPORT_UUID       65532u   //jira: CAE_MCU8-5647


// Seconds from 15th Oct 1582 to 1st Jan 1970 are 12219292800
// This is equal to 122192928000000000 intervals in terms of 100ns intervals
#define uuid_base_time_diff_lo      0x1B21DD2 
#define uuid_base_time_diff_hi      0x13814000
//#define uuid_base_time_diff_lo      0xD8539C80 /*Not considering the 10^7 factor since 64 bit int cannot be stored on PIC18F*/
//#define uuid_base_time_diff_hi      0x00000002 /*Not considering the 10^7 factor since 64 bit int cannot be stored on PIC18F*/
#define uuid_version1_mask          (1<<12) /*Currently using version 1 according to RFC4122*/

#define COMPARE_CHECK(f1, f2) if (f1 != f2) return f1 < f2 ? -1 : 1;

#define UUID_STR_SIZE               37
#define UUID_SUBSTR1_SIZE           32
#define UUID_SUBSTR2_SIZE           4
#define UUID_CREATED                0xAA

typedef struct {
    uint32_t lo;
    uint32_t hi;
    uint8_t  clk_seq_high_and_version;
    uint8_t  clk_seq_low;
} mchp_uuid_time_clk_seq_t;

typedef struct {
    uint8_t dev_Num;
} uuid_assign_device_ID;

typedef struct {
  uint8_t nodeID[6];
} uuid_node_t;

typedef struct
{
   uint32_t n1;
   uint32_t n2;
}uint64_t;

typedef struct {
    uint32_t                time_low;
    uint16_t                time_mid;
    uint16_t                time_hi_and_version;
    uint8_t                 clock_seq_hi_and_reserved;    
    uint8_t                 clock_seq_low;
    uint8_t                 node[6];
} mchp_uuid_t;



/*******************************************Function prototypes ********************************************/

/*Compare two UUID's (newly generated UUID and the UUID stored previously in EEPROM) "lexically"  
 *returns -1   u1 is lexically before u2
           0   u1 is equal to u2
           1   u1 is lexically after u2*/
uint8_t uuid_compare(mchp_uuid_t *u1, mchp_uuid_t *u2);       


/*read UUID from eeprom and send it using UDP*/
mchp_uuid_t uuid_eeprm_Read_and_Send(void);


/*read UUID from eeprom */
void uuid_eeprm_Read(char uuid[UUID_STR_SIZE]);


/*write UUID to eeprom only if it is not the same as the one already stored in eeprom*/
void uuid_eeprm_Write(char *uuid);


/* Check whether a UUID has been already generated. If it has been generated, 
 read it from the EEPROM. If not, generate a new UUID. */
void uuidValidate(char *uuid_str);


/*generate version 1 UUID */
void uuid_create(mchp_uuid_t *uuid); 



#endif	/* UUID_HEADER_H */

