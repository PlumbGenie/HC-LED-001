#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ethernet_driver.h"
#include "mac_address.h"
#include "hardware.h"
#include "log.h"
#include "tcpip_types.h"
#include "network.h"
#include "eapol.h"
#include "md5.h"

#define EAPoL_PROTOCOL_VER  0x01

#define ARRAYSIZE(a)    (sizeof(a) / sizeof(*(a)))

static const mac48Address_t Eap_Destination_Mac_Addr={0x01,0x80,0xC2,0x00,0x00,0x03};
uint8_t last_id;

unsigned char username[]="local_user";
unsigned char password[]="local_pwd";
MD5_CTX mdContext;

static bool authenticated = false;
unsigned char challenge[16],md5Digest[16];

typedef struct
{
    uint8_t     code;
    uint8_t     id;
    uint16_t    length;
    uint8_t     data_type;
    uint8_t     *data;

}eap_packet_format_t;
eap_packet_format_t eapPacket;



typedef enum
{
    EAPoL_ZERO,
    EAPoL_REQUEST,
    EAPoL_RESPONSE,
    EAPoL_SUCCESS,
    EAPoL_FAILURE
            
}eap_code_type_t;

typedef enum
{
    eap_reserved,
    identity,
    notification,
    nak,
    md5Challenge,
    oneTimepassword,
    genericTokenCard,
    expandTypes=254,
    experimentalUse=255

}eap_packet_data_type_t;

error_msg eapolProcessPacket(void);
error_msg eapolSendPacket(uint8_t packet_type,uint8_t code_type, uint8_t packet_subtype);

error_msg EAP_Send_Start_Packet(void)
{
    error_msg ret;
    ret=eapolSendPacket(EAPoL_START,0,0);
    return ret;
}


bool EAP_Authentication_Status(void)
{
    return authenticated;
}



/*******************************************Send an EAP Packet****************************************/

error_msg eapolSendPacket(uint8_t packet_type, uint8_t code_type, uint8_t packet_subtype)//subtype needed in some cases else just put a zero there
{
    error_msg ret=ERROR;
    uint8_t ID[]={0x6d,0x69,0x63,0x72,0x6f,0x63,0x68,0x69,0x70}; //create an id retrieving function

    ret = ETH_WriteStart(&Eap_Destination_Mac_Addr, ETHERTYPE_EAPoL);
    if(ret==SUCCESS)
    {
        uint8_t x;
//        const eap_packet_handler_t *hptr;

        ETH_Write8(EAPoL_PROTOCOL_VER);
        //packet type: 0 for an EAPoLoL Packet
        ETH_Write8(packet_type);

        switch(packet_type)
        {
            case EAPoL_START:
            case EAPoL_LOGOFF:
                ETH_Write16(0);
                break;
            case EAPoL_PACKET:
                switch(packet_subtype)
                {
                    case (eap_packet_data_type_t)identity:
                        // id has to have a proper place to be defined by the user may be a function call
                        ETH_Write16(sizeof(ID)+5); //Length plus other fields
                        ETH_Write8(code_type);// 2 for response
                        ETH_Write8(eapPacket.id);
                        ETH_Write16(sizeof(ID)+5);
                        ETH_Write8(packet_subtype);//identity packet
                        ETH_WriteBlock(&ID,sizeof(ID));//block of data, ID in this case
                        break;
                    case (eap_packet_data_type_t)md5Challenge:
                        ETH_Write16(sizeof(md5Digest)+6);
                        ETH_Write8(code_type);
                        ETH_Write8(eapPacket.id);
                        ETH_Write16(sizeof(md5Digest)+6);
                        ETH_Write8(packet_subtype);
                        ETH_Write8(0x10); //16 bytes of md5 data
                        ETH_WriteBlock(&md5Digest, sizeof(md5Digest));

//                        ETH_Write16(sizeof(mdContext.digest)+6);
//                        ETH_Write8(code_type);
//                        ETH_Write8(eapPacket.id);
//                        ETH_Write16(sizeof(mdContext.digest)+6);
//                        ETH_Write8(packet_subtype);
//                        ETH_Write8(0x10); //16 bytes of md5 data
//                        ETH_WriteBlock(&mdContext.digest, sizeof(mdContext.digest));


                        break;
                    case (eap_packet_data_type_t)nak:
                        break;
                    case (eap_packet_data_type_t)notification:                     
                        break;
                }
            case EAPoL_KEY:
                break;
            case EAPoL_ENCAPSULATED_ASF_ALERT:
                break;

        }
    }
    if(ret==SUCCESS)
    {
        ret=ETH_Send();
    }
    return ret;

}

//error_msg createEAPPacket(void)
//{
//
//}


/*******************************************Receive and Process an EAP Packet****************************************/

void EAPoL_Packet(void)
{
    uint8_t contentType;
    ETH_Read8();//version
    contentType=ETH_Read8();//type

    switch(contentType)
    {
        case EAPoL_PACKET:
            //Lets Figure out the type of this packet
            eapolProcessPacket();
            break;
        case EAPoL_START:
            //As an authenticatee I am not sure if we will ever process this packet
            break;
        case EAPoL_LOGOFF:
            break;
        case EAPoL_KEY:
            break;
        case EAPoL_ENCAPSULATED_ASF_ALERT:
            break;
        default:
            break;
    }
}

error_msg eapolProcessPacket(void)
{
    uint8_t packetLength,typeOfData,rawDigestSize;
    unsigned char raw_digest[]; //allocating 16 byte max for the username and password
    uint16_t x;

    packetLength        = ETH_Read16(); // includes code(1byte), id(1 byte), length(2 bytes) and payload (total max cant be more than 2500 bytes)
    eapPacket.code      = ETH_Read8();
    eapPacket.id        = ETH_Read8();
    eapPacket.length    = ETH_Read16();
    eapPacket.data_type = ETH_Read8();
    packetLength       -= 5;


    for (x=0; x<packetLength; x++)
    {
        challenge[x] = ETH_Read8();

    }
    eapPacket.data = challenge;

    //for(x=0; x<sizeof(raw_digest);x++) raw_digest[x]=0;


    switch(eapPacket.code)
    {

        case EAPoL_REQUEST:
            switch(eapPacket.data_type)
            {
                case identity:
                    eapolSendPacket((eap_packet_type_t)EAPoL_PACKET,(eap_code_type_t)EAPoL_RESPONSE,(eap_packet_data_type_t)identity);
                    break;
                case notification:
                    break;
                case md5Challenge:
                    //Initialize MD5
                    MD5Init(&mdContext);

                    //create the full raw digest value (id, login, password, challenge concatenated)

                    //strcpy(raw_digest,"shaimahusain");

//                    strncpy(raw_digest,&(eapPacket.id),sizeof(eapPacket.id));
//                    strncat(raw_digest,username,sizeof(username));
//                    strncat(raw_digest,password,sizeof(password));
//                    strncat(raw_digest,eapPacket.data,sizeof(challenge));



//                    rawDigestSize = strlen(raw_digest);//sizeof(eapPacket.id)+sizeof(username)+sizeof(password)+sizeof(challenge);

                    MD5Update(&mdContext,raw_digest,strlen(raw_digest));
//                    MD5Final(&mdContext);
                    MD5Final(&mdContext,md5Digest);
                    eapolSendPacket((eap_packet_type_t)EAPoL_PACKET,(eap_code_type_t)EAPoL_RESPONSE,(eap_packet_data_type_t)md5Challenge);
                    break;
                default :
                    break;
            }
            break;
        case EAPoL_RESPONSE:
            switch(eapPacket.data[0])
            {
                case identity:
                    break;
                case notification:
                    break;
                case nak:
                    break;
                case md5Challenge:
                    break;
                default :
                    break;
            }
            break;
        case EAPoL_SUCCESS: //authentication successful
            authenticated = true;
            break;
        case EAPoL_FAILURE: //authentication failed
            authenticated = false;
            break;
        default:
            break;
    }
    return SUCCESS;

}
