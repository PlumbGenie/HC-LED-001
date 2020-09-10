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
//  keywords.h
//  mchp_graph
//
//  Created by Joseph Julicher - C10948 on 8/27/15.
//  Copyright (c) 2015 Joseph Julicher - C10948. All rights reserved.
//

#ifndef STRING_HEAP_H
#define STRING_HEAP_H

#include <stdio.h>
#include <string.h>

typedef int16_t string_index_t;



void string_init(void);
int16_t  string_countWords(void);
void string_compactList(void);
string_index_t string_insertWord(const char *word, int16_t str_idx);
error_msg string_removeWord(int16_t);
error_msg string_replaceWord(int16_t idx, const char *word); // only if this is the last string else failure
string_index_t  string_findIndexOfWord(const char * w);
char *string_getWordAtIndex(string_index_t idx);
int16_t isTheStringPresentAtIndex(const char *w);


string_index_t query_insertWord(const char *word);
int16_t query_findIndexOfWord(const char *w);
const char *query_getWordAtIndex(int16_t idx);
//strPtr query_malloc(size_t n);

#endif /* defined STRING_HEAP_H */
