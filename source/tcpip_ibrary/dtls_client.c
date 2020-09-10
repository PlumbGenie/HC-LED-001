/*
 * File:   dtls_client.c
 * Author: C16008
 *
 * Created on May 6, 2016, 10:20 AM
 */


#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "dtls_client.h"
#include "dtls_key_store.h"
#include "udpv4.h"
#include "network.h"
#include "lfsr.h"
#include "ip_database.h"
#include "ethernet_driver.h"
//#include "atecc508a.h"
#include "log.h"
#include "device_registry.h"

#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_DTLS, msgSeverity, msgLogDest)
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(*(a)))

typedef enum
{ 
    DTLS_STARTUP =0,
    DTLS_INIT,
    DTLS_RETRANSMISSION,
    DTLS_RESET,
    DTLS_TIMEOUT
}dtls_timerState_t;

typedef enum
{
    TLS_DTLS_UNKNOWN,
    TLS_DTLS_CLIENT_LABEL,
    TLS_DTLS_SERVER_LABEL,
    TLS_DTLS_KEY_EXPANSION_LABEL
}TlsDtlsKeyLabels_t;

typedef enum {
    UNKNOWN,  
    MASTER_SECRET_LABEL,
    KEY_EXPANSION_LABEL,
    CLIENT_FINISHED_LABEL,
    SERVER_FINISHED_LABEL            
}tlsLabel_items_t;


typedef struct
{
    time_t init;
    time_t retransmission;
    time_t max_time;   
}dtls_timeouts_t;

DTLS_ClientHello clientHello;
ServerHello serverHello;
DTLS_HelloVerifyRequest helloVerifyRequest;
DTLS_Handshake txHskProtocol, rxHskProtocol;
DTLSRecordLayer txRecordProtocol, rxRecordProtocol;
CipherSuiteSet txCS;
CompressionMethod txCM;
CertificateExchange clientKeyExchange, serverKeyExchange;
Extensions recvExt[CACHED_INFO];
ExtensionValues valExt;


static uint8_t sessionID = 0;
static uint16_t handshakeMessageSequence = 0;
static uint16_t epoch = 0;
static uint8_t seqNumIdx = 0;
static uint8_t cipherSuiteIndex = 0;
static uint32_t serverCertificateLength = 0;
static bool txContinue = false;
static bool helloVerifyRecvd = false;
static bool serverHelloDone = false;
static bool certificateRequest = false;
static bool dtlsPktRecvd = false;
static CertificateType clientCertificateTypeRequest[NUMBER_OF_CLIENT_CERTIFICATE_TYPES];
static CertificateType serverCertificateTypeRequest[NUMBER_OF_SERVER_CERTIFICATE_TYPES];
//static uint8_t clientCertificateType, serverCertificateType;
//static uint16_t ecPointFormat;

//TODO - Add Sequence Number , Message Sequence and Epoch check handlers


static HandshakeType_t currentHandshakeType = (HandshakeType_t)CLIENT_HELLO;
static HandshakeType_t receivedHandshakeType = (HandshakeType_t)HELLO_VERIFY_REQUEST;
static ContentType currentContentType = (ContentType)HANDSHAKE;
static ContentType receivedContentType = (ContentType)HANDSHAKE;
static HandshakeType_t previousHandshakeType;
static dtls_timerState_t dtlsTimerState = (dtls_timerState_t)DTLS_STARTUP;
static Alert currentAlertMessage, receivedAlertMessage;
static uint8_t timerIdx = 1;
static dtls_timeouts_t dtls_timeouts;


static void readRecordLayer();
static void readHandshakeProtocol(void);
static void readCipherSpecProtocol();
static void readAlertProtocol(void);

static int receiveHandshakeProtocolMessages();
static void transmitHandshakeProtocolMessages();

static void transmitCipherSpecProtocolMessages();
static int receiveCipherSpecProtocolMessages();

static void transmitAlertProtocolMessages(AlertLevel_t AlertLevel, AlertDescriptionType_t AlertDescriptionType);
static int receiveAlertProtocolMessages();


static void sendClientHello(void);
static void sendHelloVerify(void);
static void sendClientCertificate(void);
static void sendClientKeyExchange(void);
static void sendCertificateVerify(void);
static void sendFinished(void);

static int receiveHelloVerify(void);
static int receiveServerHello(void);
static int receiveServerCertificate(void);
static int receiveServerKeyExchange(void);
static int receiveCertificateRequest(void);
static int receiveServerHelloDone(void);

static void sendCipherSpecProtocol(void);
static int receiveCipherSpecProtocol();

static void  sendAlertProtocol(void);
static void fatalAlertMessage(void);
static void warningAlertMessage(void);


static int sha256EcdsaVerify(void);
static int rawPubliceKeyCertificate(void);

static void createMasterSecret(char *serverPubKey);
static uint8_t doPRF(char *key, tlsLabel_items_t label, char* seed, char *secret);
static  uint8_t MasterSecret[48];


static uint8_t x = 1; //DB


//DB - Hard-Code the ASN.1 format
uint8_t cert[] = {0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00}; 

const SignatureAndHashAlgorithm signhashAlgorithm[1] =
{
    {SHA256 , ECDSA , sha256EcdsaVerify}  
};

/*************************** Handshake Protocol TX/RX Message Handlers **********************/

const handshake_handler_t Handshake_CallBackTable[11] = 
{
    {HELLO_REQUEST          , NULL                         , NULL                           },
    {CLIENT_HELLO           , sendClientHello              , NULL                           }, 
    {SERVER_HELLO           , NULL                         , receiveServerHello             },
    {HELLO_VERIFY_REQUEST   , sendHelloVerify              , receiveHelloVerify            }, 
    {CERTIFICATE            , sendClientCertificate        , receiveServerCertificate      }, 
    {SERVER_KEY_EXCHANGE    , NULL                         , receiveServerKeyExchange     },
    {CERTIFICATE_REQUEST    , NULL                         , receiveCertificateRequest    },
    {SERVER_HELLO_DONE      , NULL                         , receiveServerHelloDone        },
    {CERTIFICATE_VERIFY     , sendCertificateVerify        , NULL                           },
    {CLIENT_KEY_EXCHANGE    , sendClientKeyExchange        , NULL                          },
    {FINISHED               , sendFinished                 , NULL                           }
};

/*************************** CipherSpec Protocol TX/RX Message Handlers **********************/

//const cipherSpec_handler_t CipherSpec_CallBackTable[1] = 
//{
//    {sendCipherSpecProtocol                         , receiveCipherSpecProtocol}   
//};

/*************************** Alert Protocol Message Handlers **********************/
const alert_handler_t Alert_CallBackTable[25] = 
{
    {CLOSE_NOTIFY                   , fatalAlertMessage},
    {UNEXPECTED_MESSAGE             ,fatalAlertMessage},
    {BAD_RECORD_MAC                 ,fatalAlertMessage},
    {DECRYPTION_FAILED_RESERVED     ,warningAlertMessage},
    {RECORD_OVERFLOW                ,fatalAlertMessage},
    {DECOMPRESSION_FAILURE          ,fatalAlertMessage},
    {HANDSHAKE_FAILURE              ,fatalAlertMessage},
    {NO_CERTIFICATE_RESERVED        ,warningAlertMessage},
    {BAD_CERTIFICATE                ,warningAlertMessage},
    {UNSUPPORTED_CERTIFICATE        ,warningAlertMessage},
    {CERTIFICATE_REVOKED            ,warningAlertMessage},
    {CERTIFICATE_EXPIRED            ,warningAlertMessage},
    {CERTIFICATE_UNKNOWN            ,warningAlertMessage},
    {ILLEGAL_PARAMETER              ,fatalAlertMessage},
    {UNKNOWN_CA                     ,fatalAlertMessage},
    {ACCESS_DENIED                  ,fatalAlertMessage},
    {DECODE_ERROR                   ,fatalAlertMessage},
    {DECRYPT_ERROR                  ,fatalAlertMessage},
    {EXPORT_RESTRICTION_RESERVED    ,warningAlertMessage},
    {ALERT_PROTOCOL_VERSION         ,fatalAlertMessage},
    {INSUFFICIENT_SECURITY          ,fatalAlertMessage},
    {INTERNAL_ERROR                 ,fatalAlertMessage},
    {USER_CANCELED                  ,warningAlertMessage},
    {NO_RENEGOTIATION               ,warningAlertMessage},
    {UNSUPPORTED_EXTENSION          ,fatalAlertMessage} 
};
/****************************DTLS Initialization*******************************/
void DTLS_init(void)
{
//    dtls_key_store_init();
    
    memset(&clientHello,0,sizeof(clientHello));
    memset(&serverHello,0,sizeof(serverHello));
    memset(&helloVerifyRequest,0,sizeof(helloVerifyRequest)); 
    memset(&txHskProtocol,0,sizeof(txHskProtocol));
    memset(&rxHskProtocol,0,sizeof(rxHskProtocol));
    memset(&txRecordProtocol,0,sizeof(txRecordProtocol)); 
    memset(&rxRecordProtocol,0,sizeof(rxRecordProtocol)); 
    memset(&txCS,0,sizeof(txCS));
    memset(&txCM,0,sizeof(txCM));    
    memset(&clientKeyExchange,0,sizeof(clientKeyExchange));
    memset(&serverKeyExchange,0,sizeof(serverKeyExchange));   
    memset(ServerPubKey,0,sizeof(ServerPubKey));
    memset(&dtls_timeouts,1,sizeof(dtls_timeouts));  
    memset(&currentAlertMessage,0,sizeof(currentAlertMessage));
    memset(&receivedAlertMessage,0,sizeof(receivedAlertMessage));
    memset(MasterSecret,0,sizeof(MasterSecret));
    
    cipherSuiteIndex = 0;
    currentHandshakeType = (HandshakeType_t)CLIENT_HELLO;
    receivedHandshakeType = (HandshakeType_t)HELLO_VERIFY_REQUEST;
    currentContentType = (ContentType)HANDSHAKE;
    receivedContentType = (ContentType)HANDSHAKE;    
    dtlsTimerState = (dtls_timerState_t)DTLS_STARTUP;
    
    clientCertificateTypeRequest[0] = (CertificateType)RAW_PUBLIC_KEY;
    clientCertificateTypeRequest[1] = (CertificateType)X509;
    
    serverCertificateTypeRequest[0] = (CertificateType)RAW_PUBLIC_KEY;
    
    txCS.cipherSuiteLength = NUMBER_OF_CIPHER_SUITES * sizeof(txCS.cipherSuiteLength);
    txCM.compression_methods_length = NUMBER_OF_COMPRESSION_METHOD;
    dtlsPktRecvd = false;
    handshakeMessageSequence = 0;
    
    NOP();
    NOP();
    NOP();
#ifdef LOCK_CONFIG_ENABLE
    //Read Public Key
     ECCX08A_ReadPubKey(CL_ECCX08A_PUB_KEY_ID,PublicKey);
#endif
     
    NOP();
    NOP();
    NOP();
    
}

/****************************DTLS Setters**************************************/

void setCipherSuites(uint16_t cipherSuite)
{
    txCS.cipherSuite[cipherSuiteIndex] = cipherSuite;
    cipherSuiteIndex++;
}




void setMessageSequence()
{
//    printf("Handshake before set %d \r\n",handshakeMessageSequence);
    handshakeMessageSequence = handshakeMessageSequence + 1;
//    printf("Handshake after set %d \r\n",handshakeMessageSequence);
}



void setSequenceNumber()
{
    //increment the sequence number
   //TODO Find optimized way to do this process
    txRecordProtocol.sequence_number[5]++;
    if(txRecordProtocol.sequence_number[5] == 255)
    {
        txRecordProtocol.sequence_number[4]++;
        txRecordProtocol.sequence_number[5] =0;
        if(txRecordProtocol.sequence_number[4] == 255)
        {
            txRecordProtocol.sequence_number[3]++;
            txRecordProtocol.sequence_number[4] =0;            
            if(txRecordProtocol.sequence_number[3] == 255)
            {
                txRecordProtocol.sequence_number[2]++;
                txRecordProtocol.sequence_number[3] =0;                
                if(txRecordProtocol.sequence_number[2] == 255)
                {
                    txRecordProtocol.sequence_number[1]++;
                    txRecordProtocol.sequence_number[2] =0;                    
                    if(txRecordProtocol.sequence_number[1] == 255)
                    {
                        txRecordProtocol.sequence_number[0]++;
                        txRecordProtocol.sequence_number[1] =0;    
                        if(txRecordProtocol.sequence_number[0] == 255)        
                            memset(txRecordProtocol.sequence_number,0,sizeof(txRecordProtocol.sequence_number)); 
                    }
                }
            }
        }
    }
    
}

void setEpoch()
{
    epoch++;
}

/****************************DTLS Getters**************************************/

char *getSequenceNumber()
{
    return txRecordProtocol.sequence_number;
}

uint16_t getCipherSuites(uint8_t index)
{
    if(index <= NUMBER_OF_CIPHER_SUITES)
    {
        return txCS.cipherSuite[index];
    }
    return 0;
}

void getRandomBytes(char *random, uint8_t length)
{
    for(uint8_t i =0; i< length; i++)
    {
        *random++ = lfsr();
    }
}

uint16_t getProtocolVersion()
{
    return PROTOCOL_VERSION;
}

uint16_t getEpoch()
{
    return epoch;
}

uint16_t getMessageSequence()
{
    return handshakeMessageSequence;
}


/****************************DTLS Resets**************************************/

void resetMessageSequence()
{
    handshakeMessageSequence = 0;
}

void resetSequenceNumber()
{
     memset(txRecordProtocol.sequence_number,0,sizeof(txRecordProtocol.sequence_number)); 
}

void resetEpoch()
{
    epoch = 0;
}


/****************************DTLS Timers StateMachine**************************************/


/**TODO - I think this state machine is only for Handshake protocol messages*/
void DTLS_TimerStateMachine(time_t now, time_t dtlsTimer)
{       
        /***Initial Timer Value of 1 second and double the value at each transmission upto maximum 60 seconds*/
    switch(dtlsTimerState)
    {
        case DTLS_STARTUP:
//            logMsg("DTLS Startup", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            dtlsTimerState = (dtls_timerState_t)DTLS_INIT;   
            resetMessageSequence();            
            break;            
        case DTLS_INIT:
//            logMsg("DTLS Init state", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            dtls_timeouts.init = now;
            dtls_timeouts.retransmission = now + DTLS_INITIAL_TIMER;
            dtls_timeouts.max_time = now + DTLS_MAXIMUM_TIMER;                
            dtlsTimerState = (dtls_timerState_t)DTLS_RETRANSMISSION; 
            resetSequenceNumber();             
            break;
        case DTLS_RETRANSMISSION:  
//             logMsg("DTLS Retransmission", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            if(dtlsTimer==dtls_timeouts.max_time)
            {
                dtls_timeouts.init = 0;
                dtlsTimerState = (dtls_timerState_t)DTLS_TIMEOUT;
            }
            else if(dtlsTimer == dtls_timeouts.retransmission)
            {   
                dtls_timeouts.init  = dtls_timeouts.retransmission;
                dtls_timeouts.retransmission = dtls_timeouts.retransmission + (timerIdx * DTLS_RETRANSMISSION_TIMER_IDX);                    
                dtlsTimerState = (dtls_timerState_t)DTLS_RETRANSMISSION;
                timerIdx = timerIdx * DTLS_RETRANSMISSION_TIMER_IDX;  
                setSequenceNumber();                
            }
            break;
        case DTLS_RESET:
//             logMsg("DTLS Reset", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            dtlsTimerState = (dtls_timerState_t)DTLS_INIT;           
            break;           
        case DTLS_TIMEOUT:  
//             logMsg("DTLS Timeout", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
             resetSequenceNumber();
            break;
        default:
            break;
    }  
}

/****************************DTLS TX Handler**************************************/

void DTLS_Manage()
{     
    static time_t dtlsTimer = 0;   
    time_t now_dtls; 
    
    if(ipdb_getAddress()==0)
    {       
        return;
    }   
    now_dtls = time(0);   
    if(dtlsTimer <= now_dtls)
    { 
//        logMsg("DTLS START", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
        DTLS_TimerStateMachine(now_dtls, dtlsTimer);
//        logMsg("Exitted Timer State Machine", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    }
    if(dtlsTimer == dtls_timeouts.init || dtlsPktRecvd == true)
    {     
        do
        {
            switch(currentContentType)
            {               
               case CHANGE_CIPHER_SPEC:
//                   logMsg("DTLS CIPHER SPEC ", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET)); 
                   //TODo Add transmit handler- setEpoch Only after the handler
                   transmitCipherSpecProtocolMessages();
                    setEpoch();
                    break;
                case ALERT:
//                    logMsg("DTLS Alert ", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET)); 
                    transmitAlertProtocolMessages(currentAlertMessage.level, currentAlertMessage.description);
                    break;
                case HANDSHAKE:
//                    logMsg("DTLS Handshake ", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));   
                    transmitHandshakeProtocolMessages();
                    break;
                case APPLICATION_DATA:
//                    logMsg("DTLS Data ", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET)); 
                    break;
                default:
//                    logMsg("DTLS Default ", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET)); 
                    break;
            }
        }while(txContinue == true);
    }  
    dtlsTimer = now_dtls + 1;
}

/****************************DTLS RX Handler**************************************/

void DTLS_Handler(int length)
{
    int len = 0;  
    
    while(length > 0)
    {
        len = 0;        
        readRecordLayer();
        receivedContentType = rxRecordProtocol.type;
        len += sizeof(rxRecordProtocol);        
        if(rxRecordProtocol.version!=txRecordProtocol.version)
        {
            transmitAlertProtocolMessages(ALERT_FATAL,ALERT_PROTOCOL_VERSION);
            return; //Error and Dump the packet
        }

        if(strcmp(rxRecordProtocol.sequence_number, txRecordProtocol.sequence_number)!=0)
            return; //Error and Dump the packet


        switch(receivedContentType)
        {
            case CHANGE_CIPHER_SPEC:
                //Add receive handler- setEpoch Only after the handler
                len += receiveCipherSpecProtocolMessages();
                setEpoch();
                break;
            case ALERT:
                len += receiveAlertProtocolMessages();
                break;
            case HANDSHAKE:
                len += receiveHandshakeProtocolMessages();
                break;
            case APPLICATION_DATA:
                break;
            default:
                break;
        }
        length = length - len;        
    }
    
}


/****************************DTLS TX Handshake Protocol Handler**************************************/

static void transmitHandshakeProtocolMessages()
{
    const handshake_handler_t *hptr;
    static time_t dtlsTimer = 0;   
    time_t now;    
    
    hptr = Handshake_CallBackTable;    
    
//     logMsg("DTLS Inside Handshake", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
     for(uint8_t i = 0; i <ARRAYSIZE(Handshake_CallBackTable); i++)
     {
         if(hptr->type==currentHandshakeType)
         {
             hptr->txCallBack();                
             break;
         }
         hptr++;
     }
     

}

/****************************DTLS RX Handshake Protocol Handler **************************************/

static int receiveHandshakeProtocolMessages()
{
    int len = 0;
    const handshake_handler_t *hptr;   
    hptr = Handshake_CallBackTable;   
    readHandshakeProtocol();
    len += sizeof(rxHskProtocol);
    
    receivedHandshakeType = rxHskProtocol.messageType;    
    for(uint8_t i = 0; i < ARRAYSIZE(Handshake_CallBackTable); i++)
    {
        if(hptr->type==receivedHandshakeType)
        {
            len += hptr->rxCallBack();
            break;
        }
        hptr++;
    }    
    return len;
 }


/****************************DTLS TX Cipher Spec Protocol Handler**************************************/

/**Implement DTLS Timer State Machine***/

static void transmitCipherSpecProtocolMessages()
{ 
    
        sendCipherSpecProtocol();   
        currentContentType = (ContentType)HANDSHAKE;
        

}

/****************************DTLS RX CipherSpec Protocol Handler **************************************/

static int receiveCipherSpecProtocolMessages()
{
    int len = 0;
  
    readCipherSpecProtocol();
    len += sizeof(rxHskProtocol);
     
    receivedHandshakeType = rxHskProtocol.messageType;    
    len = receiveCipherSpecProtocol();
    
    currentContentType = (ContentType)APPLICATION_DATA;
      
    return len;
}


/****************************DTLS TX Alert Protocol Handler**************************************/

static void transmitAlertProtocolMessages(AlertLevel_t AlertLevel, AlertDescriptionType_t AlertDescriptionType)
{ 
    currentAlertMessage.level = AlertLevel;
    currentAlertMessage.description = AlertDescriptionType;
    
    sendAlertProtocol();   
    
}

/****************************DTLS RX Alert Protocol Handler **************************************/

static int receiveAlertProtocolMessages()
{
    int len = 0;
    const alert_handler_t *hptr;
    hptr = Alert_CallBackTable;
  
    readAlertProtocol();
    len += sizeof(receivedAlertMessage);
    
    for(uint8_t i = 0; i < ARRAYSIZE(Alert_CallBackTable); i++)
    {
        if(hptr->alertDescription==receivedAlertMessage.description)
        {
            hptr->CallBack();
            break;
        }
        hptr++;
    }    
    return len;
}

/****************************DTLS Read Record Layer**************************************/

static void readRecordLayer()
{
    rxRecordProtocol.type = UDP_Read8();
    rxRecordProtocol.version = UDP_Read16();    
    rxRecordProtocol.epoch = UDP_Read16();
    UDP_ReadBlock(rxRecordProtocol.sequence_number,6);
    rxRecordProtocol.length = UDP_Read16();
}

/****************************DTLS Read Handshake Protocol**************************************/

static void readHandshakeProtocol(void)  
{
    storeRecvdHandshakeMessage();
    rxHskProtocol.messageType = UDP_Read8();
    rxHskProtocol.length = UDP_Read24();
    rxHskProtocol.messageSequence = UDP_Read16();
    rxHskProtocol.fragmentOffset = UDP_Read24();
    rxHskProtocol.fragmentLength = UDP_Read24();
}

/****************************DTLS Read CipherSpec Protocol**************************************/

static void readCipherSpecProtocol()  
{
    //TODO - ADD code

}

/****************************DTLS Read Alert Protocol**************************************/

static void readAlertProtocol(void)  
{
    receivedAlertMessage.level = UDP_Read8();
    receivedAlertMessage.description = UDP_Read8();
}

/****************************DTLS Write Record Layer**************************************/
void writeRecordLayer(ContentType type)
{    
    UDP_Write8(type);
    UDP_Write16(txRecordProtocol.version);    
    UDP_Write16(txRecordProtocol.epoch);
    UDP_WriteBlock(txRecordProtocol.sequence_number,6);
    UDP_Write16(txRecordProtocol.length);
}


/****************************DTLS Write Handshake Protocol**************************************/

void writeHandshakeProtocol(HandshakeType_t messageType)  
{   
    storeSentHandshakeMessage();
    txHskProtocol.messageType = messageType;
    txHskProtocol.length = convert_hton24(txHskProtocol.length);
    txHskProtocol.messageSequence = htons(txHskProtocol.messageSequence);
    txHskProtocol.fragmentOffset = convert_hton24(txHskProtocol.fragmentOffset);
    txHskProtocol.fragmentLength = convert_hton24(txHskProtocol.fragmentLength);    
   
    
    UDP_WriteBlock(&txHskProtocol,sizeof(txHskProtocol));

}


/****************************DTLS Write Client Hello**************************************/
void writeClientHello()
{  
    clientHello.client_version = PROTOCOL_VERSION;
    UDP_Write16(PROTOCOL_VERSION);
    UDP_Write32(clientHello.random.gmt_unix_time);
    UDP_WriteBlock(clientHello.random.random_bytes,SIZE_OF_RANDOM_BYTES);
    UDP_Write8(clientHello.session_id.sessionIdLength);
    
    if(clientHello.session_id.sessionIdLength > 0)
    {
        UDP_WriteBlock(clientHello.session_id.sessionID,clientHello.session_id.sessionIdLength);
    }
    UDP_Write8(clientHello.client_cookie.cookieLength);
    
    if(clientHello.client_cookie.cookieLength > 0)
    {
        UDP_WriteBlock(clientHello.client_cookie.cookie,clientHello.client_cookie.cookieLength);
    }
    
    UDP_Write16(txCS.cipherSuiteLength);
    UDP_Write16(TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);//DB
//    UDP_Write16(TLS_PSK_WITH_AES_128_CCM_8);//DB    
    UDP_Write8(txCM.compression_methods_length);
    UDP_Write8(0);
    
    //TODO - May have to add Extensions if required
    
    /* The following two extension may be part of handshake when raw public keys are used */
    UDP_Write16(EXTENSIONS_LENGTH);
    //Client certificate Extension Type
    
    UDP_Write16(CLIENT_CERTIFICATE_TYPE);
    UDP_Write16(NUMBER_OF_CLIENT_CERTIFICATE_TYPES + 1);
    UDP_Write8(NUMBER_OF_CLIENT_CERTIFICATE_TYPES);
    for(uint8_t i=0; i < NUMBER_OF_CLIENT_CERTIFICATE_TYPES; i++)
    {
        UDP_Write8(clientCertificateTypeRequest[i]);
    }
    
    
    //Server certificate Extension Type
    
    UDP_Write16(SERVER_CERTIFICATE_TYPE);
    UDP_Write16(NUMBER_OF_SERVER_CERTIFICATE_TYPES + 1);
    UDP_Write8(NUMBER_OF_SERVER_CERTIFICATE_TYPES);
    for(uint8_t i=0; i < NUMBER_OF_SERVER_CERTIFICATE_TYPES; i++)
    {
        UDP_Write8(serverCertificateTypeRequest[i]);
    }
}


/****************************DTLS Send Handshake Protocol Messages**************************************/
//Flight 1/3


static void sendClientHello(void)
{
    error_msg ret = ERROR;   
    time_t now_time;
    
//     logMsg("DTLS Inside CLIENT HELLO", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    clientHello.client_cookie.cookieLength = helloVerifyRequest.server_cookie.cookieLength;
//    printf("Cookie Length %d \r\n",clientHello.client_cookie.cookieLength);
    strncpy(clientHello.client_cookie.cookie,helloVerifyRequest.server_cookie.cookie,clientHello.client_cookie.cookieLength);
    if(helloVerifyRecvd==false)
    {
        clientHello.random.gmt_unix_time = time(&now_time);
        getRandomBytes(clientHello.random.random_bytes,SIZE_OF_RANDOM_BYTES);                
    }
    else
    { 
//        printf("Going inside this \r\n");
        setMessageSequence();
//        currentHandshakeType = FINISHED; //DB
    }
    clientHello.session_id.sessionIdLength = 0; //Feature - greater than 0 if client wishes to generate new security parameters 
    
    
   
     if(clientHello.client_cookie.cookieLength > 0)
     {
         txHskProtocol.length = sizeof(DTLS_ClientHello) + sizeof(CompressionMethod) + sizeof(CipherSuiteSet) + EXTENSIONS_LENGTH + 2;
     }
    else
        txHskProtocol.length = sizeof(DTLS_ClientHello) + sizeof(CompressionMethod) + sizeof(CipherSuiteSet)+ EXTENSIONS_LENGTH + 2 - SIZE_OF_COOKIE;     
    
     if(clientHello.session_id.sessionIdLength > 0)
        txHskProtocol.length = txHskProtocol.length;
    else
        txHskProtocol.length = txHskProtocol.length - SIZE_OF_SESSIONID;    
    
        
    txHskProtocol.messageSequence = getMessageSequence();    
    txHskProtocol.fragmentOffset = 0;
    txHskProtocol.fragmentLength = txHskProtocol.length;    
    
   
    txRecordProtocol.version= PROTOCOL_VERSION;
   
    txRecordProtocol.epoch = getEpoch();    
    txRecordProtocol.length = sizeof(txHskProtocol)+ txHskProtocol.length;    
    
//    logMsg("DTLS Start UDP CLIENT HELLO", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET)); 
    ret = UDP_Start(DTLS_SERVER_ADDRESS,DTLS_SOURCE_PORT,COAP_DTLS_DEST_PORT);
    if(ret==(error_msg)SUCCESS)
    {
        //Start DTLS Packet        
//        printf("Writing client Hello \r\n");
        /******* Write Record Layer *****/
           writeRecordLayer(HANDSHAKE);     
        
        /***Write Handshake Protocol *****/
           writeHandshakeProtocol(CLIENT_HELLO);
        
        /*** Write Client Hello */
           writeClientHello();
          
            UDP_Send();
            if(helloVerifyRecvd==true)
            {
                readSentMessage();            
            }
    }
    else
    {
        dtlsTimerState = DTLS_INIT;        
//        logMsg("DTLS Start Failed", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    }
//    logMsg("DTLS Exit UDP CLIENT HELLO", LOG_EMERGENCY, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET)); 
    
}



static void sendClientCertificate(void)
{
    error_msg ret = ERROR;   
    if(certificateRequest == true && serverHelloDone == true)
    {
        setSequenceNumber();
        setMessageSequence();
//        printf("Message Sequence: %d  and HSKSeq: %d\r\n",getMessageSequence() ,handshakeMessageSequence);
        memset(&txHskProtocol,0,sizeof(txHskProtocol));
        txHskProtocol.length = sizeof(cert) + sizeof(PublicKey) + 4;
        txHskProtocol.messageSequence = getMessageSequence();    
        txHskProtocol.fragmentOffset = 0;
        txHskProtocol.fragmentLength = txHskProtocol.length;    
        
         txRecordProtocol.version= PROTOCOL_VERSION;
         txRecordProtocol.epoch = getEpoch();    
         txRecordProtocol.length = sizeof(txHskProtocol)+ txHskProtocol.length; 
         
         if(txContinue == false)
             ret = UDP_Start(DTLS_SERVER_ADDRESS,DTLS_SOURCE_PORT,COAP_DTLS_DEST_PORT);
         else
             ret = SUCCESS;
         
         if(ret==SUCCESS)
         {
            //Start DTLS Packet        

            /******* Write Record Layer *****/
               writeRecordLayer(HANDSHAKE);     

            /***Write Handshake Protocol *****/
               writeHandshakeProtocol(CERTIFICATE);

            /*** Write Client Certificate */               
               UDP_Write24(sizeof(cert) + sizeof(PublicKey) + 1);
               UDP_WriteBlock(cert,sizeof(cert));
               UDP_Write8(0x04);
               UDP_WriteBlock(PublicKey,sizeof(PublicKey));
               
               if(txRecordProtocol.length > 1200) //TODO - This should be TX Buffer size
               {
                   txContinue = false;
                   UDP_Send();
               }
               else
                   txContinue = true;
               
               currentHandshakeType = CLIENT_KEY_EXCHANGE;
               readSentMessage();         
         }
        
    }    

}

static void sendClientKeyExchange(void)
{
    error_msg ret = ERROR;
    //Write the key
    
   
    setSequenceNumber();
    setMessageSequence();
    memset(&txHskProtocol,0,sizeof(txHskProtocol));
    txHskProtocol.length = sizeof(PublicKey) + 2;
    txHskProtocol.messageSequence = getMessageSequence();    
    txHskProtocol.fragmentOffset = 0;
    txHskProtocol.fragmentLength = txHskProtocol.length;    

     txRecordProtocol.version= PROTOCOL_VERSION;
     txRecordProtocol.epoch = getEpoch();    
     txRecordProtocol.length = sizeof(txHskProtocol)+ txHskProtocol.length; 

    if(txContinue == false)
        ret = UDP_Start(DTLS_SERVER_ADDRESS,DTLS_SOURCE_PORT,COAP_DTLS_DEST_PORT);
    else
        ret = SUCCESS;
     
     if(ret==SUCCESS)
     {
         //TODo - Abstract this API
        
        //Start DTLS Packet        

        /******* Write Record Layer *****/
           writeRecordLayer(HANDSHAKE);     

        /***Write Handshake Protocol *****/
           writeHandshakeProtocol(CLIENT_KEY_EXCHANGE);

        /*** Write Client Key */               
           UDP_Write8(sizeof(PublicKey)+1);         
           UDP_Write8(0x04); //Write first byte octet           
           UDP_WriteBlock(PublicKey,sizeof(PublicKey));
           
            if(txRecordProtocol.length > 1200) //TODO - This should be TX Buffer size
            {
                txContinue = false;
                UDP_Send();
            }
            else
                txContinue = true;
           
////           currentHandshakeType = CERTIFICATE_VERIFY; 
           currentContentType = CHANGE_CIPHER_SPEC;
            readSentMessage();     
             //TODO -  Generate Pre Master Secret Key
    createMasterSecret(ServerPubKey);
     }  
}


static void sendCertificateVerify(void)
{
    memset(&txHskProtocol,0,sizeof(txHskProtocol));
    currentContentType = CHANGE_CIPHER_SPEC;
     readSentMessage();         
}

static void sendHelloVerify(void)
{
      //storeHandshakeMessage(typeNum);
}

static void sendFinished(void)
{
  /* RFC 6347 -  Finished message 
   * Hash calculations include entire handshake messages except the initial client hello (without cookie) and hello verify request
   * 
   * http://wiki.osdev.org/TLS_Handshake
   * Compute a SHA256 hash of a concatenation of all the handshake communications (or SHA384 if the PRF is based on SHA384). This means the Client Hello, Server Hello, Certificate, Server Key Exchange, Server Hello Done and Client Key Exchange messages. Note that you should concatenate only the handshake part of each TLS message (i.e. strip the first 5 bytes belonging to the TLS Record header)
   * Compute PRF(master_secret, "client finished", hash, 12) which will generate a 12-bytes hash
   * Append the following header which indicates the hash is 12 bytes: 0x14 0x00 0x00 0x0C
   * Encrypt the 0x14 0x00 0x00 0x0C | [12-bytes hash] (see the Encrypting / Decrypting data section). This will generate a 64-bytes ciphertext using AES-CBC and 40 bytes with AES-GCM
   * Send this ciphertext wrapped in a TLS Record
   */   
    uint8_t ret_code =0;
    error_msg ret = ERROR;
    char clientLabel[] = "client finished";
    char digest[47]; //no null terminator 
    char verifyData[32];
    memset(digest,0,sizeof(digest));
    memset(verifyData,0,sizeof(verifyData));
    uint8_t len=0;
    while(ret_code == 0)
    {
        ret_code = getMessageDigest(digest);
    }   

     txRecordProtocol.version= PROTOCOL_VERSION;
     txRecordProtocol.epoch = getEpoch();    
     txRecordProtocol.length = 12; 

    if(txContinue == false)
        ret = UDP_Start(DTLS_SERVER_ADDRESS,DTLS_SOURCE_PORT,COAP_DTLS_DEST_PORT);
    else
        ret = SUCCESS;
     
     if(ret==SUCCESS)
     {
         //TODo - Abstract this API
        
        //Start DTLS Packet        

        /******* Write Record Layer *****/
           writeRecordLayer(HANDSHAKE);     
           strcat(digest,clientLabel);
           
           //DB if 80 bytes of verify data - i=3
//           for(uint8_t i=0;i<2;i++) //A[1],A[2],A[3]
//           {
                len = strlen(digest);               
                ret_code = doExpansion(digest,verifyData,len);

//            if(i<1)
//            {
//                UDP_WriteBlock(verifyData,sizeof(verifyData));
////                printf("VerifyData[%d] size %d \r\n",i,sizeof(verifyData));
                strncpy(digest,verifyData,sizeof(verifyData));
//            }
//            else
                UDP_WriteBlock(verifyData,12);
//                printf("VerifyData[%d] size %d \r\n",i,sizeof(verifyData));
//           }
           UDP_Send();   
           txContinue = false;
     }
     //DB - Verify the Master key
     ret = UDP_Start(0xFFFFFFFF,65532,65533);
     if(ret ==SUCCESS)
     {
         UDP_WriteBlock(MasterSecret,48);
         UDP_Send();
     }
     
    
    currentContentType = -1;
    dtlsTimerState = DTLS_TIMEOUT; //DB
}

static void sendCipherSpecProtocol(void)
{
    error_msg ret = ERROR;
    
    setSequenceNumber();
    setMessageSequence();
    
    txRecordProtocol.version= PROTOCOL_VERSION;
    txRecordProtocol.epoch = getEpoch();    
    txRecordProtocol.length = 1; 

    if(txContinue == false)
        ret = UDP_Start(DTLS_SERVER_ADDRESS,DTLS_SOURCE_PORT,COAP_DTLS_DEST_PORT);
    else
        ret = SUCCESS;
    
    if(ret==SUCCESS)
    {
       //Start DTLS Packet        

       /******* Write Record Layer *****/
          writeRecordLayer(CHANGE_CIPHER_SPEC);     

       /***Write Change CipherSpec Protocol *****/
          UDP_Write8(0x01);        

         if(txRecordProtocol.length > 1200) //TODO - This should be TX Buffer size
         {
//             txContinue = false;
             UDP_Send();
             txContinue = false;
//             dtlsTimerState = DTLS_TIMEOUT;
             
         }
         else         
             txContinue = true;           
    }
    currentHandshakeType = FINISHED;
    currentContentType = HANDSHAKE;
    resetSequenceNumber();
    
}

static void  sendAlertProtocol(void)
{
    error_msg ret = ERROR;
    
    setSequenceNumber();
    setMessageSequence();
    
    txRecordProtocol.version= PROTOCOL_VERSION;
    txRecordProtocol.epoch = getEpoch();    
    txRecordProtocol.length = 2; 

    if(txContinue == false)
        ret = UDP_Start(DTLS_SERVER_ADDRESS,DTLS_SOURCE_PORT,COAP_DTLS_DEST_PORT);
    else
        ret = SUCCESS;
    
    if(ret==SUCCESS)
    {
       //Start DTLS Packet        

       /******* Write Record Layer *****/
          writeRecordLayer(ALERT);     

       /***Write Alert Protocol *****/
          UDP_Write8(currentAlertMessage.level);        
          UDP_Write8(currentAlertMessage.description);
          
          UDP_Send();
          txContinue = false;
          dtlsTimerState = DTLS_TIMEOUT;            
                  
    }
    currentHandshakeType = -1;
    currentContentType = -1;

}



/****************************DTLS Receive Handshake Protocol Messages**************************************/

static int receiveHelloVerify(void)
{
    int len =0;
    
    helloVerifyRequest.server_version = UDP_Read16();   
    helloVerifyRequest.server_cookie.cookieLength = UDP_Read8();
    len += 3;
    if(helloVerifyRequest.server_cookie.cookieLength > 0)
    {
        UDP_ReadBlock(helloVerifyRequest.server_cookie.cookie, helloVerifyRequest.server_cookie.cookieLength);
//        printf("helloVerifyRequest %s \r\n",&helloVerifyRequest.server_cookie.cookie[30]);
        len += helloVerifyRequest.server_cookie.cookieLength;
    }    
    helloVerifyRecvd = true;      
//    //storeHandshakeMessage(typeNum+1);
    
    return len;
}

static int receiveServerHello(void)
{
    int len =0;
    int length =0;
    uint16_t extType =0;    
    bool clientCertType= true, serverCertType = true;
    
    serverHello.server_version = UDP_Read16();
    serverHello.random.gmt_unix_time = UDP_Read32();   
    UDP_ReadBlock(serverHello.random.random_bytes, SIZE_OF_RANDOM_BYTES);    
    serverHello.session_id.sessionIdLength = UDP_Read8();   
    
    if(serverHello.session_id.sessionIdLength > 0)
    {
        UDP_ReadBlock(serverHello.session_id.sessionID,serverHello.session_id.sessionIdLength);
        len += sizeof(serverHello);        
    }
    else
    {
        len += sizeof(serverHello) - serverHello.session_id.sessionIdLength;
    }
   
    serverHello.cipherSuite = UDP_Read16();   
    serverHello.compression_methods = UDP_Read8();
   
    if((rxHskProtocol.length - len) > 0)
    {
        length = UDP_Read16();
        len += 2;
    }
    else
        length = 0;
    while(length > 0) 
    {  
        extType = UDP_Read16();
        recvExt[extType].extensionType = extType;
        switch(recvExt[extType].extensionType)
        {
            case CLIENT_CERTIFICATE_TYPE:
                clientCertType = false;
                recvExt[extType].extensionLength = UDP_Read16();
                valExt.clientCertificateType = UDP_Read8();
                len += 5;
                length -=5;
                for(uint8_t i =0; i< NUMBER_OF_CLIENT_CERTIFICATE_TYPES; i++)
                {
                    if(valExt.clientCertificateType==clientCertificateTypeRequest[i])
                    {
                        clientCertType = true;
                break;
                    }                        
                }
                break;
            case SERVER_CERTIFICATE_TYPE:
                serverCertType = true;
                recvExt[extType].extensionLength = UDP_Read16();
                valExt.serverCertificateType = UDP_Read8();
                len += 5;
                length -=5;
                 for(uint8_t i =0; i< NUMBER_OF_SERVER_CERTIFICATE_TYPES; i++)
                {
                    if(valExt.clientCertificateType==serverCertificateTypeRequest[i])
                    {
                        serverCertType = true;
                break;
                    }                        
                }
                break;
            case EC_POINT_FORMATS:
                recvExt[extType].extensionLength = UDP_Read16();
                valExt.ecPointFormat.ecPointLength = UDP_Read8();
                valExt.ecPointFormat.ecPointFormat = UDP_Read8();
                len += 6;
                length -=6;
                break;            
            default:
                //DB - Dump the Extensions
                length = UDP_Read16();
                ETH_Dump(length);
                len += length;
                //TODO - Send Alert Message
                transmitAlertProtocolMessages(ALERT_FATAL,UNSUPPORTED_EXTENSION);
                break;
        }        
        
    }    
    if(clientCertType == false || serverCertType == false)
    {
        //TODO - Send Alert message
        transmitAlertProtocolMessages(ALERT_FATAL,UNSUPPORTED_CERTIFICATE);
    }
    
    //storeHandshakeMessage(typeNum+1);
    readReceivedMessage();
    dtlsPktRecvd = true;
    return len;
}

static int receiveServerCertificate(void)
{
    int len =0;  
    
    if(rxHskProtocol.fragmentOffset == 0)
    {
        if(rxHskProtocol.length >= rxHskProtocol.fragmentLength)
        {
            serverCertificateLength = rxHskProtocol.length;
            switch(valExt.serverCertificateType) //TODO check whether this is server certificate or client certificate
            {
                case X509:
                    ETH_Dump(rxHskProtocol.fragmentLength); //DB - For now dumping the certificate  
                    len += (int)rxHskProtocol.fragmentLength;
                    break;
                case OPENPGP:
                    ETH_Dump(rxHskProtocol.fragmentLength); //DB - For now dumping the certificate   
                    len += (int)rxHskProtocol.fragmentLength;
                    break;
                case RAW_PUBLIC_KEY:
                    len += rawPubliceKeyCertificate();
                    break;
                default:
                    ETH_Dump(rxHskProtocol.fragmentLength); //DB - For now dumping the certificate   
                    len += (int)rxHskProtocol.fragmentLength;
                    break;
            }
             
            //TODO - Add condition what will be the next packet
            //have to get reassembled certificate            
        }
    }
    else
    {      
        if((serverCertificateLength - rxHskProtocol.fragmentOffset)==0)
        {
           ETH_Dump(rxHskProtocol.fragmentLength); //DB - For now dumping the certificate    
            //TODO - Add condition what will be the next packet 
            //Done receiving certificate          
        }
        else
        {
            ETH_Dump(rxHskProtocol.fragmentLength); //DB - For now dumping the certificate    
            //TODO - Add condition what will be the next packet
            //have to get reassembled certificate
        }
        
    }
     //storeHandshakeMessage(typeNum+1);

    len = (int)rxHskProtocol.fragmentLength;
    readReceivedMessage();   
    dtlsPktRecvd = true;
//    logMsg("DTLS Receive ServerCert", LOG_EMERGENCY, (LOG_DEST_ETHERNET));
    return len;
}

static int receiveServerKeyExchange(void)
{
    int len =0; 
    HashAlgorithm hash;
    SignatureAlgorithm signature;
    
    const SignatureAndHashAlgorithm *hptr;   
    
    hptr = signhashAlgorithm;
    
    serverKeyExchange.type = UDP_Read8();
    serverKeyExchange.subType = UDP_Read16();
    UDP_ReadBlock(&serverKeyExchange.len,sizeof(serverKeyExchange.len));
    UDP_Read8(); //Read first byte of Pub key
    
    UDP_ReadBlock(ServerPubKey,64);
    
    len = sizeof(serverKeyExchange) + serverKeyExchange.len;
    
    //Code - if signature
    if(rxHskProtocol.fragmentLength > len)
    {
        hash = UDP_Read8();
        signature = UDP_Read8();
        len += 2;
    
        for(uint8_t i =0; i < ARRAYSIZE(signhashAlgorithm); i++)
        {
            if(hptr->hash == hash && hptr->signature == signature)
            {
                len += hptr->callBack();
                break;
            }
            hptr++;            
        }        
       
    }    
   
    readReceivedMessage();
    dtlsPktRecvd = true;
//    logMsg("DTLS Receive ServerKX", LOG_EMERGENCY, (LOG_DEST_ETHERNET));
    
    return len;
}

static int receiveCertificateRequest(void)
{
    int len =0;//TODO add code
    
    ETH_Dump(rxHskProtocol.fragmentLength);
    
    dtlsTimerState = DTLS_INIT; 
    currentHandshakeType = CERTIFICATE;
    
    len = (int)rxHskProtocol.fragmentLength;
    
    certificateRequest = true;
   readReceivedMessage();
   dtlsPktRecvd = true;
//   logMsg("DTLS Receive CR", LOG_EMERGENCY, (LOG_DEST_ETHERNET));
    return len;
}

static int receiveServerHelloDone(void)
{ 
    serverHelloDone = true;
    if(certificateRequest == false)
        currentHandshakeType = CLIENT_KEY_EXCHANGE;
    
    readReceivedMessage();   
//    logMsg("DTLS Receive ServerHD", LOG_EMERGENCY, (LOG_DEST_ETHERNET));
    dtlsPktRecvd = true;
    return 0;
}

static int receiveCipherSpecProtocol(void)
{
    dtlsPktRecvd = true;
    return 0;
}

static void fatalAlertMessage(void)
{
    //TODO - reset all, close connection and send error message
//    printf("Send/Received Fatal \r\n");
    currentAlertMessage.level = ALERT_FATAL;
    currentAlertMessage.description = CLOSE_NOTIFY;
    sendAlertProtocol();
    
     resetMessageSequence();
     resetSequenceNumber();
     resetEpoch();
    
    
    
}

static void warningAlertMessage(void)
{
    //TODO - Connection can continue and send response based on description
    currentAlertMessage.level = ALERT_WARNING;
    currentAlertMessage.description = receivedAlertMessage.description;
//    sendAlertProtocol();
    
}





static int sha256EcdsaVerify(void)
{
    int len =0;
    Signature_t signature;


    signature.signatureLength = UDP_Read16();
    UDP_ReadBlock(signature.signature,signature.signatureLength);

    len += signature.signatureLength + 2;
    
    //TODO - Add code to verify the signature. If valid continue, else send Alert message - ALERT_FATAL, DECRYPT_ERROR
    
    return len;
}

static int rawPubliceKeyCertificate(void)
{
    int len = 0;
    Certificate serverCertificate;
    
    serverCertificate.length = UDP_Read24();
    UDP_ReadBlock(serverCertificate.Certificate,serverCertificate.length);
    
    len = serverCertificate.length + 3;
    
    return len;
}

static void createMasterSecret(char *serverPubKey)
{
    uint8_t preMasterSecret[32]; //Use local variable for premaster secret - preMaster Secret SHOULD be deleted from memory once Master Secret is been created
    uint8_t randomSeed[80];
    
    memset(preMasterSecret,0,sizeof(preMasterSecret));   
    memset(randomSeed,0,sizeof(randomSeed));
    
    clientHello.random.gmt_unix_time = htonl(clientHello.random.gmt_unix_time);
    strncpy(randomSeed,&clientHello.random.gmt_unix_time, 4);
    strncat(randomSeed,clientHello.random.random_bytes, SIZE_OF_RANDOM_BYTES);
    serverHello.random.gmt_unix_time = ntohl(serverHello.random.gmt_unix_time);
    strncat(randomSeed,&serverHello.random.gmt_unix_time, 4);
    strncat(randomSeed,serverHello.random.random_bytes, SIZE_OF_RANDOM_BYTES);
    
    getPreMasterSecret(serverPubKey, preMasterSecret);
    
    doPRF(MasterSecret,MASTER_SECRET_LABEL,randomSeed,preMasterSecret);
    
//    for(uint8_t i =0; i< 48; i++)
//    {
//        printf("MasterSecretKey[%d]: %x \r\n",i,MasterSecret[i]);
//    }   
    
}

static uint8_t doPRF(char *key, tlsLabel_items_t label, char* seed, char *secret)
{
    uint8_t length =0;  
    uint8_t iterations = 0;
    char doPRFsecret[32];
    
    memset(doPRFsecret,0,sizeof(doPRFsecret));

    switch(label)
    {
        case MASTER_SECRET_LABEL:
            strcat(seed,"master secret");
            length = 48;
            iterations = 2;
            break;
        case KEY_EXPANSION_LABEL:
             strcat(seed,"key expansion");
             length = 128;
             iterations = 4;
            break;
        case CLIENT_FINISHED_LABEL:
             strcat(seed,"client finished");
             iterations = 1;
             length = 12;
            break;
        case SERVER_FINISHED_LABEL:
             strcat(seed,"server finished");
             iterations = 1;
             length = 12;
            break;
        default:
            break;        
    }
    for(uint8_t i =0; i <iterations; i++)
    {
        doExpansion(seed,doPRFsecret,strlen(seed));
        memset(seed,0,sizeof(seed));
        
        if(i < (iterations -1))
        {
            strncat(key,doPRFsecret,32);
            strncpy(seed,doPRFsecret,strlen(key));
        }
        else
            strncat(key,doPRFsecret,length);
    }  
    
    return length;
    
}


