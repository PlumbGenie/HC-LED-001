/*
 * File:   dtls_key_store.c
 * Author: C16008
 *
 * Created on July 23, 2016, 9:59 AM
 */


#include <xc.h>
#include <string.h>
#include <stdio.h>
#include "atecc508a.h"
#include "dtls_key_store.h"
#include "dtls_client.h"
#include "eeprom.h"
#include "ethernet_driver.h"
#include "udpv4.h"
#include "eccX08_comm_marshaling.h"

static storagePtr dtlsEepromStartAddress;
static storagePtr dtlsEepromEndAddress;
static uint16_t handshakeMessageLength;
static uint16_t wrptr;
static uint16_t rdptr;
static uint16_t byteCount;
static uint16_t txbyteCount;

static uint16_t getSentBytesLength();
static uint16_t getRecvdBytesLength();
static void writeToEepromHskMessage(void *data, uint16_t length);

void dtls_key_store_init(void)
{
    
    dtlsEepromStartAddress = DTLS_START_ADDRESS;
    dtlsEepromEndAddress = DTLS_END_ADDRESS;
    handshakeMessageLength = 0;
    storageDtlsHandshakeErase();
    
}
static void writeToEepromHskMessage(void *data, uint16_t length)
{
    if((dtlsEepromStartAddress + length) > dtlsEepromEndAddress)
        return;
    
    storageWriteBlock(dtlsEepromStartAddress,data,length);
    dtlsEepromStartAddress += length;
    handshakeMessageLength += length;
    
}

uint16_t getHandshakeMessageLength()
{
    return handshakeMessageLength;
}

uint8_t getMessageDigest(char * digest)
{
   char message[64];
   uint16_t len = 0;
   uint8_t blocks = 0;
   uint8_t remainder = 0;
   uint8_t ret;   
    
   len = getHandshakeMessageLength();
    blocks = len / 64;
    remainder = len % 64;
    dtlsEepromStartAddress = DTLS_START_ADDRESS;
    
    memset(message,0,sizeof(message));
      //TODO - Remove this do while  
    do
    {
        ret = SHA_Start();
    }while(ret!=0);
    
//    printf("Block %d \r\n",blocks);
    while(blocks)
    { 
//        printf("In whiles Block %d \r\n",blocks);
        storageReadBlock(dtlsEepromStartAddress,message,64);

        ret = SHA_Update(message,SHA_BLOCK_SIZE);
        if(ret != 0)
        {
//            printf("I'm here \r\n");
            return 0;
        }
        
        dtlsEepromStartAddress += 64;
        
        memset(message,0,sizeof(message));
        
        blocks--;
    } 
    
   storageReadBlock(dtlsEepromStartAddress,message,remainder);  
   SHA_End(message,digest,remainder);  
       NOP();
    NOP();
    NOP();
    return 1;
    
}


uint8_t doExpansion(char * message, char *expansion, uint8_t len)
{  
 
   uint8_t ret;
   uint8_t blocks = 0;
   uint8_t remainder = 0;
   uint8_t dataLocation = 0;
    
   
    blocks = len / 64;
    remainder = len % 64;   
   
     //TODO - Remove this do while   
    do
    {
        ret = SHA_HMACStart();
    }while(ret!=0);    
    
    while(blocks)
    {
       
        ret = SHA_Update(message,SHA_BLOCK_SIZE);
        if(ret != 0)
        {
            return 0;
        }               
        
        dataLocation += 64;
        blocks--;
    } 

   
   SHA_HMACEnd(&message[dataLocation],expansion,remainder);  
   NOP();
   NOP();
   NOP();
   return 1;
    
}

void storeSentHandshakeMessage(void)
{
    wrptr = ETH_GetWritePtr();  
     txbyteCount = ETH_GetRxByteCount();
}



void readSentMessage(void)
{   
    char str[64];
    uint16_t len; 
    uint8_t block =0; 
    uint16_t remainder =0;      
    uint16_t ptr; 
    uint16_t count;
    
    len = getSentBytesLength();
     ptr = ETH_GetReadPtr();
     block = len / 64;
     remainder = len %64;
      count = ETH_GetRxByteCount();
    /** Set the read pointer to the Transmit Buffer*/ 
      ETH_SetRxByteCount(len);
    ETH_SetReadPtr(wrptr);
    memset(str,0,sizeof(str));
     NOP();
    NOP();
    while(block)
    {
        /** Read Data from the Transmit Buffer */
        UDP_ReadBlock(str,64); 
        writeToEepromHskMessage(str,64);
        memset(str,0,sizeof(str));
        block--;
    }
    
    if(remainder > 0)
    {
        UDP_ReadBlock(str,remainder);
        writeToEepromHskMessage(str,remainder);
    }
    
    /**Reset Read and Write Pointers*/
     ETH_SetReadPtr(ptr); 
//    ETH_TxReset();
    NOP();
    NOP();
    NOP();
}

void storeRecvdHandshakeMessage(void)
{
    rdptr = ETH_GetReadPtr();
    byteCount = ETH_GetRxByteCount();
}

void readReceivedMessage(void)
{
     char str[64];
    uint16_t len; 
     uint8_t block = 0; 
    uint16_t remainder =0;  
    uint16_t count; 
    
    NOP();
    NOP();
    NOP();
    len = getRecvdBytesLength();
    count = ETH_GetRxByteCount();
//    ptr = ETH_GetReadPtr();
     block = len / 64;;
     remainder = len %64;
    /** Set the read pointer to the Receive Buffer*/ 
    ETH_SetReadPtr(rdptr);
    ETH_SetRxByteCount(byteCount-count);
    memset(str,0,sizeof(str));
    
    while(block)
    {
        /** Read Data from the Receive Buffer */
        UDP_ReadBlock(str,64); 
        writeToEepromHskMessage(str,64);
        memset(str,0,sizeof(str));
        block--;
    }
    
    if(remainder > 0)
    {
        UDP_ReadBlock(str,remainder);
        writeToEepromHskMessage(str,remainder);
    }
    
    /**Reset Read Pointers*/
//    ETH_SetReadPtr(ptr); 
    NOP();
    NOP();
    NOP();
}

static uint16_t getSentBytesLength()
{
   return (ETH_GetWritePtr() - wrptr );
}

static uint16_t getRecvdBytesLength()
{
   return (ETH_GetReadPtr() - rdptr );
}


void getPreMasterSecret(char *srvPubKey,char *preMasterSecret)
{   
    
    ECCX08A_GenSharedSecretKey(CL_ECCX08A_PRIV_KEY_ID,srvPubKey,preMasterSecret); 
//    for(uint8_t i=0; i<32; i++)
//    {
//        printf("Secret Key secret[%d]: %x \r\n",i,preMasterSecret[i]);
//    }
    NOP();
    NOP();
    NOP();
}

void storeServerKey(char * serverPubKey)
{
    ECCX08A_WritePubKey(CL_ECCX08A_SERVER_PUB_KEY_ID, serverPubKey);
}

bool verifySignature(void)
{
    bool ret = false;
    
    return ret;
}


