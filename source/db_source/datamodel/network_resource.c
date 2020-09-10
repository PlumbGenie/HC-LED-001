/**
  Network_resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    network_resource.c

  Summary:
     This is the implementation of Network_resource.

  Description:
    This source file provides the implementation of the API for the Network_resource.

 */

/*

?  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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

/**
 Section: Included Files
 */
#include "network_resource.h"
#include "tcpip_types.h"
#include "object_heap.h"
//#include "coap_contentFormat.h"
#include "mac_address.h"
#include "ip_database.h"

#define NETWORK_EEP_ELEMENTS 10
#define SUPPORTED_FORMATS_COUNT 1
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))


const net_lst net_items_list[6] = {

    {"madt", MADT},
    {"madr", MADR},
    {"mdns", MDNS},
    {"edns", EDNS},
    {"dns", DNS },
    {"ntp", NTP }
};

obj_index_t network_index_object_index;
static error_msg writeToEthNetworkData(network_eeprom_t *p);


obj_index_t initNetworkResource(void)
{
    network_eeprom_t ne;    
   memset(&ne,0,sizeof(ne));  
   
   network_index_object_index = obj_malloc(sizeof(network_eeprom_t),"RSRC");
    
    ne.mgmtAddressType = string_insertWord("1",ne.mgmtAddressType);
    ne.mgmtAddress = string_insertWord("192.168.0.2",ne.mgmtAddress);
    ne.mgmtDNSName=string_insertWord("Building1_BMS",ne.mgmtDNSName);    
    ne.endptMacAddress=string_insertWord("d8:80:39:88:19:b8",ne.endptMacAddress);    
    ne.endptAddressType = string_insertWord("1",ne.endptAddressType);    
    ne.endptAddress =string_insertWord("192.168.1.44",ne.endptAddress);    
    ne.endptDNSName = string_insertWord("light17.example.com",ne.endptDNSName);    
    ne.endptDNSServer = string_insertWord("0.0.0.0",ne.endptDNSServer);
    ne.endptNTPServer =string_insertWord("59.72.0.0",ne.endptNTPServer);
       
    obj_setObjectAtIndex(network_index_object_index, (const void *)&ne);   
     
    return network_index_object_index;
}


error_msg networkInit(void *ptr, obj_index_t objIdx)  //jira: CAE_MCU8-5647
{    
    return SUCCESS;   
}

error_msg networkGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;    
    static uint8_t networkElements;
    network_eeprom_t p;    
  
    
    networkElements = NETWORK_EEP_ELEMENTS;  
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    /**** Use Encode to write data to the buffer ****/
    
    writeAuxData(&networkElements,MAP);      
    
    Get_ToEthernet((char *)"uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    ret = writeToEthNetworkData(&p);   
    
    return ret;

}

static error_msg writeToEthNetworkData(network_eeprom_t *p)
{
    const mac48Address_t *mac;
    char mac_addrr[18]; 
    
    mac = MAC_getAddress();
    memset(mac_addrr,0,sizeof(mac_addrr));
    sprintf(mac_addrr,"%02x:%02x:%02x:%02x:%02x:%02x",mac->s.byte1,mac->s.byte2,mac->s.byte3,mac->s.byte4,mac->s.byte5,mac->s.byte6);
 
    Get_ToEthernet((char *)"madt",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->mgmtAddressType),TEXT_STRING);    

    Get_ToEthernet((char *)"madr",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->mgmtAddress),TEXT_STRING);    

    Get_ToEthernet((char *)"mdns",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->mgmtDNSName),TEXT_STRING);    
   
    Get_ToEthernet((char *)"emac",TEXT_STRING);
    Get_ToEthernet(mac_addrr,TEXT_STRING);    

    Get_ToEthernet((char *)"eadt",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->endptAddressType),TEXT_STRING);    
    
    Get_ToEthernet((char *)"eadr",TEXT_STRING);
    Get_ToEthernet(makeIpv4AddresstoStr(ipdb_getAddress()),TEXT_STRING);  

    Get_ToEthernet((char *)"edns",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->endptDNSName),TEXT_STRING);  
    
    Get_ToEthernet((char *)"dns",TEXT_STRING);
    Get_ToEthernet(makeIpv4AddresstoStr(ipdb_getDNS()),TEXT_STRING); 
    
    Get_ToEthernet((char *)"ntp",TEXT_STRING);
    Get_ToEthernet(makeIpv4AddresstoStr(ipdb_getNTP()),TEXT_STRING); 
    
    return SUCCESS;
}

error_msg networkPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{  
    error_msg ret =  ERROR;   
   
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;
    uint8_t i;
    menu_items_t mid;
    const net_lst *choice = NULL;
    choice = net_items_list;
    uint8_t payload_length;
    network_eeprom_t p;  
     uint32_t value;

    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    payload_length = PayloadLength;
    
    while(payload_length)
    {
        len =0;
        len += parseData(read_ptr,sizeof(read_ptr));
        for(i = 0, choice = NULL; i < ARRAY_SIZE(net_items_list); i++)
        {
            if (strncmp(read_ptr, net_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0)
            {
                choice = net_items_list + i;
                break;
            }
        }           
        memset(read_ptr,0,sizeof(read_ptr));
        mid = choice ? choice->id : UNKNOWN;     
        switch(mid)
        {            
            case MADT:
                len += parseData(read_ptr,sizeof(read_ptr));
                 p.mgmtAddressType = string_insertWord(read_ptr,p.mgmtAddressType);
                break;
            case MADR:
                len += parseData(read_ptr,sizeof(read_ptr));
                 p.mgmtAddress = string_insertWord(read_ptr,p.mgmtAddress);
                break;
            case MDNS:
                len += parseData(read_ptr,sizeof(read_ptr));
                 p.mgmtDNSName = string_insertWord(read_ptr,p.mgmtDNSName);
                break;
            case EDNS:
                len += parseData(read_ptr,sizeof(read_ptr));
                 p.endptDNSName = string_insertWord(read_ptr,p.endptDNSName);
                break;
            case DNS:
                len += parseData(read_ptr,sizeof(read_ptr));                   
                value = makeStrToIpv4Address(read_ptr);
                ipdb_setDNS(0,value);                                  
                value = 0;
                break;
            case NTP:
                len += parseData(read_ptr,sizeof(read_ptr));                
                value = makeStrToIpv4Address(read_ptr);
                ipdb_setNTP(0,value);                   
                value = 0;
                break;           
            default:
               len += parseData(read_ptr,sizeof(read_ptr));
               break;
        }       
        payload_length -= len;
    }
    obj_setObjectAtIndex(objIdx, (const void *)&p);
    return SUCCESS;        
}

error_msg networkPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg networkPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg networkDeleter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    network_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret=ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.mgmtAddressType;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.mgmtAddress;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.mgmtDNSName;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.endptMacAddress;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.endptAddressType;
    ret = string_removeWord(stringIdxToBeRemoved);   
    
    stringIdxToBeRemoved = p.endptAddress;
    ret = string_removeWord(stringIdxToBeRemoved);   
    
    stringIdxToBeRemoved = p.endptDNSName;
    ret = string_removeWord(stringIdxToBeRemoved);   
    
    stringIdxToBeRemoved = p.endptDNSServer;
    ret = string_removeWord(stringIdxToBeRemoved); 
    
    stringIdxToBeRemoved = p.endptNTPServer;
    ret = string_removeWord(stringIdxToBeRemoved);   
   

    return ret;
}



restfulMethodStructure_t networkMethods = {networkInit,networkGetter,networkPutter,networkPatcher,networkDeleter,networkPoster,initNetworkResource};
