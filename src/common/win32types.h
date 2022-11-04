/******************************************************************************

     Module: win32types.h
     Author: Veton Kepuska
    Created: May 27, 2006
    Updated: 

Description: Definition of data types supported by win32 Microsoft OS.

       $Log:$

*******************************************************************************

Notes: Proprietary and Confidential. All rights reserved.
       VoiceKey Inc., 
       Cocoa Beach, Florida 32931.

******************************************************************************/

#ifndef WIN32TYPES_H
#define WIN32TYPES_H

/******************************  Include Files  ******************************/

/*********************************  Defines  *********************************/

#ifdef UNICODE
	typedef wchar_t              TCHAR;
#else
	typedef char                 TCHAR;
#endif

typedef unsigned char        BOOLEAN;  // TRUE or FALSE
typedef char	              CHAR, SCHAR;    // signed character, 8 bits
typedef unsigned char        UCHAR;    // unsigned character,  8 bits

typedef signed char          INT8, SINT8;     // integer, 8 bits
typedef unsigned char        UINT8;    //unsigned character,  8 bits  

typedef signed short         INT16, SINT16;   // integer, 16 bits
typedef unsigned short       UINT16;   // usigned int, 16 bits

typedef signed int           INT32, SINT32;   // integer, 32 bits
typedef unsigned int         UINT32;   // usigned int, 32 bits

typedef signed long long     INT64, SINT64;   // usigned int, 64 bits
typedef unsigned long long   UINT64;   // usigned int, 64 bits

typedef float                FLOAT32;  // floating point, 32 bits
typedef double               FLOAT64;  // floating point, 64 bits



#endif //WIN32TYPES_H

/*******************************  End of File  *******************************/
