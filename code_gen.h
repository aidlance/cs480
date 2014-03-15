/**************************************************
*
*   HEADER NAME:
*       code_gen.h
*
*   DESCRIPTION:
*       Provides the public API for the code
*       generator
*
**************************************************/

#ifndef __CODE_GEN_H__
#define __CODE_GEN_H__

/*-------------------------------------------------
                 GENERAL INCLUDES
-------------------------------------------------*/

#include "types.h"
#include "tokens.h"

/*-------------------------------------------------
                LITERAL CONSTANTS
-------------------------------------------------*/

typedef sint8 cgen_error_t8;
enum
{
    CGEN_NO_ERROR    =  0,      /* error code for no errors         */
    CGEN_NULL_REF    = -1,      /* error code for null reference    */
    CGEN_INIT_ERROR  = -2,      /* initialization error             */
    CGEN_OPEN_ERROR  = -3,      /* error code for file open errors  */
    CGEN_CLOSE_ERROR = -4,      /* error code for file close errors */
    CGEN_SEM_ERROR   = -5       /* error code for semantic errors   */
};

/*-------------------------------------------------
                 FUNCTION PROTOTYPES
-------------------------------------------------*/

cgen_error_t8 gen_code
(
    char *input_filename        /* input file's filename    */
);

#endif /* __CODE_GEN_H__ */
