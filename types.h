/**************************************************
*
* NAME: types.h
*
* DESCRIPTION: 
* Contains type definitions for basic types
* used by the project.
*
**************************************************/

/*-------------------------------------------------
Unsigned integer types
-------------------------------------------------*/
typedef uint8  unsigned           char;   /* 8-bit unsigned integer  */
typedef uint16 unsigned short     int;    /* 16-bit unsigned integer */
typedef uint   unsigned           int;    /* 24-bit unsigned integer */
typedef uint32 unsigned long      int;    /* 32-bit unsigned integer */
typedef uint64 unsigned long long int;    /* 64-bit unsigned integer */

/*-------------------------------------------------
Signed integer types
-------------------------------------------------*/
typedef sint8  signed           char;     /* 8-bit signed integer    */ 
typedef sint16 signed short     int;      /* 16-bit signed integer   */
typedef sint   signed           int;      /* 24-bit signed integer   */
typedef sint32 signed long      int;      /* 32-bit signed integer   */
typedef sint64 signed long long int;      /* 64-bit signed innteger  */

/*-------------------------------------------------
Booleans
-------------------------------------------------*/
typedef boolean uint8;
enum
{
    FALSE,
    TRUE
};
