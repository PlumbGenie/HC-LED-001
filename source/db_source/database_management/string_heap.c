/**
 EEPROM string heap implementation

  Company:
    Microchip Technology Inc.

  File Name:
    string_heap.c

  Summary:
    EEPROM string heap implementation.

  Description:
    This file provides the EEPROM string heap implementation.

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

//
//  string_heap.c
//  
//
//  Created by Joseph Julicher - C10948 on 8/27/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//
#include <xc.h>
#include <string.h>
#include "data_model.h"
#include "string_heap.h"
#include "strnlen.h"
#include "object_heap.h"
#include "eeprom.h"
#include "time.h"

const char *kidx_id = "KIDX";
const char *khep_id = "KHEP";

// keywords are stored as a comma list.
// 1 NULL at the end.

// STORAGE MAPPING
typedef storagePtr strPtr; // adjust for the STORAGE size.

//char *iterator;   //jira: CAE_MCU8-5647

obj_index_t string_index_object_index;
obj_heap_ptr_t string_index_object;
obj_heap_ptr_t string_stringHeap_object;
obj_index_t string_stringHeap_object_index;

#define STRHEAPSIZE 6400
#define AVG_WORD_SIZE 8

#define MAXWORDS (STRHEAPSIZE/AVG_WORD_SIZE)

#define STRINDEXADDRESS (string_index_object.ptr)
#define STRHEAPADDRESS (string_stringHeap_object.ptr)
#define STRHEAPEND (STRHEAPADDRESS + STRHEAPSIZE)

#define STRINDEXSIZE (MAXWORDS * sizeof(storagePtr))

typedef struct {
    uint8_t stringLength;
    uint8_t stringHash;
} stringID_t;

typedef struct {
    stringID_t stringID;
    uint8_t referenceCount;
    strPtr stringLocation;
} indexType_t;

void string_setPtrAtIndex(int16_t idx, strPtr p);

error_msg string_decrementReferenceCount(int16_t idx);
error_msg string_incrementReferenceCount(int16_t idx);

void string_init(void)
{
    string_index_object_index = obj_malloc(STRINDEXSIZE,kidx_id);
    obj_getObjectPtrAtIndex(string_index_object_index, &string_index_object);
    string_stringHeap_object_index = obj_malloc(STRHEAPSIZE,khep_id);
    obj_getObjectPtrAtIndex(string_stringHeap_object_index,&string_stringHeap_object);  
    }
    
#define makeAddressFromIndex(a) (string_index_object.ptr + (a * sizeof(indexType_t)))

strPtr string_getPtrFromIndex(int16_t idx)
{
    indexType_t value;
    strPtr P = makeAddressFromIndex((uint16_t)idx); //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)P, &value, sizeof(value)); //jira: CAE_MCU8-5647
    return value.stringLocation;
}

void string_getIDAtIndex(int16_t idx,stringID_t *value)
{
    strPtr P = makeAddressFromIndex((uint16_t)idx); //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)P, value, sizeof(*value)); //jira: CAE_MCU8-5647
}

void string_getValueAtIndex(int16_t idx, indexType_t *value)
{
    strPtr P = makeAddressFromIndex((uint16_t)idx); //jira: CAE_MCU8-5647
    storageReadBlock((uint32_t)P, value, sizeof(*value)); //jira: CAE_MCU8-5647
}

void string_setValueAtIndex(int16_t idx, indexType_t *value)
{
    strPtr P = makeAddressFromIndex((uint16_t)idx); //jira: CAE_MCU8-5647
    storageWriteBlock((uint32_t)P, value, sizeof(*value)); //jira: CAE_MCU8-5647
}

void string_setPtrAtIndex(int16_t idx, strPtr p)
{
    indexType_t v;
    strPtr Ptr = makeAddressFromIndex((uint16_t)idx); //jira: CAE_MCU8-5647
    v.stringLocation =p;
    storageWriteBlock((uint32_t)Ptr, &v, sizeof(v));  //jira: CAE_MCU8-5647
}

// maximum number of index's in the system
int16_t string_getIndexCount(void)
{
    return MAXWORDS;
}

// copy a string from the string store into a pointer provided
int16_t string_copyString(char *str, int16_t idx, int16_t cnt)
{
    strPtr p = string_getPtrFromIndex(idx);
    char size=0;
    if(p > -1)
    {
        storageReadBlock((uint32_t)p, str, (uint16_t)cnt); // this may actually read more characters than necessary. //jira: CAE_MCU8-5647
        size = strnlen(str,(uint16_t)cnt);  //jira: CAE_MCU8-5647
    }
    return size;
}

// compare a string with a string at an index
int16_t string_strncmp(const char *str, int16_t idx, int16_t cnt)
{
    strPtr p = string_getPtrFromIndex(idx);
    char c;
    if(p>-1)
    {
        for ( ; cnt--; ++str, ++p)
        {
            c = storageRead((uint32_t)p); //jira: CAE_MCU8-5647
            if(*str != c)
            {
                return *str - c;
            }
            else if(*str == '\0')
            {
                return 0;
            }
        }
    }
    return 0;
}

strPtr string_malloc(size_t n)
{
    strPtr ret = 0;
    int16_t x,s;
    strPtr p;
    
    // step 1 : find the last byte in the pool
    // step 2 : determine if there is space left in the pool for the new word.
    // step 3 : if there is space: return a pointer to the beginning of the bytes
    // step 3 : if there is no space, return 0
    
    // note : the pool is always defragmented because string_free will pay the price.
    p = string_stringHeap_object.ptr;

    for(x = STRHEAPSIZE-1;x!=0;x--)
    {
        if (STORAGE_POST_ERASE_VAL == 0xFF)
        {
            if(storageRead((uint32_t)(p+x))==0)  //jira: CAE_MCU8-5647
            {
                // string\0string\0string\0\0xF\0xF\0xF
                //      X is pointing here ^
                // We need to add 1 to skip the trailing NULL.
                // and 1 more to point at the first available byte.
                x += 1;
                break;
            }
        }
        else if (STORAGE_POST_ERASE_VAL ==0)
        {
            if(storageRead((uint32_t)(p+x))) //jira: CAE_MCU8-5647
            {
                // string\0string\0string\0\0\0
                //      X is pointing here ^
                // We need to add 1 to skip the trailing NULL.
                // and 1 more to point at the first available byte.
                x += 2;
                break;
            }
            
        }
    }
    s = STRHEAPSIZE-x; // bytes remaining in the heap
    if((uint16_t)s >= n+1) // make sure we have enough bytes remaining. //jira: CAE_MCU8-5647
    {
        ret = p + x;
        
    }
    else
        ret = -1;

    return ret;
}

void string_compactList(void)
{
    strPtr dest_ptr, src_ptr;
    int16_t indexToMove=0;
    int16_t zeroCount = 0;
    
    // Scan the list for more than 1 NULL in a row.
    dest_ptr = string_stringHeap_object.ptr;
    while(dest_ptr < string_stringHeap_object.ptr + string_stringHeap_object.size)
    {
        for(int16_t x=MAX_WORD_SIZE ; x && storageRead((uint32_t)dest_ptr) ; x --, dest_ptr ++); //jira: CAE_MCU8-5647
        dest_ptr ++;
        
        if(storageRead((uint32_t)dest_ptr) == 0) // found a string terminator //jira: CAE_MCU8-5647
        {
            zeroCount ++;
        }
        else
        {
            if(zeroCount>1)
            {
                // found the end of the zero block... time to move a word
                src_ptr = dest_ptr;
                dest_ptr -= (zeroCount);// move the dest_ptr bacstrards
                // find the index that matches the src_ptr;
                for(int16_t x=0;x<MAXWORDS;x++)
                {
                    if( string_getPtrFromIndex(x) == src_ptr)
                    {
                        indexToMove = x;
                        break;
                    }
                }
                if(indexToMove)
                {
                    char c;
                    // update the index
                    string_setPtrAtIndex(indexToMove, dest_ptr);
                    // move one string while clearing the original one.
                    while((c=storageRead((uint32_t)src_ptr)))  //jira: CAE_MCU8-5647
                    {
                        storageWrite((uint32_t)dest_ptr++, c); //jira: CAE_MCU8-5647
                        storageWrite((uint32_t)src_ptr++,0);   //jira: CAE_MCU8-5647
                    }
                    zeroCount = 0;
                }
            }
        }
    }
}

error_msg string_removeWord(int16_t idx)
{
    indexType_t value;
    error_msg ret=ERROR;
    uint32_t spaceWriteReady;
    
    //get value at index
    string_getValueAtIndex(idx, &value);
    if(value.referenceCount != 0)
    {
        ret = string_decrementReferenceCount(idx);
    }
    string_getValueAtIndex(idx, &value);
    if(value.referenceCount == 0)    
    {
        strPtr p;
        p = string_getPtrFromIndex(idx);
        // found the word...
        // clear the word...
         spaceWriteReady = SPACE_WRITE_READY;
        storageWriteBlock((uint32_t)p,&spaceWriteReady,strlen(string_getWordAtIndex(idx))); //jira: CAE_MCU8-5647
        // step 2 : clear the index
        string_setPtrAtIndex(idx,SPACE_WRITE_READY);
        // step 3 : defragment the pool
//        string_compactList();
        ret = SUCCESS;
    }
    return ret;
}

error_msg string_replaceWord(int16_t idx, const char *word)
{
    indexType_t value;
    error_msg ret = ERROR;

    string_getValueAtIndex(idx, &value);
    if(value.referenceCount == 1)
    {
        // step 1 : find the index that points to this word
        strPtr p = string_getPtrFromIndex(idx);
        // found the word...
        size_t cnt = strnlen(word,MAX_WORD_SIZE);
        cnt ++; // strnlen returns the size of the word without including the terminator so we will add one to include it in our memory copy.
        if(cnt < MAX_WORD_SIZE)
        {
            storageWriteBlock((uint32_t)p, word, cnt);  //jira: CAE_MCU8-5647
        } 
        ret = SUCCESS;
    }
    else if (value.referenceCount == 0) ret = SUCCESS;
    return ret;
}

error_msg string_incrementReferenceCount(int16_t idx)
{
    indexType_t value;
    //get value at index
    string_getValueAtIndex(idx, &value);
    //increment
    value.referenceCount++;
    //set value at index
    string_setValueAtIndex(idx,&value);
    return SUCCESS;
}

error_msg string_decrementReferenceCount(int16_t idx)
{
    indexType_t value;
    //get value at index
    string_getValueAtIndex(idx, &value);
    //decrement
    if (value.referenceCount != 0) value.referenceCount--;
    //set value at index
    string_setValueAtIndex(idx,&value);
    return SUCCESS;
    
    
}

int16_t string_countWords(void)
{
    int16_t ret = 0;
    for(int16_t x=0;x<string_getIndexCount();x++)
    {
        if((uint32_t)string_getPtrFromIndex(x)!=SPACE_WRITE_READY) //jira: CAE_MCU8-5647
            ret ++;
    }
    return ret;
}

uint8_t string_mkhash(const char *w)
{
    // using a pearson hash function
    uint8_t h;
    uint8_t index;
    static const unsigned char T[256] = {
        // 256 values 0-255 in any (random) order suffices
          98,  6, 85,150, 36, 23,112,164,135,207,169,  5, 26, 64,165,219, //  1
          61, 20, 68, 89,130, 63, 52,102, 24,229,132,245, 80,216,195,115, //  2
          90,168,156,203,177,120,  2,190,188,  7,100,185,174,243,162, 10, //  3
         237, 18,253,225,  8,208,172,244,255,126,101, 79,145,235,228,121, //  4
         123,251, 67,250,161,  0,107, 97,241,111,181, 82,249, 33, 69, 55, //  5
          59,153, 29,  9,213,167, 84, 93, 30, 46, 94, 75,151,114, 73,222, //  6
         197, 96,210, 45, 16,227,248,202, 51,152,252,125, 81,206,215,186, //  7
          39,158,178,187,131,136,  1, 49, 50, 17,141, 91, 47,129, 60, 99, //  8
         154, 35, 86,171,105, 34, 38,200,147, 58, 77,118,173,246, 76,254, //  9
         133,232,196,144,198,124, 53,  4,108, 74,223,234,134,230,157,139, // 10
         189,205,199,128,176, 19,211,236,127,192,231, 70,233, 88,146, 44, // 11
         183,201, 22, 83, 13,214,116,109,159, 32, 95,226,140,220, 57, 12, // 12
         221, 31,209,182,143, 92,149,184,148, 62,113, 65, 37, 27,106,166, // 13
           3, 14,204, 72, 21, 41, 56, 66, 28,193, 40,217, 25, 54,179,117, // 14
         238, 87,240,155,180,170,242,212,191,163, 78,218,137,194,175,110, // 15
          43,119,224, 71,122,142, 42,160,104, 48,247,103, 15, 11,138,239  // 16
         };
    
    h = 0;
    while(*w)
    {
        index = (uint8_t)(h ^ *(w++)); //jira: CAE_MCU8-5647
        h = T[index];
    }
    return h;
}

int16_t string_insertWord(const char *word, int16_t str_idx)
{
    int16_t ret = -1;
    int16_t x;
    indexType_t idx;
    
    ret = isTheStringPresentAtIndex(word);
//    printf("KEYWORD:%s\r\n",word);
    if(ret == -1)
    {
        // find a free index
        for(x=0;x<string_getIndexCount();x++)
        {
            if((uint32_t)string_getPtrFromIndex(x)==SPACE_WRITE_READY) //jira: CAE_MCU8-5647
            {
                strPtr p;
                size_t cnt = strnlen(word,MAX_WORD_SIZE);
                cnt ++; // strnlen returns the size of the word without including the terminator so we will add one to include it in our memory copy.
                if(cnt <= MAX_WORD_SIZE)
                {
                    p = string_malloc(cnt);
                    if(p > -1)
                    {
                        idx.stringLocation = p;
                        idx.stringID.stringHash = string_mkhash(word);
//                        printf("string hash at insert: %d\r\n",idx.stringID.stringHash); 
                        idx.stringID.stringLength = cnt - 1;
//                        printf("string LENGTH at insert: %d\r\n",idx.stringID.stringLength);
                        //first time the reference should get a 1
                        idx.referenceCount = 1;
                        string_setValueAtIndex(x,&idx);
                        storageWriteBlock((uint32_t)p, word, cnt); //jira: CAE_MCU8-5647
                        //Decrement reference count only for strings which are replaced with new ones.     
                        if(str_idx > 0)
                        {
                            string_removeWord(str_idx);
                        }
                        
                        ret = x;
                        break;
                    }
                    //- Have to test this case
                }                
                ret = str_idx;
                break;
            }
        }
    }
    else
    {
        string_incrementReferenceCount(ret);
    }
    return ret;
}

int16_t isTheStringPresentAtIndex(const char *w)
{
    int16_t x=0;
    for(x= 0; x< string_getIndexCount(); x++)
    {
       if(strncmp(string_getWordAtIndex(x),w,MAX_WORD_SIZE)==0) 
           return x;
    }
    return -1;
}

int16_t string_findIndexOfWord(const char *w)
{
    int16_t x;
    int16_t ret = -1;
    uint8_t theHash;
    uint8_t theLength;
    indexType_t thisID;
//    
//    clock_t c1,c2;
//    c1 = clock();
    
    theHash = string_mkhash(w);

    theLength = strnlen(w, MAX_WORD_SIZE);
    
//    printf("string_local Length:%d Hash:%d\r\n",theLength, theHash);
    for(x=0;x<string_getIndexCount();x++)
    {
        string_getIDAtIndex(x, &thisID.stringID);
//        printf("string_eep Length:%d\ Hash:%d\r\n",thisID.stringID.stringLength, thisID.stringID.stringHash);
        if(thisID.stringID.stringLength)
        {
            if(thisID.stringID.stringHash == theHash && thisID.stringID.stringLength == theLength)
            {
                if((thisID.stringLocation=string_getPtrFromIndex(x))>-1)
                {
                    if(string_strncmp(w,x,MAX_WORD_SIZE)==0)
                    {
//                        printf("found %s at index =%x\r\n",w,x);
                        ret = x;
                        break;
                    }
                }   

            }
        }
    }
//    c2 = clock();
//    printf("\tfound %s in %f seconds : ",w,(double)(c2-c1)/CLOCKS_PER_SEC);
    return ret;
}

char *string_getWordAtIndex(int16_t idx)
{
    static char return_string[32];
    memset(return_string,0,sizeof(return_string));
    if(idx >-1)
    {
        string_copyString(return_string,idx,sizeof(return_string));
    }
    return return_string;
}
