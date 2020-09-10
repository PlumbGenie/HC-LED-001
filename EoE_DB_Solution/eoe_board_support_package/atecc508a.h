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
#ifndef ATECC508A_H
#define	ATECC508A_H

#include <xc.h> 
#include <stdint.h>
#include <stdbool.h>

#define CL_ECCX08A_PRIV_KEY_ID                              (0)  
#define CL_ECCX08A_PUB_KEY_ID                               (9)
#define CL_ECCX08A_SERVER_PUB_KEY_ID                        (11)
#define CL_ECCX08A_CERTIFICATE_ID
#define CL_ECCX08A_SERVER_CERTIFICATE_ID

#define CL_ECCX08A_PRIV_KEY_SLOTCONFIG_BLOCK                (0)
#define CL_ECCX08A_PUB_KEY_SLOTCONFIG_BLOCK                 (1)
#define CL_ECCX08A_SERVER_PUB_KEY_SLOTCONFIG_BLOCK          (1)
#define CL_ECCX08A_CERTIFICATE_SLOTCONFIG_BLOCK
#define CL_ECCX08A_SERVER_CERTIFICATE_SLOTCONFIG_BLOCK

#define CL_ECCX08A_PRIV_KEY_SLOTCONFIG_OFFSET               (5)
#define CL_ECCX08A_PUB_KEY_SLOTCONFIG_OFFSET                (1)
#define CL_ECCX08A_SERVER_PUB_KEY_SLOTCONFIG_OFFSET         (2)
#define CL_ECCX08A_CERTIFICATE_SLOTCONFIG_OFFSET
#define CL_ECCX08A_SERVER_CERTIFICATE_KEYCONFIG_OFFSET


#define CL_ECCX08A_PRIV_KEY_SLOTCONFIG                      (0x8F208F20)
#define CL_ECCX08A_PUB_KEY_SLOTCONFIG                       (0x00000F0F)
#define CL_ECCX08A_SERVER_PUB_KEY_SLOTCONFIG                (0x00000F0F)
#define CL_ECCX08A_CERTIFICATE_SLOTCONFIG
#define CL_ECCX08A_SERVER_CERTIFICATE_SLOTCONFIG

#define CL_ECCX08A_PRIV_KEY_KEYCONFIG_BLOCK                 (3)
#define CL_ECCX08A_PUB_KEY_KEYCONFIG_BLOCK                  (3)
#define CL_ECCX08A_SERVER_PUB_KEY_KEYCONFIG_BLOCK           (3)
#define CL_ECCX08A_CERTIFICATE_KEYCONFIG_BLOCK
#define CL_ECCX08A_SERVER_CERTIFICATE_KEYCONFIG_BLOCK

#define CL_ECCX08A_PRIV_KEY_KEYCONFIG_OFFSET                (0)
#define CL_ECCX08A_PUB_KEY_KEYCONFIG_OFFSET                 (4)
#define CL_ECCX08A_SERVER_PUB_KEY_KEYCONFIG_OFFSET          (5)
#define CL_ECCX08A_CERTIFICATE_KEYCONFIG_OFFSET
#define CL_ECCX08A_SERVER_CERTIFICATE_KEYCONFIG_OFFSET


#define CL_ECCX08A_PRIV_KEY_KEYCONFIG                       (0x33003300)
#define CL_ECCX08A_PUB_KEY_KEYCONFIG                        (0x3C003C00)
#define CL_ECCX08A_SERVER_PUB_KEY_KEYCONFIG                 (0x3C003C00)
#define CL_ECCX08A_CERTIFICATE_KEYCONFIG
#define CL_ECCX08A_SERVER_CERTIFICATE_KEYCONFIG

//#define LOCK_CONFIG_ENABLE  
//#define LOCK_DATA_ENABLE
#define VERIFY_STORED_MODE

void    ECCX08A_Init(void);
uint8_t ECCX08A_SlotConfig(uint8_t slot,uint8_t block, uint8_t offset,uint32_t data);
uint8_t ECCX08A_KeyConfig(uint8_t slot,uint8_t block, uint8_t offset,uint32_t data);
uint8_t ECCX08A_IsZoneLocked(uint8_t zone);
uint8_t ECCX08A_LockConfigZone(void);
uint8_t ECCX08A_LockDataOTPZone(void);
uint8_t ECCX08A_LockDataSlot(uint8_t slot);
uint8_t ECCX08A_GenPrivKey(uint8_t keyID); 
uint8_t ECCX08A_GenPubKey(uint8_t privslot, uint8_t * pubkey);
uint8_t ECCX08A_WritePubKey(uint8_t slot, uint8_t *pubkey);
uint8_t ECCX08A_ReadPubKey(uint8_t slot, uint8_t *pubkey);
uint8_t ECCX08A_GetAddress(uint8_t zone, uint8_t slot, uint8_t block, uint8_t offset, uint16_t* addr);
uint8_t ECCX08A_GenSharedSecretKey(uint8_t keyID,uint8_t * serverpubKey, uint8_t *sharedKey);
uint8_t SHA_Start();
uint8_t SHA_Update(uint8_t *message, uint8_t length);
uint8_t SHA_End(uint8_t *message, uint8_t *digest, uint8_t length);
uint8_t SHA_HMACStart(void);
uint8_t SHA_PublicUpdate(void);
uint8_t SHA_HMACEnd(uint8_t *message, uint8_t *digest, uint8_t length);
uint8_t ECCX08A_SignExternalModeCommand(uint8_t keyId, uint8_t *signature);
uint8_t ECCX08A_VerifyCommand(uint8_t *Rcomponent,  uint8_t rlen ,uint8_t *Scomponent, uint8_t slen);
uint8_t ECCx08A_RandomCommand(uint8_t *randomNumber);
uint8_t ECCX08A_CheckResponseStatus(uint8_t ret_code, uint8_t *response);

extern uint8_t PublicKey[64];

/********Debug APIs**********/
void    readConfig();
uint8_t ECCx08A_InfoCommand(void);
uint8_t ECCX08A_ReadConfigZone(uint8_t device_id, uint8_t *config_data);
uint8_t ECCX08A_check_lock_status(void);
uint8_t ECC108A_check_private_key_slot0_config(void);
uint8_t ECCX08A_INFO_COMMAND(uint8_t mode, uint16_t param);
uint8_t ECCX08A_WriteData(uint8_t slot, uint8_t *pubkey);
uint8_t ECCX08A_ReadData(uint8_t slot, uint8_t *pubkey);

/*******Abstracted APIs***********/
void getPublicKey(uint8_t *pubKey);
void getMasterSecretKey(uint8_t *serverPubKey,uint8_t *secretKey);
#endif	/* ATECC508A_H */

