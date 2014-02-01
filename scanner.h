/**************************************************
*   NAME:
*       scanner.h
*
*   DESCRIPTION;
*       provides the public interface for the
*       lexical scanner
*
*************************************************/

#ifndef __SCANNER_H__
#define __SCANNER_H__

/*-------------------------------------------------
                 PROJECT INCLUDES
-------------------------------------------------*/

#include "types.h"

/*-------------------------------------------------
                 LITERAL CONSTANTS
-------------------------------------------------*/

typedef sint8 scanner_error_t8;
enum
{
    SCN_NO_ERROR           =  0,    /* no errors                */
    SCN_OPEN_ERROR         = -1,    /* errors opening file      */
    SCN_INVALID_OP         = -2,    /* invalid operation error  */
    SCN_BUFFER_OVERFLOW    = -3,    /* buffer overflow error    */
    SCN_INFINITE_STRING    = -4,    /* no closing '"' on string */
    SCN_INVALID_CONSTANT   = -5,    /* invalid constant error   */
    SCN_INVALID_FLOAT      = -6,    /* multiple '.' in a float  */
    SCN_INVALID_IDENTIFIER = -7,    /* invalid identifier       */
    SCN_INIT_ERROR         = -8     /* initialization error     */
};

/*-------------------------------------------------
                  FUNCTION PROTOTYPES
-------------------------------------------------*/

uint32 get_total_scanner_errors
(
    void
);

scanner_error_t8 init_scanner
(
    void
);

scanner_error_t8  tokenize
(
    char       *file_name   /* file to scan     */
);

scanner_error_t8 unload_scanner
(
    void
);

#endif /* __SCANNER_H__ */
