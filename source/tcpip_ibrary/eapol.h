/* 
 * File:   eapol.h
 * Author: c14210
 *
 * Created on April 24, 2015, 3:01 PM
 */

#ifndef EAPOL_H
#define	EAPOL_H

//EAPoLoL: Extensible Authentication Protocol over Lan for 802.3x



typedef enum
{
    EAPoL_PACKET,
    EAPoL_START,
    EAPoL_LOGOFF,
    EAPoL_KEY,
    EAPoL_ENCAPSULATED_ASF_ALERT

}eap_packet_type_t;

typedef error_msg (*createEapPacketFuncPtr)(void);


error_msg EAP_Send_Start_Packet(void);

bool EAP_Authentication_Status(void);
void EAPoL_Packet(void);


#endif	/* EAPoLOL_H */

