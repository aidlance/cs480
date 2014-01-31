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

#include "types.h"

typedef sint8 scanner_error_t8;
enum
{
    SCN_NO_ERROR           =  0,
    SCN_OPEN_ERROR         = -1,
    SCN_INVALID_OP         = -2,
    SCN_BUFFER_OVERFLOW    = -3,
    SCN_INFINITE_STRING    = -4,
    SCN_INVALID_CONSTANT   = -5,
    SCN_INVALID_FLOAT      = -6,
    SCN_INVALID_IDENTIFIER = -7,
    SCN_INIT_ERROR         = -8
};

scanner_error_t8 init_scanner();
scanner_error_t8 unload_scanner();
scanner_error_t8  tokenize
(
    char *file_name
);

#endif /* __SCANNER_H__ */
