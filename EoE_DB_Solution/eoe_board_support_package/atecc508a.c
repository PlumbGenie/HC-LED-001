/*
 * File:   atecc508a.c
 * Author: C16008
 *
 * Created on June 16, 2016, 3:46 PM
 */





#include <xc.h>
#include "atecc508a.h"
#include <string.h>
#include "eccX08_comm_marshaling.h"
#include "eccX08_lib_return_codes.h"
#include "eccX08_physical.h"
#include "eccX08_comm.h"
#include "hardware.h"
#include "log.h"


  
// GenKey response buffer
uint8_t PublicKey[64];
uint8_t response_random[32];
volatile uint8_t cpyIndex = 0;
volatile uint8_t cpySize = 0;
uint8_t block = 0;
uint8_t offset = 0;
uint16_t slot_data_address;
uint8_t slotnum = 0;
uint8_t writeIndex = 0;	
uint8_t readIndex = 0;


//uint8_t serverpubKey[64] = {0x23,0x15, 0x04, 0x23, 0xa2, 0x3b, 0xc9, 0xe1, 0x91, 0xa1, 0x56, 0x14, 0x2f, 0x1e, 0x8f, 0x35, 0x3a,
//0x69, 0xef, 0xb6, 0x14, 0x1d, 0xed, 0xfc, 0x1e, 0x2f, 0x98, 0xb1, 0x70, 0x93, 0x38,0xc3, 0x80, 0x04, 0xec, 0x80, 0x9a, 0x6d, 0x6b, 0xdf, 0xe4,
//0x9e, 0x0d, 0x27, 0xb7, 0x03, 0xcf,0xc9, 0x5c, 0x39, 0xed, 0xde, 0x0b, 0xe5, 0x11, 0x4b, 0x65, 0xe1, 0xfc, 0xf8, 0x9f, 0x1b, 0x00,0xc4}; 
#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_ECC, msgSeverity, msgLogDest)
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

void ECCX08A_Init()
{    
    if(!ECCX08A_IsZoneLocked(ECCX08_ZONE_CONFIG))
    {  
        logMsg("ECC_CONF_NL", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
       
        /*  PrivateKey Slot COnfiguration */
        ECCX08A_SlotConfig(CL_ECCX08A_PRIV_KEY_ID, CL_ECCX08A_PRIV_KEY_SLOTCONFIG_BLOCK, CL_ECCX08A_PRIV_KEY_SLOTCONFIG_OFFSET, CL_ECCX08A_PRIV_KEY_SLOTCONFIG);      
        
        /* PublicKey Slot Configuration */        
        ECCX08A_SlotConfig(CL_ECCX08A_PUB_KEY_ID, CL_ECCX08A_PUB_KEY_SLOTCONFIG_BLOCK, CL_ECCX08A_PUB_KEY_SLOTCONFIG_OFFSET, CL_ECCX08A_PUB_KEY_SLOTCONFIG);
     
        /*  TLS/DTLS Server PublicKey Slot Configuration */
        ECCX08A_SlotConfig(CL_ECCX08A_SERVER_PUB_KEY_ID, CL_ECCX08A_SERVER_PUB_KEY_SLOTCONFIG_BLOCK, CL_ECCX08A_SERVER_PUB_KEY_SLOTCONFIG_OFFSET, CL_ECCX08A_SERVER_PUB_KEY_SLOTCONFIG);
      
        /* PrivateKey Key Configuration */
        ECCX08A_KeyConfig(CL_ECCX08A_PRIV_KEY_ID, CL_ECCX08A_PRIV_KEY_KEYCONFIG_BLOCK, CL_ECCX08A_PRIV_KEY_KEYCONFIG_OFFSET, CL_ECCX08A_PRIV_KEY_KEYCONFIG);
     
        /* PublicKey Key Configuration */
        ECCX08A_KeyConfig(CL_ECCX08A_PUB_KEY_ID, CL_ECCX08A_PUB_KEY_KEYCONFIG_BLOCK, CL_ECCX08A_PUB_KEY_KEYCONFIG_OFFSET, CL_ECCX08A_PUB_KEY_KEYCONFIG);

        /* TLS/DTLS Server PublicKey Key Configuration */
        ECCX08A_KeyConfig(CL_ECCX08A_SERVER_PUB_KEY_ID, CL_ECCX08A_SERVER_PUB_KEY_KEYCONFIG_BLOCK, CL_ECCX08A_SERVER_PUB_KEY_KEYCONFIG_OFFSET, CL_ECCX08A_SERVER_PUB_KEY_KEYCONFIG);
                 

        /* LOCK the Configuration Zone */
#ifdef LOCK_CONFIG_ENABLE
        ECCX08A_LockConfigZone();        //TODO - Debug and see if the configuration is set properly an then umcomment this API      
                                        //TODO - Lock config zone only when the configuration is matched
#endif
    }
     ECCX08A_GenPrivKey(CL_ECCX08A_PRIV_KEY_ID);
     ECCX08A_WritePubKey(CL_ECCX08A_PUB_KEY_ID, PublicKey);     
        
#ifdef LOCK_DATA_ENABLE    
        ECCX08A_LockDataOTPZone();
#endif
//        ECCX08A_ReadPubKey(CL_ECCX08A_PUB_KEY_ID, pkey);
        ECCx08A_RandomCommand(response_random);
        
     logMsg("ECC_INIT_DONE", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));

}

uint8_t ECCX08A_SlotConfig(uint8_t slot,uint8_t block, uint8_t offset,uint32_t data)
{
    uint8_t ret_code;	
   	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    uint16_t slot_config_address;
    uint8_t config_data[ECCX08_ZONE_ACCESS_4];
    
 
	ret_code = eccX08c_wakeup(response);

	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    memset(config_data,0,sizeof(config_data));
    memset(response,0,sizeof(response));
    config_data[0] = ((uint8_t *)&data)[3];
    config_data[1] = ((uint8_t *)&data)[2];
    config_data[2] = ((uint8_t *)&data)[1];
    config_data[3] = ((uint8_t *)&data)[0];

    /* Debug Info
     *  
     *      config_data[0] = 0x8F;
     *      config_data[1] = 0x20;
     *      config_data[2] = 0x83;
     *      config_data[3] = 0x20;
     *      atcau_get_addr(ECCX08_ZONE_CONFIG,0,0,5,&slot_config_address); //slot0 
     */
    
    ECCX08A_GetAddress(ECCX08_ZONE_CONFIG,slot,block,offset,&slot_config_address);

    ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_CONFIG, slot_config_address, sizeof(config_data), config_data,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
  
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();
        
    return ret_code;

}

uint8_t ECCX08A_KeyConfig(uint8_t slot,uint8_t block, uint8_t offset,uint32_t data)
{
    uint8_t ret_code;	
    uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    uint16_t key_config_address;
    uint8_t config_data[ECCX08_ZONE_ACCESS_4];    
   
	ret_code = eccX08c_wakeup(response);

	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    memset(response,0,sizeof(response));
    config_data[0] = ((uint8_t *)&data)[3];
    config_data[1] = ((uint8_t *)&data)[2];
    config_data[2] = ((uint8_t *)&data)[1];
    config_data[3] = ((uint8_t *)&data)[0];
   /* Debug Info
    *   
    *      config_data[0] = 0x33;
    *      config_data[1] = 0x00;
    *      config_data[0] = 0x30;
    *      config_data[1] = 0x00;     
    *      atcau_get_addr(ECCX08_ZONE_CONFIG,0,3,0,&key_config_address); //key0
    */
    

    ECCX08A_GetAddress(ECCX08_ZONE_CONFIG,slot,block,offset,&key_config_address);     
    ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_CONFIG, key_config_address, sizeof(config_data), config_data,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();
    
    return ret_code;    
}

uint8_t ECCX08A_IsZoneLocked(uint8_t zone)
{
    uint8_t ret_code =0;
    uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    uint16_t lock_config_address; 
    uint8_t idx;
  
    ret_code = eccX08c_wakeup(response);

	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    memset(command, 0, sizeof(command));
    memset(response, 0, sizeof(response));
    
    ret_code = ECCX08A_GetAddress(ECCX08_ZONE_CONFIG,0,2,5,&lock_config_address);

    ret_code = ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG, lock_config_address, 0, NULL,
            0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    
    if (ret_code != ECCX08_SUCCESS) {
        return ret_code;
    }
    eccX08p_sleep();
    
    if(zone == 0)
        idx = 4;
    if(zone == 1 || zone == 2)
        idx = 3;
    
    if(response[idx] == 0x00)
        return ECCX08_FUNC_FAIL;  
    else 
        return ECCX08_SUCCESS;       
    
}

uint8_t ECCX08A_LockConfigZone()
{
	uint8_t ret_code;
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
	uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
	ret_code = ECCX08A_IsZoneLocked(ECCX08_ZONE_CONFIG);
  
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;	
    }    
    
	ret_code = eccX08c_wakeup(response);
	ret_code = eccX08m_execute(ECCX08_LOCK, LOCK_MODE_CONFIG | LOCK_MODE_NO_CRC, 0, 0, NULL, 0, NULL, 0, NULL,
				sizeof(command), command, sizeof(response), response);
    
     eccX08p_sleep();

	return ret_code;
}

uint8_t ECCX08A_LockDataOTPZone()
{
	uint8_t ret_code;
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
	uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
    /** Config Zone MUST be Locked */
	ret_code = ECCX08A_IsZoneLocked(ECCX08_ZONE_CONFIG);
  
	if (ret_code == ECCX08_SUCCESS) {
		return ret_code;
	}
    
    /** If already Locked*/
    ret_code = ECCX08A_IsZoneLocked(ECCX08_ZONE_DATA);
  
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    

	ret_code = eccX08c_wakeup(response);
	ret_code = eccX08m_execute(ECCX08_LOCK, LOCK_MODE_DATA_OTP | LOCK_MODE_NO_CRC, 0, 0, NULL, 0, NULL, 0, NULL,
				sizeof(command), command, sizeof(response), response);
    
     eccX08p_sleep();

	return ret_code;
}

uint8_t ECCX08A_LockDataSlot(uint8_t slot)
{
	uint8_t ret_code;
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
	uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
    /** Config Zone MUST be Locked  and Data zone may or may not be locked **/
	ret_code = ECCX08A_IsZoneLocked(ECCX08_ZONE_CONFIG);
  
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}   

	ret_code = eccX08c_wakeup(response);
	ret_code = eccX08m_execute(ECCX08_LOCK, (slot << 2) | LOCK_MODE_SINGLE_SLOT, 0, 0, NULL, 0, NULL, 0, NULL,
				sizeof(command), command, sizeof(response), response);
    
     eccX08p_sleep();

	return ret_code;
}


   

uint8_t ECCX08A_GenPrivKey(uint8_t keyID)
{
    uint8_t ret_code;
	uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];	
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];

	// Wake up the device.
	memset(wakeup_response, 0, sizeof(wakeup_response));
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    NOP();
    NOP();
    NOP();

	ret_code = eccX08c_wakeup(wakeup_response);

	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}

	ret_code = eccX08m_execute(ECCX08_GENKEY, GENKEY_MODE_PRIVATE,
				keyID, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
				command, sizeof(response), response);
    
    while(response[0] == 0xFF)
    {
        ret_code = eccX08m_execute(ECCX08_GENKEY, GENKEY_MODE_PRIVATE,
				keyID, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
				command, sizeof(response), response);
    } 
   
	if (ret_code != ECCX08_SUCCESS) {
		(void) eccX08p_sleep();
		return ret_code;
	}
    
     eccX08p_sleep();
     memcpy(PublicKey, &response[1],GENKEY_RSP_SIZE_LONG);

	return ret_code;
}

uint8_t ECCX08A_GenSharedSecretKey(uint8_t keyID,uint8_t * serverpubKey, uint8_t *sharedKey)
{
    uint8_t ret_code;
	uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];	
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];

	// Wake up the device.
	memset(wakeup_response, 0, sizeof(wakeup_response));
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    NOP();
    NOP();
    NOP();

//	ret_code = eccX08c_wakeup(wakeup_response);
//
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}

//	ret_code = eccX08m_execute(ECCX08_ECDH, ECDH_MODE_MASK,
//				keyID, 32, &serverpubKey[0], 32, &serverpubKey[32], 0, NULL, sizeof(command),
//				command, sizeof(response), response);
    
    do
    {
        ret_code = eccX08c_wakeup(wakeup_response);

	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
       ret_code = eccX08m_execute(ECCX08_ECDH, ECDH_MODE_MASK,
				keyID, 32, &serverpubKey[0], 32, &serverpubKey[32], 0, NULL, sizeof(command),
				command, sizeof(response), response);
       (void) eccX08p_sleep();
    }while(response[1] == 0x011 || response[1] == 0x03);    
  
    NOP();
    NOP();
    NOP();
	if (ret_code != ECCX08_SUCCESS) {
		(void) eccX08p_sleep();
		return ret_code;
	}
    
     eccX08p_sleep();
     memcpy(sharedKey, &response[1],GENKEY_RSP_SIZE_LONG);

	return ret_code;
}

uint8_t ECCX08A_WritePubKey(uint8_t slot, uint8_t *pubkey)
{
    uint8_t ret_code;	
   	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
   
    uint8_t write_block[ECCX08_BLOCK_SIZE];
	
	
	writeIndex = 0;	
    slot_data_address = 0;
    slotnum = slot;

    
    if(pubkey == NULL) {
        return ECCX08_BAD_PARAM;
    }
    
    if(slot < 8 || slot > 15) {
        return ECCX08_BAD_PARAM;
    }
    
    // Setup the first write block accounting for the 4 byte pad
 
    block = 0; offset = 0;
    writeIndex = (uint8_t)ECCX08_PUB_KEY_PAD;
    memset(write_block, 0, sizeof(write_block));
    cpySize = (uint8_t)ECCX08_BLOCK_SIZE - (uint8_t)ECCX08_PUB_KEY_PAD;
    memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
    cpyIndex += cpySize;
    
    // Write the first block
    ret_code = eccX08c_wakeup(response);
    memset(response,0,sizeof(response));    
    
    ECCX08A_GetAddress(ECCX08_ZONE_DATA,slotnum,block,offset,&slot_data_address);

    ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, sizeof(write_block), write_block,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    
    while(response[1] == 0x0F)
    {
        ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, sizeof(write_block), write_block,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    }
    
  
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();
    // Setup the second write block accounting for the 4 byte pad
    
    NOP();
    NOP();
    NOP();
    block = 1; offset = 0;
    writeIndex = 0;
    memset(write_block, 0, sizeof(write_block));
    // Setup for write 4 bytes starting at 0
    cpySize = (uint8_t)ECCX08_PUB_KEY_PAD;
    memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
    cpyIndex += cpySize;
    // Setup for write skip 4 bytes and fill the remaining block
    writeIndex += cpySize + (uint8_t)ECCX08_PUB_KEY_PAD;
    cpySize = ECCX08_BLOCK_SIZE - writeIndex;
    memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
    cpyIndex += cpySize;
    
    // Write the second block
    ret_code = eccX08c_wakeup(response);
    memset(response,0,sizeof(response));   
   
    ECCX08A_GetAddress(ECCX08_ZONE_DATA ,slotnum,block,offset,&slot_data_address);

    ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, sizeof(write_block), write_block,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
      while(response[1] == 0x0F)
    {
        ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, sizeof(write_block), write_block,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    }
  
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();
    
    // Setup the third write block
    
    NOP();
    NOP();
    NOP();
    block = 2;
    offset = 0;
    writeIndex = 0;
    memset(write_block, 0, sizeof(write_block));
    // Setup for write 8 bytes starting at 0
    cpySize = ECCX08_PUB_KEY_PAD + ECCX08_PUB_KEY_PAD;
    memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
    // Write the third block
    ret_code = eccX08c_wakeup(response);
    memset(response,0,sizeof(response));    
//    slotnum = slot;
    ECCX08A_GetAddress(ECCX08_ZONE_DATA ,slotnum,block,offset,&slot_data_address);
    
    ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, sizeof(write_block), write_block,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    
      while(response[1] == 0x0F)
    {
        ret_code = eccX08m_execute(ECCX08_WRITE, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, sizeof(write_block), write_block,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
    }
    
     NOP();
    NOP();
    NOP();
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();
    NOP();
    NOP();
    NOP();
     return ret_code;    
}


uint8_t ECCX08A_GenPubKey(uint8_t privslot, uint8_t * pubkey)
{
    volatile uint8_t ret_code;

	// Make the command buffer the size of the Verify command.
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];	

	// GenKey response buffer	
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
    NOP();
    NOP();
    NOP();
    eccX08p_sleep();
    ret_code = eccX08c_wakeup(response);
     memset(command,0,sizeof(command)); 
    memset(response,0,sizeof(response)); 
    ret_code = eccX08m_execute(ECCX08_GENKEY, GENKEY_MODE_PUBLIC,
				privslot, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
				command, sizeof(response), response);
    
    
     NOP();
    NOP();
    NOP();
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    
    eccX08p_sleep();
    
   
    memcpy(pubkey, response, sizeof(response));
    
    
    return ret_code;
}

uint8_t ECCX08A_ReadPubKey(uint8_t slot, uint8_t *pubkey)
{
    uint8_t ret_code;	
   	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    uint8_t read_buf[ECCX08_RSP_SIZE_64];
    
	 block = 0;
	 offset = 0;
	
	readIndex = 0;    
    
    
    if(slot < 8 || slot > 15) {
        return ECCX08_BAD_PARAM;
    }
     cpyIndex = 0;
	 cpySize = 0;
     slot_data_address = 0;  
    block = 0;
    NOP();
    NOP();
    NOP();
    memset(response,0,sizeof(response));  
    ret_code = eccX08c_wakeup(response);
    
    memset(read_buf,0,sizeof(read_buf));
    
     slotnum = slot;
      ECCX08A_GetAddress(ECCX08_ZONE_DATA ,slotnum,block,offset,&slot_data_address);
//     slot_data_address = 0x0048;
    ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32, slot_data_address, 0, NULL,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(read_buf), read_buf);
    
      while(read_buf[1] == 0x0F || read_buf[1] == 0x11)
    {
        ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32, slot_data_address, 0, NULL,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(read_buf), read_buf);
    }
    
  
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();
    
    cpySize = ECCX08_BLOCK_SIZE - ECCX08_PUB_KEY_PAD;
    readIndex = ECCX08_PUB_KEY_PAD + 1;
    memcpy(&pubkey[cpyIndex], &read_buf[readIndex], cpySize);
    cpyIndex += cpySize;
    
    block = 1;
    offset = 0;
    memset(read_buf,0,sizeof(read_buf));
    ret_code = eccX08c_wakeup(response);   
    memset(response,0,sizeof(response));  
    
     
   ECCX08A_GetAddress(ECCX08_ZONE_DATA ,slotnum,block,offset,&slot_data_address);
//     slot_data_address = 0x0148;
    ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, 0, NULL,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(read_buf), read_buf);
      while(read_buf[1] == 0x0F || read_buf[1] == 0x11)
    {
        ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, 0, NULL,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(read_buf), read_buf);
    }
  
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();

 	// Copy.  First four bytes
    cpySize = ECCX08_PUB_KEY_PAD;
    readIndex = 1;
    memcpy(&pubkey[cpyIndex], &read_buf[readIndex], cpySize);
    cpyIndex += cpySize;
    // Copy.  Skip four bytes
    readIndex = ECCX08_PUB_KEY_PAD + ECCX08_PUB_KEY_PAD;
    cpySize = ECCX08_BLOCK_SIZE - readIndex;
    readIndex = readIndex + 1;
    memcpy(&pubkey[cpyIndex], &read_buf[readIndex], cpySize);
    cpyIndex += cpySize;
    
    // Read the next block
    block = 2;
    offset = 0;
    memset(read_buf,0,sizeof(read_buf));
     ret_code = eccX08c_wakeup(response);   
    memset(response,0,sizeof(response));  
    
    
   ECCX08A_GetAddress(ECCX08_ZONE_DATA ,slotnum,block,offset,&slot_data_address);
//     slot_data_address = 0x0248;
    ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, 0, NULL,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(read_buf), read_buf);
      while(read_buf[1] == 0x0F || read_buf[1] == 0x11)
    {
        ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_DATA|ECCX08_ZONE_READWRITE_32 , slot_data_address, 0, NULL,
				0, NULL, 0, NULL, sizeof(command), command, sizeof(read_buf), read_buf);
    }
    
    if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
    eccX08p_sleep();

    // Copy.  The remaining 8 bytes
    cpySize = ECCX08_PUB_KEY_PAD + ECCX08_PUB_KEY_PAD;
    readIndex = 1;
    memcpy(&pubkey[cpyIndex], &read_buf[readIndex], cpySize);     
    NOP();
    NOP();
    NOP();
    
    return ret_code;
}


uint8_t ECCX08A_GetAddress(uint8_t zone, uint8_t slot, uint8_t block, uint8_t offset, uint16_t* addr)
{
	bool status = 1;

	if (addr == NULL) return 0;
	if (zone != ECCX08_ZONE_CONFIG && zone != ECCX08_ZONE_DATA && zone != ECCX08_ZONE_OTP) {
		return 0;;
	}
	*addr = 0;
	offset = offset & (uint8_t)0x07;

	if ((zone == ECCX08_ZONE_CONFIG) || (zone == ECCX08_ZONE_OTP)) {
		*addr = block << 3;
		*addr |= offset;
	}else if (zone == ECCX08_ZONE_DATA) {
		*addr = slot << 3;
		*addr  = *addr | offset;
		*addr = *addr | block << 8;
	}else
		status = 0;
	return status;
}

uint8_t ECCx08A_ShaCommand(uint8_t *message, uint8_t *digest, uint8_t length)
{
    uint8_t ret_code;
    
    uint8_t blocks = 0;
    uint8_t remainder = 0;
    uint8_t msgIndex = 0;
    
    if ( length == 0 || message == NULL || digest == NULL )
		return ECCX08_BAD_PARAM;
    
    do
    {
        blocks = length / SHA_BLOCK_SIZE;
		remainder = length % SHA_BLOCK_SIZE;       
       

        ret_code = SHA_Start();
        if ( ret_code != ECCX08_SUCCESS )
            break;

        while(blocks)
        {
            ret_code = SHA_Update(&message[msgIndex],SHA_BLOCK_SIZE);
            if ( ret_code != ECCX08_SUCCESS )
                break;
            msgIndex += SHA_BLOCK_SIZE;
            length -= SHA_BLOCK_SIZE;
            blocks--;
        }

        ret_code = SHA_End(&message[msgIndex], digest, length);

        if (ret_code != ECCX08_SUCCESS) {
            eccX08p_sleep();
            return ret_code;
        }

        

	} while(0);
    
    
    
    return ret_code;
}

uint8_t SHA_Start()
{
    uint8_t ret_code = ECCX08_SUCCESS;
    uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    
    ret_code = eccX08c_wakeup(wakeup_response);

    if (ret_code != ECCX08_SUCCESS) {
        return ret_code;
    }


    
    
    ret_code = eccX08m_execute(ECCX08_SHA, SHA_MODE_START,
				0, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
				command, sizeof(response), response); 
    if(response[1] != 0x00)
        return 1;
    NOP();
    NOP();
    NOP();
  
    return ret_code;    
}

uint8_t SHA_Update(uint8_t *message, uint8_t length)
{
    uint8_t ret_code = ECCX08_SUCCESS;
        
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
	
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    
    if(length > 0 && length==SHA_BLOCK_SIZE)
    {
        ret_code = eccX08m_execute(ECCX08_SHA, SHA_MODE_UPDATE,
                    length, length, message, 0, NULL, 0, NULL, sizeof(command),
                    command, sizeof(response), response);
       
    }

    if(response[1] != 0x00)
        return 1;
    return ret_code;      
}

uint8_t SHA_End(uint8_t *message, uint8_t *digest, uint8_t length)
{
    uint8_t ret_code;
    
   
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];

	
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    if(length >= 0 && length < SHA_BLOCK_SIZE) //0 to 63 bytes
    {
        ret_code = eccX08m_execute(ECCX08_SHA, SHA_MODE_END,
                    length, length, message, 0, NULL, 0, NULL, sizeof(command),
                    command, sizeof(response), response);
       
    }
    eccX08p_sleep();

    
    
    memcpy(digest,&response[1],32);
    
    return ret_code; 
}

uint8_t SHA_HMACStart(void)
{
    uint8_t ret_code = ECCX08_SUCCESS;
    uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
      NOP();
    NOP();
    NOP();
    
    ret_code = eccX08c_wakeup(wakeup_response);

    if (ret_code != ECCX08_SUCCESS) {
        return ret_code;   
    }
    do
    {
    ret_code = eccX08m_execute(ECCX08_SHA, SHA_MODE_HMAC_START,
				(CL_ECCX08A_PRIV_KEY_ID+1), 0, NULL, 0, NULL, 0, NULL, sizeof(command),
				command, sizeof(response), response); 
    }while(response[1] != 0x00);
//        return 1;
    NOP();
    NOP();
    NOP();
  
    return ret_code;    
}

uint8_t SHA_PublicUpdate(void)
{
    uint8_t ret_code = ECCX08_SUCCESS;
        
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
	
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    
  
   
        ret_code = eccX08m_execute(ECCX08_SHA, SHA_MODE_PUBLIC,
                CL_ECCX08A_PUB_KEY_ID, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
                command, sizeof(response), response);
     
  NOP();
    NOP();
    NOP();

    if(response[1] != 0x00)
    {
         eccX08p_sleep();
        return 1;
    }
    return ret_code;      
}

uint8_t SHA_HMACEnd(uint8_t *message, uint8_t *digest, uint8_t length)
{
    uint8_t ret_code;
    
   
	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];

	
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));

    
    if(length >= 0 && length < SHA_BLOCK_SIZE) //0 to 63 bytes
    {
        ret_code = eccX08m_execute(ECCX08_SHA, SHA_MODE_HMAC_END,
                    length, length, message, 0, NULL, 0, NULL, sizeof(command),
                    command, sizeof(response), response);
       
    }
    else
    {
        NOP();
        NOP();
        NOP();       
    }
    eccX08p_sleep();
    NOP();
    NOP();
    NOP();
    
    
    memcpy(digest,&response[1],32);
     for(uint8_t i =0; i< 32; i++)
    {
        printf("HMAC SHA Digest %x \r\n",digest[i]);
    }
    
    return ret_code; 
}

uint8_t ECCX08A_VerifyCommand(uint8_t *Rcomponent,  uint8_t rlen ,uint8_t *Scomponent, uint8_t slen)
{
    uint8_t ret_code;
    uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
    
#ifdef VERIFY_EXTERNAL_MODE
    uint8_t command[ECCX08_CMD_SIZE_MIN+128];
#endif
    
#ifdef VERIFY_STORED_MODE
    uint8_t command[ECCX08_CMD_SIZE_MIN+64];
#endif
    
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];

   	// Dummy Random response buffer
    uint8_t rdm[32];

	// Wake up the device.
	memset(wakeup_response, 0, sizeof(wakeup_response));
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
   
    //Dummy Random Number 
     ECCx08A_RandomCommand(rdm);     
     
    //Nonce Command
    ret_code = eccX08c_wakeup(wakeup_response);
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    
    ret_code = eccX08m_execute(ECCX08_NONCE, NONCE_MODE_PASSTHROUGH,
				NONCE_ZERO_RANDOM_OUT, NONCE_NUMIN_SIZE_PASSTHROUGH,
				(uint8_t *) &response_random, 0, NULL,
				0, NULL, sizeof(command), command, sizeof(response),
				response);
	ret_code = ECCX08A_CheckResponseStatus(ret_code, response);
	if (ret_code != ECCX08_SUCCESS) {
		(void) eccX08p_sleep();
		return ret_code;
	}
    
  //Verify Command  
    memset(response,0,sizeof(response));	
#ifdef VERIFY_EXTERNAL_MODE  
        ret_code = eccX08m_execute(ECCX08_VERIFY, VERIFY_MODE_EXTERNAL,
                VERIFY_KEY_P256, rlen, Rcomponent, slen, Scomponent, 0, NULL, sizeof(command),
                command, sizeof(response), response);
#endif
        
        
#ifdef VERIFY_STORED_MODE     
        ret_code = eccX08m_execute(ECCX08_VERIFY, VERIFY_MODE_STORED,
                CL_ECCX08A_PUB_KEY_ID, rlen, Rcomponent, slen, Scomponent, 0, NULL, sizeof(command),
                command, sizeof(response), response);
#endif
  
   ret_code = ECCX08A_CheckResponseStatus(ret_code, response);
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    
  
    eccX08p_sleep();
        
    return ret_code;      
    
}

uint8_t ECCX08A_SignExternalModeCommand(uint8_t keyId, uint8_t *signature)
{
    uint8_t ret_code;
    uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
    uint8_t command[ECCX08_CMD_SIZE_MIN+64];
    uint8_t response[ECCX08_RSP_SIZE_MIN+64];
    
    // Random response buffer
    uint8_t rdm[32];
    
	// Wake up the device.
	memset(wakeup_response, 0, sizeof(wakeup_response));
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    
    
    //Dummy Random Number 
    ECCx08A_RandomCommand(rdm);
    
    
    //Nonce Command
    ret_code = eccX08c_wakeup(wakeup_response);
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}

    ret_code = eccX08m_execute(ECCX08_NONCE, NONCE_MODE_PASSTHROUGH,
				NONCE_ZERO_RANDOM_OUT, NONCE_NUMIN_SIZE_PASSTHROUGH,
				(uint8_t *) &response_random, 0, NULL,
				0, NULL, sizeof(command), command, sizeof(response),
				response);

	ret_code = ECCX08A_CheckResponseStatus(ret_code, response);
	if (ret_code != ECCX08_SUCCESS) {
		(void) eccX08p_sleep();
		return ret_code;
	}
    
   //Sign Command 
    memset(response,0,sizeof(response));
    
    ret_code = eccX08m_execute(ECCX08_SIGN, SIGN_MODE_EXTERNAL,
            keyId, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
            command, sizeof(response), response);  

    ret_code = ECCX08A_CheckResponseStatus(ret_code, response);
	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}
    
    eccX08p_sleep();
    memcpy(signature,&response[1],64);

    return ret_code;          
}

uint8_t ECCx08A_RandomCommand(uint8_t *randomNumber)
{
   uint8_t ret_code;   
   uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
   uint8_t command[ECCX08_CMD_SIZE_MIN+64];
   uint8_t response[ECCX08_RSP_SIZE_MIN+64];

	// Wake up the device.
	memset(wakeup_response, 0, sizeof(wakeup_response));
    memset(command,0,sizeof(command));
    memset(response,0,sizeof(response));
    
	ret_code = eccX08c_wakeup(wakeup_response);

	if (ret_code != ECCX08_SUCCESS) {
		return ret_code;
	}

	ret_code = eccX08m_execute(ECCX08_RANDOM, RANDOM_MODE_NO_SEED_UPDATE, 0x0000, 0, NULL, 0, NULL,
				0, NULL, sizeof(command), command, sizeof(response), response);    
   

	if (ret_code != ECCX08_SUCCESS) {
		(void) eccX08p_sleep();
		return ret_code;
	}
    
    memcpy(randomNumber,&response[1],32);
    eccX08p_sleep();
    
	return ret_code;
}

uint8_t ECCX08A_CheckResponseStatus(uint8_t ret_code, uint8_t *response)
{
	if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
		return ret_code;
	}
	ret_code = response[ECCX08_BUFFER_POS_STATUS];
	if (ret_code != ECCX08_SUCCESS) {
		eccX08p_sleep();
	}

	return ret_code;
}

 /*
  * 
  * Debug APIs - Uncomment when required
  *   
  */  

//void readConfig()
//{
//    uint8_t ret_code;
//    uint8_t config_data[ECCX08_CONFIG_SIZE];
//  
//        NOP();
//    NOP();
//    NOP();
////    ret_code = ECCX08A_ReadConfigZone(0xC0, config_data);
//	if (ret_code != ECCX08_SUCCESS) {
//         NOP();
//         NOP();
//    }
//
//    
//    ret_code = ECCX08A_ReadConfigZone(0xC0, config_data);
//    NOP();
//    NOP();
//    NOP();
//    
//}

//uint8_t ECCX08A_ReadConfigZone(uint8_t device_id, uint8_t *config_data)
//{
//	// declared as "volatile" for easier debugging
//	volatile uint8_t ret_code;
//
//	uint16_t config_address;
//
//	// Make the command buffer the size of the Read command.
//	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
//
//	// Make the response buffer the size of the maximum Read response.
//	uint8_t command[ECCX08_RSP_SIZE_MIN+64];[READ_32_RSP_SIZE];
//
//    NOP();
//        NOP();
//        NOP();
//        
//	// Read first 32 bytes. Put a breakpoint after the read and inspect "response" to obtain the data.
//	ret_code = eccX08c_wakeup(response);
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	memset(response, 0, sizeof(response));
//	config_address = 0;
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG | ECCX08_ZONE_COUNT_FLAG, config_address >> 2,
//				0, NULL, 0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
//	eccX08p_sleep();
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	if (config_data) {
//		memcpy(config_data, &response[ECCX08_BUFFER_POS_DATA], ECCX08_ZONE_ACCESS_32);
//		config_data += ECCX08_ZONE_ACCESS_32;
//	}
//
//	// Read second 32 bytes. Put a breakpoint after the read and inspect "response" to obtain the data.
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08c_wakeup(response);
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	config_address += ECCX08_ZONE_ACCESS_32;
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG | ECCX08_ZONE_COUNT_FLAG, config_address >> 2,
//				0, NULL, 0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
//	eccX08p_sleep();
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	if (config_data) {
//		memcpy(config_data, &response[ECCX08_BUFFER_POS_DATA], ECCX08_ZONE_ACCESS_32);
//		config_data += ECCX08_ZONE_ACCESS_32;
//	}
//
//	// Read third 32 bytes. Put a breakpoint after the read and inspect "response" to obtain the data.
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08c_wakeup(response);
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	config_address += ECCX08_ZONE_ACCESS_32;
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG | ECCX08_ZONE_COUNT_FLAG, config_address >> 2,
//				0, NULL, 0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
//	eccX08p_sleep();
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	if (config_data) {
//		memcpy(config_data, &response[ECCX08_BUFFER_POS_DATA], ECCX08_ZONE_ACCESS_32);
//		config_data += ECCX08_ZONE_ACCESS_32;
//	}
//
//	// Read foruth 32 bytes. Put a breakpoint after the read and inspect "response" to obtain the data.
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08c_wakeup(response);
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	config_address += ECCX08_ZONE_ACCESS_32;
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG | ECCX08_ZONE_COUNT_FLAG, config_address >> 2,
//				0, NULL, 0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
//	eccX08p_sleep();
//	if (ret_code != ECCX08_SUCCESS)
//		return ret_code;
//
//	if (config_data) {
//		memcpy(config_data, &response[ECCX08_BUFFER_POS_DATA], ECCX08_ZONE_ACCESS_32);
//		config_data += ECCX08_ZONE_ACCESS_32;
//	}
//
//	// Put a breakpoint here and inspect "response" to obtain the data.
//	eccX08p_sleep();
//
//	if (ret_code == ECCX08_SUCCESS && config_data) {
//		memcpy(config_data, &response[ECCX08_BUFFER_POS_DATA], ECCX08_ZONE_ACCESS_32);
//	} 
//    
//    NOP();
//        NOP();
//        NOP();
//
//	return ret_code;
//}

//uint8_t ECCx08A_InfoCommand(void)
//{
//	uint8_t ret_code;
//	uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
//	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
//	uint8_t command[ECCX08_RSP_SIZE_MIN+64];[INFO_RSP_SIZE];
//
//	eccX08p_set_device_id(ECCX08_I2C_DEFAULT_ADDRESS);
//
//	// Wake up the device.
//	memset(wakeup_response, 0, sizeof(wakeup_response));
//    memset(command,0,sizeof(command));
//    memset(response,0,sizeof(response));
//    
//	ret_code = eccX08c_wakeup(wakeup_response);
//
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}
//
//	ret_code = eccX08m_execute(ECCX08_INFO, INFO_MODE_REVISION, INFO_NO_STATE, 0, NULL, 0, NULL,
//				0, NULL, sizeof(command), command, sizeof(response), response);    
//
//	if (ret_code != ECCX08_SUCCESS) {
//		(void) eccX08p_sleep();
//		return ret_code;
//	}
//
//	return ret_code;
//}

//

//uint8_t pubKeyGen()
//{
//     uint8_t ret_code;
//	uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
//	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
//// GenKey response buffer
//	static uint8_t response_genkey[GENKEY_RSP_SIZE_LONG];
//	static uint8_t response_status[ECCX08_RSP_SIZE_MIN];
//	eccX08p_set_device_id(ECCX08_I2C_DEFAULT_ADDRESS);
//
//	// Wake up the device.
//	memset(wakeup_response, 0, sizeof(wakeup_response));
//    memset(command,0,sizeof(command));
//    memset(response_genkey,0,sizeof(response_genkey));
//    NOP();
//    NOP();
//	ret_code = eccX08c_wakeup(wakeup_response);
//
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}
//
//	ret_code = eccX08m_execute(ECCX08_GENKEY, GENKEY_MODE_PUBLIC,
//				ECCX08_KEY_ID, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
//				command, sizeof(response_genkey), response_genkey);
//    
//   
//
//	if (ret_code != ECCX08_SUCCESS) {
//		(void) eccX08p_sleep();
//		return ret_code;
//	}    
//
//    
//     NOP();
//    NOP();
//    NOP();
//    NOP();
//
//	return ret_code;
//}

//uint8_t ECCX08A_check_lock_status(void)
//{
//	// declared as "volatile" for easier debugging
//	volatile uint8_t ret_code;
//
//	uint16_t lock_config_address = 84;
//
//	// Make the command buffer the size of a Read command.
//	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
//
//	// Make the response buffer the minimum size of a Read response.
//	uint8_t command[ECCX08_RSP_SIZE_MIN+64];[READ_4_RSP_SIZE];
//
//	// Make sure that configuration zone is locked.
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG,
//				lock_config_address >> 2, 0, NULL, 0, NULL,	0, NULL,
//				sizeof(command), command, sizeof(response), response);
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}
//
//	// Put client device to sleep.
//	eccX08p_sleep();
//
//	// Check the configuration lock status.
//	if (response[4] == 0x55) {
//		// Configuration Zone has not been locked.
//		// Throw error code.
//		return ECCX08_FUNC_FAIL;
//	}
//
//	return ECCX08_SUCCESS;
//}

//uint8_t ECC108A_check_private_key_slot0_config(void)
//{
//	// declared as "volatile" for easier debugging
//	volatile uint8_t ret_code;
//
//	// Slot configuration address for key (e.g. 48, 49)
//	uint16_t slot_config_address = 20;
//
//	const uint8_t read_config = 0x8F;
//	const uint8_t write_config = 0x20;
//
//	// Key configuration address for key (e.g. 48, 49)
//	uint16_t key_config_address = 96;
//
//	const uint8_t key_config_lsb = 0x33;
//	const uint8_t key_config_msb = 0x00;
//
//	// Make the command buffer the size of a Read command.
//	uint8_t command[ECCX08_CMD_SIZE_MIN+64];
//
//	// Make the response buffer the minimum size of a Read response.
//	uint8_t command[ECCX08_RSP_SIZE_MIN+64];[READ_4_RSP_SIZE];
//    
//    uint8_t wakeup_response[ECCX08_RSP_SIZE_MIN];
//    
//    NOP();
//    NOP();
//    NOP();
//
//	// Wake up the client device.
//	ret_code = eccX08c_wakeup(wakeup_response);
//
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}
//
//	// Read device configuration of SlotConfig0.
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG, slot_config_address >> 2, 0, NULL,
//				0, NULL, 0, NULL, sizeof(command), command, sizeof(response), response);
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}
//
//	// Check the configuration of SlotConfig0.
//	if (response[ECCX08_BUFFER_POS_DATA] != read_config ||
//			response[ECCX08_BUFFER_POS_DATA + 1] != write_config) {
//		// The Slot have not been configured correctly.
//		// Throw error code.
//		eccX08p_sleep();
//        NOP();
//        NOP();
//        NOP();
//		return ECCX08_FUNC_FAIL;
//	}
//
//	// Read device configuration of KeyConfig0.
//	memset(response, 0, sizeof(response));
//	ret_code = eccX08m_execute(ECCX08_READ, ECCX08_ZONE_CONFIG, key_config_address >> 2, 0, NULL, 0, NULL,
//				0, NULL, sizeof(command), command, sizeof(response), response);
//	if (ret_code != ECCX08_SUCCESS) {
//		return ret_code;
//	}
//
//	// Check the configuration of KeyConfig0.
//	if (response[ECCX08_BUFFER_POS_DATA] != key_config_lsb ||
//			response[ECCX08_BUFFER_POS_DATA + 1] != key_config_msb) {
//		// The Key have not been configured correctly.
//		// Throw error code.
//		eccX08p_sleep();
//		return ECCX08_FUNC_FAIL;
//	}
//
//// For this example, lock should be done by using ACES.
//// This function is only to show users how to lock the configuration zone
//// using a library function.
//#if defined(ECC108_EXAMPLE_CONFIG_WITH_LOCK)
//	ecc108p_sleep();
//	if (ret_code != ECC108_SUCCESS) {
//		return ret_code;
//	}
//
//	ret_code = ecc108e_lock_config_zone(ECC108_HOST_ADDRESS);
//#endif
//
//	// Check the configuration zone lock status
//	ret_code = eccX08e_check_lock_status();
//
//	return ret_code;
//}

//uint8_t ECCX08A_INFO_COMMAND(uint8_t mode, uint16_t param)
//{
//     volatile uint8_t ret_code;
//
//	// Make the command buffer the size of the Verify command.
//	static uint8_t command[ECCX08_CMD_SIZE_MIN+64];
//
//
//    uint8_t command[ECCX08_RSP_SIZE_MIN+64];[READ_4_RSP_SIZE];    
//    memset(response,0,4);
//    NOP();
//    NOP();
//    NOP();
//    ret_code = eccX08c_wakeup(response);
//    ret_code = eccX08m_execute(ECCX08_INFO, mode,
//				param, 0, NULL, 0, NULL, 0, NULL, sizeof(command),
//				command, sizeof(response), response);
//    
//    
//     NOP();
//    NOP();
//    NOP();
//    if (ret_code != ECCX08_SUCCESS) {
//		eccX08p_sleep();
//		return ret_code;
//	}
//    
//    eccX08p_sleep();
//    
//return ret_code;  
//    
//}

void getPublicKey(uint8_t *pubKey)
{
     ECCX08A_ReadPubKey(CL_ECCX08A_PUB_KEY_ID,pubKey);
}

void getMasterSecretKey(uint8_t *serverPubKey,uint8_t *secretKey)
{
      ECCX08A_GenSharedSecretKey(CL_ECCX08A_PRIV_KEY_ID, serverPubKey, secretKey);
}