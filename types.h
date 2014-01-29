/**************************************************
*
*   NAME:
*       types.h
*
*   DESCRIPTION:
*       Contains type definitions for basic types
*       used by the project.
*
**************************************************/

#ifndef __AIDAN_TYPES_H__
#define __AIDAN_TYPES_H__

/*-------------------------------------
Integer maximums and minimums
-------------------------------------*/
#define UINT8_MAX  ( (uint8) ( ( 1 << 8  ) - 1 ) )
#define UINT16_MAX ( (uint16)( ( 1 << 16 ) - 1 ) )
#define UINT32_MAX ( (uint32)( ( 1 << 32 ) - 1 ) )

#define SINT8_MAX  (sint8) ( (uint8)(  1 << 7  ) - 1 )
#define SINT16_MAX (sint16)( (uint16)( 1 << 15 ) - 1 )
#define SINT32_MAX (sint32)( (uint32)( 1 << 31 ) - 1 )

#define SINT8_MIN  ( (sint8) ( 1 <<  7 ) )
#define SINT16_MIN ( (sint16)( 1 << 15 ) )
#define SINT32_MIN ( (sint32)( 1 << 31 ) )

/*-------------------------------------------------
Unsigned integer types
-------------------------------------------------*/
typedef unsigned           char uint8;  /* 8-bit unsigned integer  */
typedef unsigned short     int  uint16; /* 16-bit unsigned integer */
typedef unsigned           int  uint;   /* 24-bit unsigned integer */
typedef unsigned long      int  uint32; /* 32-bit unsigned integer */
typedef unsigned long long int  uint64; /* 64-bit unsigned integer */

/*-------------------------------------------------
Signed integer types
-------------------------------------------------*/
typedef signed           char sint8;    /* 8-bit signed integer    */
typedef signed short     int  sint16;   /* 16-bit signed integer   */
typedef signed           int  sint;     /* 24-bit signed integer   */
typedef signed long      int  sint32;   /* 32-bit signed integer   */
typedef signed long long int  sint64;   /* 64-bit signed integer  */

/*-------------------------------------------------
Booleans
-------------------------------------------------*/
typedef uint8 boolean;
enum
{
    FALSE,
    TRUE
};

#endif /* __AIDAN_TYPES_H__ */
