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
#ifndef DTLS_CLIENT_H
#define	DTLS_CLIENT_H

#include <stdint.h>
#include <time.h>

#ifndef __XC8
 // TODO: This probably does not actually produce the correct packets.
 // It DOES compile so it is useful for GCC compile testing.
 typedef uint32_t uint32_t;
 #define UINT24_MAX 16777216UL
 #define UINT24_MIN 0
#endif

#define DTLS_SOURCE_PORT                           (65529)
#define COAP_DTLS_DEST_PORT                        (5684)
#define DTLS_SERVER_ADDRESS                        MAKE_IPV4_ADDRESS(192,168,1,2)
#define ECDH_ALGORITHM
 #define RAW_PUBLIC_KEY_CERTIFICATE

 #ifdef RAW_PUBLIC_KEY_CERTIFICATE
    #define SERVER_CERTIFICATE_LENGTH   (91)
#endif

#ifdef RSA_ALGORITHM
    typedef uint16_t PubKeyLengthType;                               
#endif
    
#ifdef ECDH_ALGORITHM
typedef uint8_t PubKeyLengthType;                               
    #define LENGHT_OF_SIGNATURE                         (72)
//    uint8_t ServerPubKey[64]; //jira: CAE_MCU8-5647
//    uint8_t PublicKey[64]; //jira: CAE_MCU8-5647
#endif

//Seconds
#define DTLS_INITIAL_TIMER                         (1)                  
#define DTLS_MAXIMUM_TIMER                         (60)

#define DTLS_RETRANSMISSION_TIMER_IDX              (2)
#define SIZE_OF_RANDOM_BYTES                       (28)
#define SIZE_OF_COOKIE                             (32)//255
#define SIZE_OF_SESSIONID                          (32)
#define NUMBER_OF_CIPHER_SUITES                    (1)
#define NUMBER_OF_COMPRESSION_METHOD               (1)
#define NUMBER_OF_CLIENT_CERTIFICATE_TYPES         (2)
#define NUMBER_OF_SERVER_CERTIFICATE_TYPES         (1)
#define NUMNER_OF_TX_EXTENSIONS                    (2)
#define NUMNER_OF_RX_EXTENSIONS                    (3)
#define EXTENSIONS_LENGTH                          ((NUMNER_OF_TX_EXTENSIONS * 4) + NUMBER_OF_CLIENT_CERTIFICATE_TYPES + 1 + NUMBER_OF_SERVER_CERTIFICATE_TYPES + 1)
#define VERIFY_DATA_LENGTH                         (40) //Minimum 12 bytes


typedef  uint16_t ProtocolVersion;                             
#define PROTOCOL_VERSION                           (0xFEFD)  //DTLS v1.2

typedef void (*handshake_tx_function_ptr)(void);
typedef int  (*handshake_rx_function_ptr)(void);

typedef void (*alert_function_ptr)(void);


//typedef void (*changeCipherSpec_tx_function_ptr)(void);
//typedef int (*changeCipherSpec_rx_function_ptr)();


typedef int (*sign_hash_function_ptr)(void);


typedef enum 
{
    HELLO_REQUEST = 0,
    CLIENT_HELLO = 1,
    SERVER_HELLO = 2,
    HELLO_VERIFY_REQUEST= 3,                          // New field
    CERTIFICATE = 11,
    SERVER_KEY_EXCHANGE =12,
    CERTIFICATE_REQUEST = 13,
    SERVER_HELLO_DONE =14,
    CERTIFICATE_VERIFY = 15,
    CLIENT_KEY_EXCHANGE = 16,
    FINISHED = 20
}HandshakeType_t;

typedef struct
{
    HandshakeType_t type;    
    handshake_tx_function_ptr txCallBack;
    handshake_rx_function_ptr rxCallBack;    
}handshake_handler_t;

//typedef struct
//{    
//    changeCipherSpec_tx_function_ptr txCallBack;
//    changeCipherSpec_rx_function_ptr rxCallBack;    
//}cipherSpec_handler_t;

//typedef struct 
//{
//    uint8_t major;
//    uint8_t minor;
//}ProtocolVersion;

typedef enum {
    CHANGE_CIPHER_SPEC = 20,
    ALERT= 21,
    HANDSHAKE = 22,
    APPLICATION_DATA =23
}ContentType;

typedef enum {
    ALERT_WARNING = 1,
    ALERT_FATAL = 2
    //255            
}AlertLevel_t;

typedef enum {
   CLOSE_NOTIFY=0,
   UNEXPECTED_MESSAGE=10,
   BAD_RECORD_MAC=20,
   DECRYPTION_FAILED_RESERVED=21,
   RECORD_OVERFLOW=22,
   DECOMPRESSION_FAILURE=30,
   HANDSHAKE_FAILURE=40,
   NO_CERTIFICATE_RESERVED=41,
   BAD_CERTIFICATE=42,
   UNSUPPORTED_CERTIFICATE=43,
   CERTIFICATE_REVOKED=44,
   CERTIFICATE_EXPIRED=45,
   CERTIFICATE_UNKNOWN=46,
   ILLEGAL_PARAMETER=47,
   UNKNOWN_CA=48,
   ACCESS_DENIED=49,
   DECODE_ERROR=50,
   DECRYPT_ERROR=51,
   EXPORT_RESTRICTION_RESERVED=60,
   ALERT_PROTOCOL_VERSION=70,
   INSUFFICIENT_SECURITY=71,
   INTERNAL_ERROR=80,
   USER_CANCELED=90,
   NO_RENEGOTIATION=100,
   UNSUPPORTED_EXTENSION=110    
}AlertDescriptionType_t;

typedef struct
{
    AlertLevel_t level;
    AlertDescriptionType_t description;
}Alert;

typedef struct
{
    AlertDescriptionType_t alertDescription;
    alert_function_ptr CallBack;    
}alert_handler_t;

typedef enum
{
     TLS_RSA_EXPORT_WITH_RC4_40_MD5 = 0x0003 ,
     TLS_RSA_WITH_RC4_128_MD5 = 0x0004,
     TLS_RSA_WITH_RC4_128_SHA = 0x0005,
     TLS_DH_anon_EXPORT_WITH_RC4_40_MD5 = 0x0017,
     TLS_DH_anon_WITH_RC4_128_MD5 = 0x0018,
     TLS_KRB5_WITH_RC4_128_SHA = 0x0020,
     TLS_KRB5_WITH_RC4_128_MD5 = 0x0024,
     TLS_KRB5_EXPORT_WITH_RC4_40_SHA = 0x0028,
     TLS_KRB5_EXPORT_WITH_RC4_40_MD5 = 0x002B,
     TLS_PSK_WITH_RC4_128_SHA= 0x008A,
     TLS_DHE_PSK_WITH_RC4_128_SHA = 0x008E,
     TLS_RSA_PSK_WITH_RC4_128_SHA = 0x0092,
     TLS_ECDH_ECDSA_WITH_RC4_128_SHA = 0xC002,
     TLS_ECDHE_ECDSA_WITH_RC4_128_SHA = 0xC007,
     TLS_ECDH_RSA_WITH_RC4_128_SHA = 0xC00C,         
     TLS_ECDHE_RSA_WITH_RC4_128_SHA = 0xC011,        
     TLS_ECDH_anon_WITH_RC4_128_SHA = 0xC016,      
     TLS_ECDHE_PSK_WITH_RC4_128_SHA = 0xC033,
     TLS_ECDH_ECDSA_WITH_AES_128_CCM_8 = 0xC0AE,
     TLS_PSK_WITH_AES_128_CCM_8 = 0xC0A8,
             TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 = 0xC023
}CipherSuites;

/** From http://www.iana.org/assignments/tls-extensiontype-values/tls-extensiontype-values.xhtml */
typedef enum
{
    SERVER_NAME = 0x0000,
    MAX_FRAGMENT_LENGTH,
    CLIENT_CERTIFICATE_URL,
    TRUSTED_CA_KEYS,
    TRUNCATED_HMAC,
    STATUS_REQUEST,
    USER_MAPPING,
    CLIENT_AUTHZ,
    SERVER_AUTHZ,
    CERT_TYPE,
    ELLIPTIC_CURVES, //SUPPORTED_CURVES
    EC_POINT_FORMATS,
    SECURE_REMOTE_PASSWORD,
    SIGNATURE_ALGORITHMS,
    USE_SRTP,
    HEARTBEAT,
    APPLICATION_LAYER_PROTOCOL_NEGOTIATION,
    STATUS_REQUEST_V2,
    SIGNED_CERTIFICATE_TIMESTAMP,
    CLIENT_CERTIFICATE_TYPE,
    SERVER_CERTIFICATE_TYPE,
    PADDING,
    ENECRYPT_THEN_MAC,
    EXTENDED_MASTER_MAC,
    EXTENDED_MASTER_SECRET,
    TOKEN_BINDING,
    CACHED_INFO
    /** 65535 extension types*/   
}ExtensionType;

typedef enum
{ 
    EXPLICIT_PRIME = 0x01,
    EXPLICIT_CHAR2 = 0x02,
    NAMED_CURVE = 0x03
    /*reserved 255*/
} ECCurveType;

/*******************************************************************************
 * RFC 4492
From http://infosecurity.ch/20100926/not-every-elliptic-curve-is-the-same-trough-on-ecc-security/
 * 
Curve names chosen by different standards organizations
------------+---------------+-------------
SECG        |  ANSI X9.62   |  NIST
------------+---------------+-------------
sect163k1   |               |   NIST K-163
sect163r1   |               |
sect163r2   |               |   NIST B-163
sect193r1   |               |
sect193r2   |               |
sect233k1   |               |   NIST K-233
sect233r1   |               |   NIST B-233
sect239k1   |               |
sect283k1   |               |   NIST K-283
sect283r1   |               |   NIST B-283
sect409k1   |               |   NIST K-409
sect409r1   |               |   NIST B-409
sect571k1   |               |   NIST K-571
sect571r1   |               |   NIST B-571
secp160k1   |               |
secp160r1   |               |
secp160r2   |               |
secp192k1   |               |
secp192r1   |  prime192v1   |   NIST P-192
secp224k1   |               |
secp224r1   |               |   NIST P-224
secp256k1   |               |
secp256r1   |  prime256v1   |   NIST P-256
secp384r1   |               |   NIST P-384
secp521r1   |               |   NIST P-521
------------+---------------+-------------
 ******************************************************************************/

typedef enum {
    SEC163K1 = 0x0001,
    SECT163R1 = 0x0002,
    SECT163R2 = 0x0003,           
    SECT193R1 = 0x0004,
    SECT193R2 = 0x0005,
    SECT233K1 = 0x0006,
    SECT233R1 = 0x0007, 
    SECT239K1 = 0x0008,
    SECT283K1 = 0x0009,
    SECT283R1 = 0x000A,
    SECT409K1 = 0x000B,
    SECT409R1 = 0x000C,
    SECT571K1 = 0x000B,
    SECT571R1 = 0x000E,
    SECP160K1 = 0x000F,
    SECP160R1 = 0x0010,
    SECP160R2 = 0x0011,
    SECP192K1 = 0x0012,
    SECP192R1 = 0x0013,
    SECP224K1 = 0x0014,
    SECP224R1 = 0x0015,
    SECP256K1 = 0x0016,
    SECP256R1 = 0x0017, //NIST P-256
    SECP384R1 = 0x00018,
    SECP521R1 = 0x0019,
    /* Reserved (0xFE00..0xFEFF) */
    ARBITRARY_EXPLICIT_PRIME_CURVES = 0XFF01,
    ARBITRARY_EXPLICIT_CHAR2_CURVES = 0XFF02
} NamedCurve;

typedef enum
{
    X509 = 0x00,
    OPENPGP = 0x01,
    RAW_PUBLIC_KEY = 0x02 
}CertificateType;

typedef struct
{
    uint32_t length;
    uint8_t Certificate[SERVER_CERTIFICATE_LENGTH];
}Certificate;

typedef struct
{
    ECCurveType type;
    uint16_t subType;
    PubKeyLengthType len;    
}CertificateExchange;

typedef struct 
{
    time_t gmt_unix_time;
    uint8_t random_bytes[SIZE_OF_RANDOM_BYTES];
}Random;

typedef struct
{
    uint16_t cipherSuiteLength;
    CipherSuites cipherSuite[NUMBER_OF_CIPHER_SUITES];
}CipherSuiteSet;

typedef struct
{
    uint8_t compression_methods_length;
    uint8_t compression_methods[NUMBER_OF_COMPRESSION_METHOD]; 
}CompressionMethod;

typedef enum 
{
    NONE,
    MD5,
    SHA1,
    SHA224,
    SHA256,
    SHA384,
    SHA512
}HashAlgorithm;

typedef enum
{
    ANONMYMOUS,
    RSA,
    DSA,
    ECDSA
}SignatureAlgorithm;

 typedef struct
 {
    HashAlgorithm hash;
    SignatureAlgorithm signature;
    sign_hash_function_ptr callBack;
}SignatureAndHashAlgorithm;

typedef struct
{
    uint16_t signatureLength;
    char signature[LENGHT_OF_SIGNATURE];
}Signature_t;


 typedef struct
 {
     uint8_t ecPointLength;
     uint8_t ecPointFormat;
 }ECPointFormats;

typedef struct
{
    CertificateType clientCertificateType;
    CertificateType serverCertificateType;
    ECPointFormats ecPointFormat;
       
}ExtensionValues;

typedef struct
{
    char verifyData[VERIFY_DATA_LENGTH];
}Finished;

typedef struct
{
    uint8_t cookieLength;
    char cookie[SIZE_OF_COOKIE];
}Cookie;

typedef struct
{
    uint8_t sessionIdLength;
    char sessionID[SIZE_OF_SESSIONID];    
}SessionID;

typedef struct
{
    ExtensionType extensionType;
    uint16_t extensionLength;     
}Extensions;

typedef struct
{
    HandshakeType_t messageType;
    uint32_t length;
    uint16_t messageSequence;
    uint32_t fragmentOffset;
    uint32_t fragmentLength;
}DTLS_Handshake;

typedef struct
{
    ProtocolVersion client_version;
    Random random;
    SessionID session_id;    
    Cookie client_cookie;           
}DTLS_ClientHello;

typedef struct
{
    ProtocolVersion server_version;
    Random random;
    SessionID session_id;  
    CipherSuites cipherSuite;
    uint8_t compression_methods;
}ServerHello;

typedef struct 
{
    ProtocolVersion server_version;
    Cookie server_cookie;
}DTLS_HelloVerifyRequest;
    

typedef struct 
{
    ContentType type;
    ProtocolVersion version;
    uint16_t epoch;                                     // New field
    uint8_t sequence_number[6];                           // New field
    uint16_t length; 
    //Followed by Fragment - similar to tls v1.2 - Block of Application data or Handshake Protocols (Handshake, Alert, ChangeCipherSpec)
} DTLSRecordLayer;



void DTLS_init(void);
void DTLS_Manage();
void DTLS_Handler(int length);
      




#endif	/* DTLS_CLIENT_H */

