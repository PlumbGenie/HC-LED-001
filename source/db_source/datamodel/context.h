/**
  Descriptive File Name
	
  Company:
    Microchip Technology Inc.

  File Name:
    context.h

  Summary:
    This is the header file for the context.c

  Description:
    This header file provides the API for the Context Resource.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
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

//  Created by Joseph Julicher - C10948 on 9/23/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.


#ifndef CONTEXT_H
#define CONTEXT_H

#include "coapMethods.h"
#include "data_model.h"
#include "string_heap.h"



typedef struct
{   
    string_index_t domainName;                             //domn
    string_index_t importance;                                   //impo
    string_index_t roleDescription;                        //role    
    string_index_t keywords[MAX_CONTEXT_KEYWORDS];                 //keyw
    uint8_t kewywordCount;
}context_eeprom_t;

 
 typedef struct 
{
  const char name[SIZE_OF_QUERY_NAME_ENUM];
  menu_items_t id;
}context_lst;

typedef struct
{
   char string_word[MAX_WORD_SIZE+5];
}keyword_t;

extern restfulMethodStructure_t contextMethods;

int16_t initContextResource(void);
error_msg deleteContextResource(obj_index_t objIdx);

extern bool queryDomnHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryImpoHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryRoleHandler(obj_index_t objIdx,uint32_t query_value);
extern bool queryKeywHandler(obj_index_t objIdx,uint32_t query_value);

/*Context Keywords Getter.
 * The function will return the list of all Context Keywords of the resource.

 * @param forQuery
 *      true  - Name of the query is appended.
 *      false - Name of the query is not appended.
 * 
 * @param string_idx
 *      Keyword Index
 * 
 * @param return
 *      List of the keywords with name of the query appended (or)
 *      List of the keywords
 * 
 */
keyword_t *getContextKeywords(int16_t parentVIdx,bool forQuery, uint8_t string_idx);

/*Context Role Description Getter.
 * The function will return the Context Role Description of the resource.
 * 
 * 
 * @param forQuery
 *      true  - Name of the query is appended.
 *      false - Name of the query is not appended.
 * 
 * @param return
 *      Role Description with name of the query appended (or)
 *      Role Description
 * 
 */
char *getContextRoleDescription(obj_index_t objIdx, bool forQuery);

#endif /* CONTEXT_H */
