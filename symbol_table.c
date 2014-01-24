#include <string.h>

#include "symbol_table.h"
#include "types.h"

const char *__identifiers[ 8 ] = 
{
    "while",
    "if",
    "let",
    "stdout",
    "true",
    "false",
    "and",
    "or",
    "not",
    "sin",
    "cos",
    "tan",
    "bool",
    "int",
    "real",
    "string"
};

const char *__symbols[ 4 ] =
{
    "+",
    "-",
    "/",
    "*",
    "%",
    "^",
    "=",
    "!=",
    ">=",
    "<=",
    "<",
    ">",
    ":=",
    "[",
    "]"
};

#define size( buf ) ( sizeof( buf ) / sizeof( buf[ 0 ] ) )

static bool __is_initialized;
static char *__symbol_table[];

sym_table_error_t8 init_symbol_table
(
    void
)
{
    int i;
    int len;
    int idx;

    if( __is_initialized )
    {
        return( SYM_ALREADY_INITIALIZED );
    }
 
    for( i = 0; i < __NUM_IDENTIFIERS; ++i )
    {
        idx = __hash( __identifiers[ i ] );
        __symbol_table[ idx ] = __identifiers + i; 
    }

    for( i = 0; i < __NUM_SYMBOLS; ++i )
    {
    }

    return( SYM_NO_ERROR );

}   /* init_symbol_table() */
