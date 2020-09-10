/**
 Header file for CBOR Implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    cbor.h

  Summary:
    This is the header file for the cbor.c

  Description:
    This header file provides the API for CBOR Binary encoding and decoding.

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

/* 
 * Author: C16813
 */

#ifndef CBOR_H
#define CBOR_H

#include <stdint.h>
//#include <__size_t.h>
#include <stdbool.h>

#define data_type_t uint8_t

/*  major type 0(uint) Added Info bits */
#define CBOR_UINT8_FOLLOWS      24u      /* 0x18 */          //jira: CAE_MCU8-5647
#define CBOR_UINT16_FOLLOWS     25u      /* 0x19 */          //jira: CAE_MCU8-5647
#define CBOR_UINT32_FOLLOWS     26u      /* 0x1a */          //jira: CAE_MCU8-5647
#define CBOR_UINT64_FOLLOWS     27u      /* 0x1b */          //jira: CAE_MCU8-5647

/*28, 29, 30 are reserved for future Expansion*/

/*Indefinite Length item*/
#define CBOR_INDEFINITE_LENGTH_FOLLOWS  31  /*0x1F*/

/* Jump Table for Initial Byte (cf. table 5) */
#define CBOR_MAJOR_UINT             0x00u            /* type 0 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_NEGINT           0x20u            /* type 1 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_BYTE_STRING      0x40u            /* type 2 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_TEXT_STRING      0x60u            /* type 3 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_ARRAY            0x80u            /* type 4 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_MAP              0xA0u            /* type 5 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_TAG              0xC0u            /* type 6 */      //jira: CAE_MCU8-5647
#define CBOR_MAJOR_FLOATNMORE       0xE0u            /* type 7 (float and other types) */  //jira: CAE_MCU8-5647

/* Major type 7: FLOATNMORE */
#define CBOR_MAJOR_FLOATNMORE_FALSE      (CBOR_MAJOR_FLOATNMORE | 20)
#define CBOR_MAJOR_FLOATNMORE_TRUE       (CBOR_MAJOR_FLOATNMORE | 21)
#define CBOR_MAJOR_FLOATNMORE_NULL       (CBOR_MAJOR_FLOATNMORE | 22)
#define CBOR_MAJOR_FLOATNMORE_UNDEFINED  (CBOR_MAJOR_FLOATNMORE | 23)
/* CBOR_BYTE_FOLLOWS == 24 */
#define CBOR_MAJOR_FLOATNMORE_FLOAT16    (CBOR_MAJOR_FLOATNMORE | 25)
#define CBOR_MAJOR_FLOATNMORE_FLOAT32    (CBOR_MAJOR_FLOATNMORE | 26)
#define CBOR_MAJOR_FLOATNMORE_FLOAT64    (CBOR_MAJOR_FLOATNMORE | 27)
#define CBOR_MAJOR_FLOATNMORE_BREAK      (CBOR_MAJOR_FLOATNMORE | 31)


/* Major type 6: Semantic tagging */
#define CBOR_DATETIME_STRING_FOLLOWS        0
#define CBOR_DATETIME_EPOCH_FOLLOWS         1

typedef struct
{
   uint32_t n1;
   uint32_t n2;
} uint64_type; 

typedef enum
{
    MAJOR_UINT          =   0x00 ,           /* type 0 */
    MAJOR_NEGINT        =   0x20 ,           /* type 1 */
    MAJOR_BYTE_STRING   =   0x40 ,           /* type 2 */
    MAJOR_TEXT_STRING   =   0x60 ,           /* type 3 */
    MAJOR_ARRAY         =   0x80 ,           /* type 4 */
    MAJOR_MAP           =   0xA0 ,           /* type 5 */
    MAJOR_TAG           =   0xC0 ,           /* type 6 */
    MAJOR_FLOATNMORE    =   0xE0 ,           /* type 7 (float and other types) */

}CBOR_MAJOR_TYPES_t;

typedef union
{
    uint32_t uint_Element;
    int32_t  int_Element;
    char     *char_Element;
    bool     bool_Element;
}arr_or_map_Element;

typedef struct                                      
{
    arr_or_map_Element Element;
//    void *Array_or_Map_Element;
    CBOR_MAJOR_TYPES_t Array_or_Map_majortype;    
    CBOR_MAJOR_TYPES_t Array_or_Map_internalMemberType;
    uint8_t Array_or_Map_internalMemberLength;       /*Length for Array within an array or map within an Array*/
} Array_or_Map_Item_t;

/**************Encoding*************/

/*Encoding signed integer.
 * The function will encode a signed integer.
 * 
 * @param val
 *      Integer value to be encoded
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_Signed_Int(int32_t val);


/*Encoding signed integer.
 * The function will encode an unsigned integer.
 * 
 * @param val
 *      Unsigned integer value to be encoded
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_Unsigned_Int(uint32_t val);


/*Encoding byte string.
 * The function will encode a byte string.
 * 
 * @param byteString
 *      Pointer to the byte string to be encoded
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_Byte_String(char *byteString);


/*Encoding byte string.
 * The function will encode a text string.
 * 
 * @param byteString
 *      Pointer to the text string to be encoded
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_Text_String(const char *textString);


/*Encoding a definite length array.
 * The function will encode an array of definite length.
 * 
 * @param Arrayelement
 *      Pointer to the array to be encoded
 * 
 * @param length
 *      Length of the array
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_ArrayDefinite(Array_or_Map_Item_t *Arrayelement, uint32_t length);


/*Encoding a definite length map.
 * The function will encode a map of definite length.
 * 
 * @param Arrayelement
 *      Pointer to the map to be encoded
 * 
 * @param DataItemPairs
 *      The number of key-value pairs
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_MapsDefinite (Array_or_Map_Item_t *Map, uint32_t DataItemPairs);


/*Encoding a boolean value.
 * The function will encode a boolean value.
 * 
 * @param boolvalue
 *      Boolean value to be encoded
 * 
 * @param return
 *      Nothing
 * 
 */
void CBOR_Encode_Bool(bool boolvalue);



/**************Decoding*************/

/*Decoding data.
 * The function will decode the data read from the Ethernet buffer.
 * 
 * @param readPtr
 *      Pointer to the data being read for decoding
 * 
 * @param return
 *      Number of bytes read
 * 
 */
data_type_t     CBOR_Decode_Read_Data(char *readPtr);


/*Track the bytes to follow.
 * The function will keep track of length of the bytes to follow a packet being read.
 * 
 * @param val
 *      Value from which the number of bytes are to be extracted
 * 
 * @param return
 *      Number of bytes to follow
 * 
 */
int8_t          CBOR_Decode_Bytes_To_Follow(uint8_t val);


/*Decode integer value.
 * The function will decode a signed and unsigned integer values.
 * 
 * @param readPtr
 *      Pointer to the Ethernet buffer from which data is being read for decoding
 * 
 * @param return
 *      Number of bytes read
 * 
 */
uint8_t         cbor_decode_read_int(char *readPtr);


/*Decode a string.
 * The function will decode a text or a byte string.
 * 
 * @param readPtr
 *      Pointer to the Ethernet buffer from which data is being read for decoding
 * 
 * @param return
 *      Number of bytes read
 * 
 */
uint8_t         cbor_decode_read_string(char *readPtr);


/*Decode tags.
 * The function will decode tag values.
 * 
 * @param readPtr
 *      Pointer to the Ethernet buffer from which data is being read for decoding
 * 
 * @param return
 *      Number of bytes read
 * 
 */
uint8_t         cbor_decode_tags(char *readPtr);


/*Decode boolean value.
 * The function will decode a boolean value.
 * 
 * @param readPtr
 *      Pointer to the Ethernet buffer from which data is being read for decoding
 * 
 * @param return
 *      Number of bytes read
 * 
 */
uint8_t         cbor_decode_bool(char *readPtr);


/*Number of elements to be decoded.
 * The function will decode a boolean value.
 * 
 * @param readPtr
 *      Pointer to the Ethernet buffer from which data is being read for decoding
 * 
 * @param return
 *      Number of bytes read
 * 
 */
uint8_t cbor_elements();


void Test_Decode(int length);
void Test1_Decode(int length);


#endif

/** @} */


