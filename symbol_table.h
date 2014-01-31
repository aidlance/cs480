/**************************************************
*
*   NAME:
*       symbol_table.h
*
*   DESCRIPTION:
*       Provides the public API for the symbol
*       table
*
**************************************************/

#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

/*-------------------------------------------------
                 PROJECT INCLUDES
-------------------------------------------------*/
#include "tokens.h"
#include "types.h"

/*-------------------------------------------------
                LITERAL CONSTANTS
-------------------------------------------------*/

/*-------------------------------------
Error types
-------------------------------------*/
typedef sint8 sym_table_error_t8;
enum
{
    SYM_NO_ERROR            =  0,   /* no error             */
    SYM_PRINT_ERROR         = -1,   /* error while printing */
    SYM_UPDATE_ERROR        = -2,   /* error updating tables*/
    SYM_INIT_ADD_ERROR      = -3,   /* initialization error */
    SYM_INIT_ERROR          = -4,   /* initialization error */
    SYM_ALREADY_INITIALIZED = -5    /* initialization error */
};

/*-------------------------------------------------
                 FUNCION PROTOTYPES
-------------------------------------------------*/

sym_table_error_t8 init_symbol_table
(
    void
);

token_class_t8 is_in_table
(
    char       *str     /* string to check      */
);

token_class_t8 is_identifier
(
    char       *str     /* string to check      */
);

token_class_t8 is_keyword
(
    char       *str     /* string to check      */
);

struct token_type *get_token_data
(
    char       *str     /* string to check      */
);

sym_table_error_t8 update_symbol_table
(
    char               *str,    /* string to add                    */
    struct token_type  *data    /* token corresponding to string    */
);

sym_table_error_t8 print_table
(
    void
);

void unload_tables
(
    void
);

#endif // __SYMBOL_TABLE_H__

