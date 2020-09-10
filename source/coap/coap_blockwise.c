/*
 * File:   coap_blockwise.c
 * Author: C16008
 *
 * Created on January 16, 2017, 1:53 PM
 */
#include <stddef.h>
#include <time.h>
#include "coap_blockwise.h"
#include "graph.h"
#include "udpv4.h"
#include "coap.h"
#include "coap_options.h"
#include "network.h"
#include "data_model.h"

#define MAX_NUMBER_OF_BLOCK_CLIENTS  2
#define BLOCK_TIMEOUT ACK_TIMEOUT
#define COAP_DEFAULT_BLOCK_SIZE  COAP_BLOCK_SIZE_64_BYTE
#define COAP_DEFAULT_BLOCK2_OPTION_ENABLE_SIZE   1024

const uint16_t COAP_BlockMapSzxToBlockInBytes[7] ={16,32,64,128,256,512,1024};
//const uint8_t  COAP_BlockMapSzxToPowerOf2[7] ={4,5,6,7,8,9,10};    //jira: CAE_MCU8-5647

typedef enum
{
    COAP_BLOCK_SIZE_16_BYTE = 0,
    COAP_BLOCK_SIZE_32_BYTE = 1,
    COAP_BLOCK_SIZE_64_BYTE = 2,
    COAP_BLOCK_SIZE_128_BYTE = 3,
    COAP_BLOCK_SIZE_256_BYTE = 4,
    COAP_BLOCK_SIZE_512_BYTE = 5,
    COAP_BLOCK_SIZE_1024_BYTE = 6            
}coapBlockwiseSize_t;

typedef struct
{
    vertex_index_t lastLeafVertex;
    vertex_index_t lastPathVertex;
    vertex_index_t leafVertex;
    vertex_index_t pathVertex;
    bool endflag;   
    uint16_t discoveryOffset;
}DiscoveryAttributes_t;

typedef struct
{
    uint32_t destinationIpAddress;   
    uint16_t blockSizeInBytes;
    coapBlockwiseSize_t  szx;
    bool mBit;
    uint32_t blockNum;
    uint16_t totalLength;
    uint16_t lengthToBeTransmitted;     
    time_t packetTimeout;
    bool retransmission;
    uint16_t offset;
    void *prev;
    void *now;   
}BlockOptionParameters_t;

    
DiscoveryAttributes_t prevDisc[MAX_NUMBER_OF_BLOCK_CLIENTS];
DiscoveryAttributes_t nowDisc[MAX_NUMBER_OF_BLOCK_CLIENTS];
DiscoveryAttributes_t *now, *prev;

static BlockOptionParameters_t blockOptionParams[MAX_NUMBER_OF_BLOCK_CLIENTS];
static int blockClientIndex =0;
static time_t currentTime;
static volatile uint16_t rsrcDirtyFlag = 0;    //jira: CAE_MCU8-5647


static uint32_t COAP_ConvertBlockOptionToNetworkOrder(uint32_t val,uint8_t byteLength);
static void COAP_ResetBlockParams(uint8_t i);
static void COAP_ResetDiscoveryParams(uint8_t i);

static bool writeBlockOfBody(char * str,uint16_t len);
static bool COAP_ConfigBlockParams(uint8_t szx, uint32_t blockNum);
static int  COAP_FindClientIdxOrAddBlockClient(uint32_t destinationAddress);
static void COAP_FreeBlockClient(void);
static void clearResourceDirtyFlag(void);

static void setMBit(bool M);
static void setBlockSzx(coapBlockwiseSize_t szx); 
static void setBlockNum(uint32_t blockNum); 
static void setBlockInBytes(uint16_t blockBytes);

//static coapBlockwiseSize_t getBlockSzx(coapBlockwiseSize_t szx);         //jira: CAE_MCU8-5647
//static uint32_t getBlockNum(void);                                       //jira: CAE_MCU8-5647
static bool getMBit(void); 
static uint32_t getCoAPBlockOptionValue(uint8_t szx, bool m,uint8_t num);



/********************** Block Transmit API ***********************************/
static bool writeBlockOfBody(char * str,uint16_t len)
{  
   blockOptionParams[(uint8_t)blockClientIndex].lengthToBeTransmitted = 0; //jira: CAE_MCU8-5647
   blockOptionParams[(uint8_t)blockClientIndex].offset = 0;                //jira: CAE_MCU8-5647
     blockOptionParams[(uint8_t)blockClientIndex].totalLength +=len;       //jira: CAE_MCU8-5647
     if(blockOptionParams[(uint8_t)blockClientIndex].totalLength >= blockOptionParams[(uint8_t)blockClientIndex].blockSizeInBytes)     //jira: CAE_MCU8-5647
     { 
        blockOptionParams[(uint8_t)blockClientIndex].offset = blockOptionParams[(uint8_t)blockClientIndex].totalLength - blockOptionParams[(uint8_t)blockClientIndex].blockSizeInBytes;  //jira: CAE_MCU8-5647
        blockOptionParams[(uint8_t)blockClientIndex].lengthToBeTransmitted = len - blockOptionParams[(uint8_t)blockClientIndex].offset; //jira: CAE_MCU8-5647
        UDP_WriteBlock(str,blockOptionParams[(uint8_t)blockClientIndex].lengthToBeTransmitted);                                         //jira: CAE_MCU8-5647
        blockOptionParams[(uint8_t)blockClientIndex].totalLength = 0;                                                                   //jira: CAE_MCU8-5647
        blockOptionParams[(uint8_t)blockClientIndex].packetTimeout = (time(&currentTime) + BLOCK_TIMEOUT);                              //jira: CAE_MCU8-5647
        return true;
     }
     blockOptionParams[(uint8_t)blockClientIndex].lengthToBeTransmitted = len;                //jira: CAE_MCU8-5647
     UDP_WriteBlock(str,blockOptionParams[(uint8_t)blockClientIndex].lengthToBeTransmitted);  //jira: CAE_MCU8-5647
     memset(str,0,strlen(str));
     return false;
}


 /********************** Discovery Handler using Block2 Option ***********************************/

void COAP_DiscoveryBlockTransmit(coapReceive_t *ptr)
{    
    graph_iterator leafIterator,pathIterator;
    uint32_t block2OptionValue;
    uint16_t block2OptionLength;   
    vertex_t v;
    uint8_t lenOfStr;
    uint8_t byteToWrite;
    
    blockClientIndex = COAP_FindClientIdxOrAddBlockClient(ptr->destIP); //Required when Discovery with Block Option not included
    blockOptionParams[(uint8_t)blockClientIndex].prev = &prevDisc[(uint8_t)blockClientIndex];   //jira: CAE_MCU8-5647
    blockOptionParams[(uint8_t)blockClientIndex].now = &nowDisc[(uint8_t)blockClientIndex];     //jira: CAE_MCU8-5647
    now = blockOptionParams[(uint8_t)blockClientIndex].now;                                     //jira: CAE_MCU8-5647
    prev = blockOptionParams[(uint8_t)blockClientIndex].prev;                                   //jira: CAE_MCU8-5647
    
    blockOptionParams[(uint8_t)blockClientIndex].totalLength = 0;                               //jira: CAE_MCU8-5647
    
    now->discoveryOffset = blockOptionParams[(uint8_t)blockClientIndex].offset;                 //jira: CAE_MCU8-5647
    char str[MAX_WORD_SIZE+3]; //Buffer to hold upto '<' + MAX_WORD_SIZE + '/'  
   
   
    setMBit(true);
    block2OptionValue = getCoAPBlockOptionValue((uint8_t)(blockOptionParams[(uint8_t)blockClientIndex].szx),true,blockOptionParams[(uint8_t)blockClientIndex].blockNum); //jira: CAE_MCU8-5647
    sprintf(str,"%lu",block2OptionValue);      
    COAP_AddBlock2Options(str); //more flag unset   
    
    
    UDP_Write8(0xFF); 
    if(blockOptionParams[(uint8_t)blockClientIndex].retransmission == false || blockOptionParams[(uint8_t)blockClientIndex].blockNum == 0)   //jira: CAE_MCU8-5647
    {
        prev->endflag = now->endflag;        
        prev->lastLeafVertex = now->lastLeafVertex;
        prev->lastPathVertex = now->lastPathVertex;
        prev->leafVertex = now->leafVertex;
        prev->pathVertex = now->pathVertex;
        prev->discoveryOffset = now->discoveryOffset;
    }   

     memset(str,0,sizeof(str));
     
    if(blockOptionParams[(uint8_t)blockClientIndex].retransmission == true)   //jira: CAE_MCU8-5647
    {       
        now->endflag = prev->endflag;        
        now->lastLeafVertex = prev->lastLeafVertex;
        now->lastPathVertex = prev->lastPathVertex;
        now->leafVertex = prev->leafVertex;
        now->pathVertex = prev->pathVertex;  
        now->discoveryOffset = prev->discoveryOffset;
    }
     
     if(now->discoveryOffset > 0)
     {
         if(now->endflag == false)
         {
             //Get Path vertex Info             
             graph_getVertexAtIndex(&v, now->pathVertex); 
             if(now->discoveryOffset > 1)
             {
                 strncpy(str,string_getWordAtIndex(v.nameIdx),MAX_WORD_SIZE); 
                 lenOfStr = strlen(str);
                 byteToWrite = lenOfStr + 1u - now->discoveryOffset; //'+1 is for "/"    //jira: CAE_MCU8-5647
                 strncpy(str,&string_getWordAtIndex(v.nameIdx)[byteToWrite],now->discoveryOffset);                
             }
             strncat(str,"/",1);
             writeBlockOfBody(str,strlen(str));                     
         }
         else
         {
             writeBlockOfBody((char *)(">,"),2);      //jira: CAE_MCU8-5647
         }
     }
     
    graph_makeLeafIterator(&leafIterator);
    while((now->leafVertex = graph_nextVertex(&leafIterator))!=-1)
    {     
        if(now->leafVertex >= now->lastLeafVertex)
        {            

            graph_makePathIterator(&pathIterator, now->leafVertex);
           

            if(now->leafVertex != now->lastLeafVertex)
                strncpy(str,"<",1);           

            while((now->pathVertex = graph_nextVertex(&pathIterator))!=-1)
            {                        
                if(now->pathVertex > now->lastPathVertex)                
                {                   
                    
                    graph_getVertexAtIndex(&v, now->pathVertex);                      
                    strncat(str,string_getWordAtIndex(v.nameIdx),strlen(string_getWordAtIndex(v.nameIdx)));   
                    strncat(str,"/",1);               
                    if(writeBlockOfBody(str,strlen(str)))
                    {                 
                        now->lastLeafVertex = now->leafVertex;
                        now->lastPathVertex = now->pathVertex; 
                        now->endflag = false;
                        return;
                    }                     
                }
            }            
            now->endflag = true;           
            memset(str,0,sizeof(str));
            strncpy(str,">,",2);               
            if(writeBlockOfBody(str,strlen(str)))
            { 
                 now->lastLeafVertex = now->leafVertex;
                 now->lastPathVertex = now->pathVertex;
                return;
            }
            now->lastLeafVertex = now->leafVertex = 0;
            now->lastPathVertex = now->pathVertex = 0;         
        }

    }
    
    if(now->leafVertex == -1)
    {
        if(now->endflag==false)
        {
            now->endflag=true;
            strncat(str,">,",2);            
            if(writeBlockOfBody(str,strlen(str)))
            { 
                return;
            }
        }
        setMBit(false);
        block2OptionValue = getCoAPBlockOptionValue((uint8_t)(blockOptionParams[(uint8_t)blockClientIndex].szx),false,blockOptionParams[(uint8_t)blockClientIndex].blockNum);   //jira: CAE_MCU8-5647
        block2OptionLength= COAP_GetLengthOfIntegerOptionValue(block2OptionValue);       
        block2OptionValue = COAP_ConvertBlockOptionToNetworkOrder(block2OptionValue,block2OptionLength); //Change to network Order 
        COAP_InsertLastOptionValue(ptr,(char *)&block2OptionValue,block2OptionLength);                      //jira: CAE_MCU8-5647
        blockOptionParams[(uint8_t)blockClientIndex].packetTimeout = (time(&currentTime) +BLOCK_TIMEOUT );  //jira: CAE_MCU8-5647    
    }   
}

 
 bool COAP_ProcessBlock2Option(uint32_t destinationAddress, char *value,uint16_t block2Length)
 {
    bool ret = 0;
    uint32_t block2OptionValue =0;   
    uint8_t last4Bits =0;
    uint32_t blockNum =0;
    bool mBit = false;
    uint8_t szx = 0;
    
    
    blockClientIndex = COAP_FindClientIdxOrAddBlockClient(destinationAddress);
    
    //Currently do not Process - Try Again
    if(blockClientIndex == -1)
        return 0;
     
     while(block2Length--)
     {
         ((uint8_t *)&block2OptionValue)[block2Length] = *value++;        
     }    
    last4Bits = (((uint8_t *)&block2OptionValue)[0] & 0x0Fu);      //jira: CAE_MCU8-5647
    mBit = (bool)((last4Bits & 0x08u)?true :false);                //jira: CAE_MCU8-5647
    szx = mBit ? 1u : last4Bits;                                   //jira: CAE_MCU8-5647
    blockNum = block2OptionValue >> 4;
    if(szx > 6 || mBit == true) //szx > 6 are reserved  and Currently, Block2 receive for Clients are NOT supported
        return 0;    
    

    ret = COAP_ConfigBlockParams(szx, blockNum);
    
    return ret;     
 }
 
 static int COAP_FindClientIdxOrAddBlockClient(uint32_t destinationAddress)
 {     
     int ret = -1;
     uint8_t emptyIdx =0;
     for(uint8_t i =0; i < MAX_NUMBER_OF_BLOCK_CLIENTS; i++)
     {
         if(blockOptionParams[i].destinationIpAddress == destinationAddress)
         {
             ret = i;
             break;
         }
         else if(blockOptionParams[i].destinationIpAddress == 0)
             emptyIdx = i;
     }
     if(ret == -1 && emptyIdx < MAX_NUMBER_OF_BLOCK_CLIENTS)
     {
        blockOptionParams[emptyIdx].destinationIpAddress = destinationAddress; 
        ret = emptyIdx;
        rsrcDirtyFlag++;
     }
     return ret;
     
 }
 
static void COAP_FreeBlockClient(void)
 {    
     for(uint8_t i =0; i < MAX_NUMBER_OF_BLOCK_CLIENTS; i++)
     {
         if(blockOptionParams[i].packetTimeout == time(&currentTime))         
         {
             blockOptionParams[i].packetTimeout  = 0;
             blockOptionParams[i].destinationIpAddress = 0;            
             blockOptionParams[i].blockNum = 0;
             blockOptionParams[i].szx = COAP_DEFAULT_BLOCK_SIZE;
             blockOptionParams[i].mBit = true;
             blockOptionParams[i].blockSizeInBytes = COAP_BlockMapSzxToBlockInBytes[(uint8_t)(blockOptionParams[i].szx)];    //jira: CAE_MCU8-5647
             memset(&prevDisc[i],0,sizeof(prevDisc));
             memset(&nowDisc[i],0,sizeof(nowDisc));
        

             blockOptionParams[i].prev = &prevDisc[i];
             blockOptionParams[i].now = &nowDisc[i];    
             blockOptionParams[i].totalLength =0;
             blockOptionParams[i].lengthToBeTransmitted =0; 
             rsrcDirtyFlag--;                         
          }
     } 
}
 
 /********************** Block Option Setter APIs ***********************************/
 
 static void setBlockSzx(coapBlockwiseSize_t szx)
 {
     blockOptionParams[(uint8_t)blockClientIndex].szx = szx;           //jira: CAE_MCU8-5647
 }
 
 static void setBlockNum(uint32_t blockNum)
 {
     blockOptionParams[(uint8_t)blockClientIndex].blockNum = blockNum; //jira: CAE_MCU8-5647
 }
 
 static void setMBit(bool M)
 {
     blockOptionParams[(uint8_t)blockClientIndex].mBit = M;            //jira: CAE_MCU8-5647
 }
 
 static void setBlockInBytes(uint16_t blockBytes)
 {
     blockOptionParams[(uint8_t)blockClientIndex].blockSizeInBytes = blockBytes; //jira: CAE_MCU8-5647
 }
 
 
 /********************** Block Option Getter APIs ***********************************/
 //jira: CAE_MCU8-5647
// static coapBlockwiseSize_t getBlockSzx(coapBlockwiseSize_t szx)
// {
//     return blockOptionParams[blockClientIndex].szx;
// }
 //jira: CAE_MCU8-5647
// static uint32_t getBlockNum(void)
// {
//     return blockOptionParams[blockClientIndex].blockNum;
// }
 
 static bool getMBit(void)
 {
     return blockOptionParams[(uint8_t)blockClientIndex].mBit;   //jira: CAE_MCU8-5647
 }
 
static uint32_t getCoAPBlockOptionValue(uint8_t szx, bool m,uint8_t num)
{
    uint32_t value = 0;
    int last4Bits;
    
    last4Bits= szx | (m ? 1<<3 : 0);
    if (num == 0 && !m && szx==0)
    {        
        return 0;
        
    }    
    else{
        value   = (uint32_t)((last4Bits) | (num << 4u));     //jira: CAE_MCU8-5647
        return value;
    }
           
    
}
 
 
 /********************** Block Option Map APIs ***********************************/
static uint32_t COAP_ConvertBlockOptionToNetworkOrder(uint32_t val,uint8_t byteLength)
{
    switch(byteLength)
    {
        case 1:
            break;
        case 2:
            val = htons(val);
            break;
        case 3:
            val = convert_hton24(val);         
            break;
        case 4:
            val = htonl(val);
            break;
        default:
            break;
    }
    return val;                    
}
  
void COAP_ConfigDefaultBlockParams(void)
{ 
    for(uint8_t i =0; i < MAX_NUMBER_OF_BLOCK_CLIENTS; i++)
    {
        COAP_ResetBlockParams(i);
        COAP_ResetDiscoveryParams(i);
              
    
    }
}

static void COAP_ResetBlockParams(uint8_t i)
{
    blockOptionParams[i].packetTimeout = 0;
    blockOptionParams[i].blockNum = 0;
    blockOptionParams[i].szx = COAP_DEFAULT_BLOCK_SIZE;
    blockOptionParams[i].mBit = true;
    blockOptionParams[i].blockSizeInBytes = COAP_BlockMapSzxToBlockInBytes[(uint8_t)blockOptionParams[i].szx];       //jira: CAE_MCU8-5647
    blockOptionParams[i].totalLength =0;
    blockOptionParams[i].lengthToBeTransmitted =0;
    blockOptionParams[i].retransmission = false;
}

static void COAP_ResetDiscoveryParams(uint8_t i)
{
    memset(&prevDisc[i],0,sizeof(prevDisc));
    memset(&nowDisc[i],0,sizeof(nowDisc));
    prevDisc[i].lastPathVertex = -1;
    prevDisc[i].endflag = true;
    nowDisc[i].lastPathVertex = -1;
    nowDisc[i].endflag = true;    
    
}


  
static bool COAP_ConfigBlockParams(uint8_t szx, uint32_t blockNum)
{
    uint32_t expectedBlock;
    
    if(blockOptionParams[(uint8_t)blockClientIndex].szx <= szx)          //jira: CAE_MCU8-5647
    {         
        setBlockSzx(szx);    
        setBlockInBytes(COAP_BlockMapSzxToBlockInBytes[szx]);        
        
        if(blockNum == blockOptionParams[(uint8_t)blockClientIndex].blockNum)   //jira: CAE_MCU8-5647
        {
            setBlockNum(blockNum); //Retransmission or Early Negotiation              
            blockOptionParams[(uint8_t)blockClientIndex].retransmission = true; //jira: CAE_MCU8-5647
            return 1; 
        }
        else if(blockNum == blockOptionParams[(uint8_t)blockClientIndex].blockNum+1) //jira: CAE_MCU8-5647
        {
            if(getMBit() == 0) //last Block is sent - No more blocks to be transmitted
                return 0;
            blockOptionParams[(uint8_t)blockClientIndex].retransmission = false;     //jira: CAE_MCU8-5647
            setBlockNum(blockNum);
            return 1; 
        }
        else
        {
            return 0;      
        }          
    }
    else if(blockOptionParams[(uint8_t)blockClientIndex].szx > szx)           //jira: CAE_MCU8-5647
    {
        if(blockOptionParams[(uint8_t)blockClientIndex].blockNum == blockNum) //jira: CAE_MCU8-5647
        {
             setBlockNum(blockNum); //Retransmission or Early Negotiation
             blockOptionParams[(uint8_t)blockClientIndex].retransmission = true; //jira: CAE_MCU8-5647
             setBlockSzx(szx);    
             setBlockInBytes(COAP_BlockMapSzxToBlockInBytes[szx]);  
             return 1;
        }
        
        else if(blockOptionParams[(uint8_t)blockClientIndex].blockNum < blockNum) //Late Negotiation  //jira: CAE_MCU8-5647
        {
            //Division is efficient than Right Shift  
            expectedBlock = (COAP_BlockMapSzxToBlockInBytes[(uint8_t)(blockOptionParams[(uint8_t)blockClientIndex].szx)]/COAP_BlockMapSzxToBlockInBytes[szx]);  //jira: CAE_MCU8-5647
//            expectedBlock = (COAP_BlockMapSzxToBlockInBytes[blockOptionParams[blockClientIndex].szx] >> COAP_BlockMapSzxToPowerOf2[szx]);
            if( blockNum ==expectedBlock )
            {                 
                setBlockNum(blockNum);
                setBlockSzx(szx);    
                setBlockInBytes(COAP_BlockMapSzxToBlockInBytes[szx]);    
                blockOptionParams[(uint8_t)blockClientIndex].retransmission = false;   //jira: CAE_MCU8-5647
                return 1;
            }              
        }
        else
        {
            return 0;
        }
        setBlockSzx(szx); 
        setBlockInBytes(COAP_BlockMapSzxToBlockInBytes[szx]);
    }
    else 
    {        
        return 0;
    }  
    return 0;   //jira: CAE_MCU8-5647
}
  
void COAP_BlockClientManage(void)
{
    COAP_FreeBlockClient();
   
}

bool isBlockOptionRequired(void)
{
    if(getDiscoveryLength() > COAP_DEFAULT_BLOCK2_OPTION_ENABLE_SIZE)
        return true;   
    else    
        return false;   
}

bool isResourceDataDirty(void)
{
    if(rsrcDirtyFlag == 0)
        return false;
    else
        return true;
}


 