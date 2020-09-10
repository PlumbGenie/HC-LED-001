/**
  Context Resource implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    context.c

  Summary:
     This is the implementation of Context Resource.

  Description:
    This source file provides the implementation of the API for the Context Resource.

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
#include "context.h"
#include "tcpip_types.h"
#include "object_heap.h"
#include "graph.h"
#include "contentFormat.h"


#define CONTEXT_EEP_ELEMENTS 5

obj_index_t context_index_object_index;
//static uint8_t elmnt_count = MAX_CONTEXT_KEYWORDS;  //jira: CAE_MCU8-5647

static error_msg writeToEthContextData(context_eeprom_t *p);
static uint8_t readFromEthContextData(obj_index_t objIdx, menu_items_t mid, context_eeprom_t *p);

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

const context_lst cont_items_list[4] = {
  { "domn", DOMN },
  { "impo", IMPO },
  { "role", ROLE },
  { "keyw", KEYW}
};




int16_t initContextResource(void)
{    
    context_eeprom_t ae;
    memset(&ae,0,sizeof(ae));

    context_index_object_index = obj_malloc(sizeof(context_eeprom_t),"RSRC");
   
    ae.domainName = string_insertWord("LondonEye",ae.domainName);
    ae.importance = string_insertWord("90",ae.importance);
    ae.roleDescription = string_insertWord("TempSensor",ae.roleDescription);
    ae.kewywordCount = MAX_CONTEXT_KEYWORDS;
    for(uint8_t i=0; i < MAX_CONTEXT_KEYWORDS; i++)
    {
        ae.keywords[i] = string_insertWord("group1",ae.keywords[i]);       
    }
    
    obj_setObjectAtIndex(context_index_object_index, (const void *)&ae);   
     
    return context_index_object_index;

}

error_msg deleteContextResource(obj_index_t objIdx)
 {

    obj_free(objIdx);    


    return SUCCESS;
}


error_msg contextInit(void *rdata, int16_t objId)
{    
    return SUCCESS;   
}


static error_msg writeToEthContextData(context_eeprom_t *p)
{    
    Get_ToEthernet((char *)"domn", TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->domainName),TEXT_STRING);    
    
    Get_ToEthernet((char *)"impo",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->importance),TEXT_STRING);
    
    Get_ToEthernet((char *)"role",TEXT_STRING);
    Get_ToEthernet(string_getWordAtIndex(p->roleDescription),TEXT_STRING);  
    
    Get_ToEthernet((char *)"keyw",TEXT_STRING);
    
    writeAuxData(&p->kewywordCount,ARRAY);    
    
    for(uint8_t i=0; i < p->kewywordCount; i++)
    {
        Get_ToEthernet(string_getWordAtIndex(p->keywords[i]),TEXT_STRING);  
    }
    
    return SUCCESS;
}

static uint8_t readFromEthContextData(obj_index_t objIdx, menu_items_t mid, context_eeprom_t *p)
{   
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;    
    
   
    memset(read_ptr,0,sizeof(read_ptr));
    switch(mid)
    {
        case DOMN:
            len += parseData(read_ptr,sizeof(read_ptr));
            p->domainName = string_insertWord(read_ptr,p->domainName);
            break;
        case IMPO:
            len += parseData(read_ptr,sizeof(read_ptr));             
            p->importance = string_insertWord(read_ptr, p->importance);           
            break;
        case ROLE:
            len += parseData(read_ptr,sizeof(read_ptr));
            p->roleDescription = string_insertWord(read_ptr,p->roleDescription);           
            break;       
        default:                
            break;  
    }
    return len;
}


error_msg contextGetter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{       
    error_msg ret = ERROR;
    context_eeprom_t p;   
    static uint8_t contextElements;
    
    contextElements = CONTEXT_EEP_ELEMENTS;
  
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    /**** Use Encode to write data to the buffer ****/  
    
    writeAuxData(&contextElements,MAP);  
    
    Get_ToEthernet((char *) "uuid",TEXT_STRING);
    Get_ToEthernet(getResourceUuid(idx),TEXT_STRING);
    
    ret = writeToEthContextData(&p);
    
    return ret;   
}

error_msg contextPutter(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{   
    error_msg ret =  ERROR;
    
    error_msg ret = SUCCESS;
    char read_ptr[MAX_WORD_SIZE];
    uint8_t len =0;
    uint8_t i;
    menu_items_t mid;
    const context_lst *choice = NULL;
    choice = cont_items_list;
    uint8_t payload_length;
    context_eeprom_t p;   
    static bool keyw = false; 
    uint8_t  count =0, prevKeywCount =0;
    uint8_t k = 0;
    
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    
    payload_length = PayloadLength;
    
    while(payload_length)
    {
        memset(read_ptr,0,sizeof(read_ptr));
        len =0;
        len += parseData(read_ptr,sizeof(read_ptr));
        
        for(i = 0, choice = NULL; i < 4; i++)
        {
            if (strncmp(read_ptr, cont_items_list[i].name, SIZE_OF_QUERY_NAME_ENUM) == 0)
            {
                choice = cont_items_list + i;
                break;
            }
        }           
        memset(read_ptr,0,sizeof(read_ptr));
        mid = choice ? choice->id : UNKNOWN;        
        switch(mid)
        {
            case UNKNOWN:     
                if(keyw)
                {
                    count = CfElements.array_elements;
                    printf("Count=%d\r\n",count);
                    prevKeywCount = p.kewywordCount;
                    if(count <= MAX_CONTEXT_KEYWORDS)
                    {
                        p.kewywordCount = count;
                        if(prevKeywCount > count)
                        {
                            while((prevKeywCount--)&& prevKeywCount >= count)
                            {
                                string_removeWord(p.keywords[prevKeywCount]);     
                                p.keywords[prevKeywCount] = 0;
                            }
                        }
                        while((count--) && count <=MAX_CONTEXT_KEYWORDS)
                        {
                            memset(read_ptr,0,sizeof(read_ptr));
                            len +=parseData(read_ptr,sizeof(read_ptr));
                            p.keywords[k] = string_insertWord(read_ptr,p.keywords[k]);                           
                            k++;                        
                        }
                        
                    }
                    keyw = false;
                    CfElements.array_elements = 0;
                }
                break;
            case KEYW:
                keyw = true;
                break;
            default:
                len += readFromEthContextData(objIdx, mid, &p);
                break;
        }      
        payload_length -= len;
    }
    obj_setObjectAtIndex(objIdx, (const void *)&p);      
   
    return ret;
    
}



error_msg contextPatcher(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg contextPoster(obj_index_t objIdx, int16_t idx, uint8_t PayloadLength)
{
    error_msg ret = ERROR;
     
    return ret;
}

error_msg contextDeleter(obj_index_t objIdx,int16_t idx, uint8_t PayloadLength)
{

    context_eeprom_t p;
    string_index_t stringIdxToBeRemoved;
    error_msg ret = ERROR;
    
    obj_getObjectAtIndex(objIdx, (const void *) &p);
    
    stringIdxToBeRemoved = p.domainName;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    stringIdxToBeRemoved = p.importance;
    ret = string_removeWord(stringIdxToBeRemoved);
    
    for (uint8_t i=0; i<MAX_CONTEXT_KEYWORDS; i++)
    {
        stringIdxToBeRemoved = p.keywords[i];
        ret = string_removeWord(stringIdxToBeRemoved);
    }
    
    stringIdxToBeRemoved = p.roleDescription;
    ret = string_removeWord(stringIdxToBeRemoved);
    


    return ret;
}





restfulMethodStructure_t contextMethods = {contextInit,contextGetter,contextPutter,contextPatcher,contextDeleter,contextPoster,initContextResource};


/**************************************************************Query Handlers**************************************************************************/

bool queryDomnHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    context_eeprom_t p;  
    
    if(domn_query==0)
    {
        obj_getObjectAtIndex(objIdx, (const void *)&p);  
      
        if((uint16_t)p.domainName == query_value) //jira: CAE_MCU8-5647
         {                
            domn_query = 1;
            ret = true;
        }
        else ret=false;
    }            
    else 
        ret=true;
    return ret;
}

bool queryImpoHandler(obj_index_t objIdx,uint32_t query_value)
{
    bool ret = false;
    context_eeprom_t p;     
     
     if(impo_query==0)
     {
        obj_getObjectAtIndex(objIdx, (const void *)&p);                 
        if((uint16_t)p.importance== query_value)
        {                 
            impo_query = 1;
            ret = true;
        }
        else ret = false;

    }
    else     
        ret = true;         
     
     return ret; 
 
}

bool queryRoleHandler(obj_index_t objIdx, uint32_t query_value)
{
   bool ret = false;
   context_eeprom_t p;    
     
     if(role_query==0)
     {        
        obj_getObjectAtIndex(objIdx, (const void *)&p); 
        if((uint16_t)p.roleDescription == query_value) //jira: CAE_MCU8-5647
        { 
            role_query = 1;
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
     else     
         ret = true;        
     
     return ret;   
}

bool queryKeywHandler(obj_index_t objIdx, uint32_t query_value)
{
   bool ret = false;
   context_eeprom_t p; 
   uint8_t j;
     
     if(keyw_query==0)
     {
        obj_getObjectAtIndex(objIdx, (const void *)&p);
        for(j=0; j< p.kewywordCount; j++)
        {
            if((uint16_t)p.keywords[j] == query_value) //jira: CAE_MCU8-5647
            { 
                keyw_query = 1;
                ret = true;
                break;
            }
        }
     }
     else     
         ret = true;        
     
     return ret;   
}

/**************************************************Context Getters****************************************************************************/
keyword_t *getContextKeywords(int16_t parentVIdx,bool forQuery, uint8_t string_idx)
{
    context_eeprom_t p;   
    static keyword_t kw;    
    vertex_index_t conVIdx;
    vertex_t parentVertex, contextVertex;
    uint16_t childCount =0;
    
    graph_getVertexAtIndex(&parentVertex,parentVIdx);
    childCount = parentVertex.childCount;
    
    for(uint8_t i =1; i <= childCount; i++)
    {
        graph_getVertexAtIndex(&contextVertex,parentVIdx+i);
        if(strncmp((string_getWordAtIndex(contextVertex.nameIdx)),CONTEXT,8) == 0)
        {
            conVIdx = parentVIdx + i;
            break;
        }
        else
        {
            conVIdx = 0;
        }
    }
    
    if(conVIdx == 0)
        return 0;  //jira: CAE_MCU8-5647
    

    obj_getObjectAtIndex(contextVertex.objIdx, (const void *)&p);
    if(string_idx <= p.kewywordCount)
    {
        memset(kw.string_word,0,sizeof(kw));
        if(forQuery==true)
        {
            strcpy(kw.string_word,"keyw=");            
        }
        strcat(kw.string_word,string_getWordAtIndex(p.keywords[string_idx]));
         return &kw;
    }
    else
        return 0;
   

}

char *getContextRoleDescription(obj_index_t objIdx, bool forQuery) 
{
    context_eeprom_t p; 
    char role_word[MAX_WORD_SIZE];

    memset(role_word,0,sizeof(role_word));
    obj_getObjectAtIndex(objIdx, (const void *)&p);
    if(forQuery)
    {
        strcpy(role_word,"role=");
        return strcat(role_word,string_getWordAtIndex(p.roleDescription));
    }
    return string_getWordAtIndex(p.roleDescription);
}
