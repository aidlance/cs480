/**************************************************
*
*   MUDILE:
*       symbol_table.c
*
*   DESCRIPTION:
*       Contains the implementation for the
*       symbol table
*
**************************************************/

/*-------------------------------------------------
                PROJECT INCLUDES
-------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"
#include "symbol_table.h"
#include "tokens.h"
#include "types.h"

/*-------------------------------------------------
                LITERAL CONSTANTS
-------------------------------------------------*/

#define __MAX_KEYWORD_LEN   8   /* maximum number of characters */
                                /*  in a keyword string         */
#define __MAX_SYMBOL_LEN    4   /* maximum number of characters */
                                /*  in a symbol string          */

/*-------------------------------------------------
                      TYPES
-------------------------------------------------*/

struct __reserved_symbol
{
    char                word[ __MAX_KEYWORD_LEN ];  /* keyword              */
    struct token_type   tok;                        /* corresponding token  */
};

/*-------------------------------------------------
                VARIABLE CONSTANTS
-------------------------------------------------*/

/*-------------------------------------
Table containing all reserved words
and the corresponding token values.

TODO: Fill the out_str fieds with
      Gforth output
-------------------------------------*/
const struct __reserved_symbol __keywords[] =
{
    { "while",  { TOK_RESERVED_WORD, { TOK_WHILE,      "while",  "while"  } } },
    { "let",    { TOK_RESERVED_WORD, { TOK_LET,        "let",    "let"    } } },
    { "stdout", { TOK_RESERVED_WORD, { TOK_STDOUT,     "stdout", "stdout" } } },
    { "true",   { TOK_RESERVED_WORD, { TOK_TRUE,       "true",   "true"   } } },
    { "if",     { TOK_RESERVED_WORD, { TOK_IF,         "if",     "if"     } } },
    { "false",  { TOK_RESERVED_WORD, { TOK_FALSE,      "false",  "false"  } } },
    { "int",    { TOK_RESERVED_WORD, { TOK_INT,        "int",    ""       } } },
    { "real",   { TOK_RESERVED_WORD, { TOK_REAL,       "real",   ""       } } },
    { "bool",   { TOK_RESERVED_WORD, { TOK_BOOL,       "bool",   ""       } } },
    { "string", { TOK_RESERVED_WORD, { TOK_STRING,     "string", ""       } } },
    { "and",    { TOK_BINARY_OPP,    { TOK_AND_OPP,    "and",    "and"    } } },
    { "or",     { TOK_BINARY_OPP,    { TOK_OR_OPP,     "or",     "or"     } } },
    { "+",      { TOK_BINARY_OPP,    { TOK_ADD_OPP,    "+",      "+"      } } },
    { "-",      { TOK_BINARY_OPP,    { TOK_SUB_OPP,    "-",      "-"      } } },
    { "*",      { TOK_BINARY_OPP,    { TOK_MUL_OPP,    "*",      "*"      } } },
    { "/",      { TOK_BINARY_OPP,    { TOK_DIV_OPP,    "/",      "/"      } } },
    { "%",      { TOK_BINARY_OPP,    { TOK_MOD_OPP,    "%",      "%"      } } },
    { "^",      { TOK_BINARY_OPP,    { TOK_EXP_OPP,    "^",      "^"      } } },
    { "=",      { TOK_BINARY_OPP,    { TOK_EQ_OPP,     "=",      "="      } } },
    { "<",      { TOK_BINARY_OPP,    { TOK_LT_OPP,     "<",      "<"      } } },
    { ">",      { TOK_BINARY_OPP,    { TOK_GT_OPP,     ">",      ">"      } } },
    { "<=",     { TOK_BINARY_OPP,    { TOK_LE_OPP,     "<=",     "<="     } } },
    { ">=",     { TOK_BINARY_OPP,    { TOK_GE_OPP,     ">=",     ">="     } } },
    { "!=",     { TOK_BINARY_OPP,    { TOK_NE_OPP,     "!=",     "!="     } } },
    { ":=",     { TOK_BINARY_OPP,    { TOK_ASSN_OPP,   ":=",     ":="     } } },
    { "[",      { TOK_LIST_TYPE,     { TOK_LIST_BEGIN, "[",      "["      } } },
    { "]",      { TOK_LIST_TYPE,     { TOK_LIST_END,   "]",      "]"      } } },
    { "sin",    { TOK_UNARY_OPP,     { TOK_SIN_OPP,    "sin",    "sin"    } } },
    { "cos",    { TOK_UNARY_OPP,     { TOK_COS_OPP,    "cos",    "cos"    } } },
    { "tan",    { TOK_UNARY_OPP,     { TOK_TAN_OPP,    "tan",    "tan"    } } },
    { "not",    { TOK_UNARY_OPP,     { TOK_NOT_OPP,    "not",    "not"    } } },
    { "-",      { TOK_UNARY_OPP,     { TOK_NEG_OPP,    "-",      "neg"    } } },
    { "+",      { TOK_UNARY_OPP,     { TOK_POS_OPP,    "+",      ""       } } }
};

/*-------------------------------------------------
                 GLOBAL VARIABLES
-------------------------------------------------*/

/*-------------------------------------
The actual symbol table consists of
two tables: one reserved for just the
keywords, and the other reserved for
the identifiers and variable constants.
-------------------------------------*/
static struct map *__keyword_table = NULL;
static struct map *__id_table      = NULL;

/*-------------------------------------------------
                        MACROS
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       size - "Size"
*
*   DESCRIPTION:
*       Computes the size of a buffer
*
**************************************************/
#define size( buf ) ( sizeof( buf ) / sizeof( buf[ 0 ] ) )

/*-------------------------------------------------
              FUNCTION PROTOTYPES
-------------------------------------------------*/

static void __print_entry
(
    void   *data    /* data to print    */
);

/*-------------------------------------------------
                    PROCEDURES
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       __print_entry - "Print Entry"
*
*   DESCRIPTION:
*       Prints an entry from the symbol table.
*       The callback function used by show_map()
*
**************************************************/
static void __print_entry
(
    void   *data    /* data to print    */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    struct token_type  *tok;    /* token to print   */

    tok = (struct token_type *)data;
    switch( tok->token_class )
    {
        case TOK_BINARY_OPP:
            printf( "<BIN_OP, %s>\n", tok->opp.in_str );
            break;

        case TOK_UNARY_OPP:
            printf( "<UN_OP, %s>\n", tok->opp.in_str );
            break;

        case TOK_LITERAL:
            printf( "<LITERAL, %s>\n", tok->literal.str );
            break;

        case TOK_IDENT:
            printf( "<ID, %s>\n", tok->id.in_str );
            break;

        case TOK_RESERVED_WORD:
            printf( "<%s>\n", tok->res_word.in_str );
            break;

        case TOK_LIST_TYPE:
            printf( "<%s>\n", tok->res_word.in_str );
            break;

        default:
            break;
    }

}   /* __print_entry() */


/**************************************************
*
*   FUNCTION:
*       init_symbol_table - "Initialize Symbol
*                            Table"
*
*   DESCRIPTION:
*       This initializes the symbol table.
*
*   ERRORS:
*       * Returns SYM_NO_ERROR if there were
*         no errors
*       * Returns SYM_ALREADY_INITIALIZED if
*         the symbol table has already been
*         initialized
*       * Returns SYM_INIT_ERROR if there was
*         an error encountered while either
*         creating or initializing the symbol
*         table
*       * Returns SYM_INIT_ADD_ERROR if there
*         was an error when adding the different
*         symbols to the table.
*
**************************************************/
sym_table_error_t8 init_symbol_table
(
    void
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32  i;          /* a for-loop iterator      */
    sint    table_size; /* size of the table        */

    /*---------------------------------
    Check if the table is already
    initialized
    ---------------------------------*/
    if( NULL != __keyword_table )
    {
        return( SYM_ALREADY_INITIALIZED );
    }

    /*---------------------------------
    Create and initialize the keyword
    table
    ---------------------------------*/
    table_size = (sint)ceil( 0.5 * (double)( size( __keywords ) ) );
    __keyword_table = create_map();
    if( NULL == __keyword_table )
    {
        return( SYM_INIT_ERROR );
    }

    if( ERR_NO_ERROR != init_static_map( __keyword_table,
                                         table_size      ) )
    {
        return( SYM_INIT_ERROR );
    }

    /*---------------------------------
    Create and initialize the
    identifier table
    ---------------------------------*/
    __id_table = create_map();
    if( NULL == __id_table )
    {
        return( SYM_INIT_ERROR );
    }

    if( ERR_NO_ERROR != init_static_map( __id_table, 0 ) )
    {
        return( SYM_INIT_ERROR );
    }

    /*---------------------------------
    Add all of the keywords in the
    giant table somewhere above
    ---------------------------------*/
    for( i = 0; i < size( __keywords ); ++i )
    {
        if( 0 == add_map (  __keyword_table,
                            __keywords[ i ].word,
                           &__keywords[ i ].tok,
                            sizeof( struct token_type ) ) )
        {
            return( SYM_INIT_ADD_ERROR );
        }
    }

    return( SYM_NO_ERROR );

}   /* init_symbol_table() */


/**************************************************
*
*   FUNCTION:
*       is_in_table - "Is In Table"
*
*   DESCRIPTION:
*       Checks whether a string is in the
*       symbol table
*
*   RETURNS:
*       Returns TRUE if the string is in the
*       symbol table and FALSE if it isn't.
*
**************************************************/
boolean is_in_table
(
    char       *str     /* string to check      */
)
{
    return( is_keyword( str ) || is_identifier( str ) );

}   /* is_in_table() */


/**************************************************
*
*   FUNCTION:
*       is_identifier - "Is Identifier"
*
*   DESCRIPTION:
*       Checks whether a string is an identifier
*
*   RETURNS:
*       Returns TRUE if the string is an identifier
*       and FALSE if it isn't.
*
**************************************************/
boolean is_identifier
(
    char       *str     /* string to check      */
)
{
    return( is_in_map( __id_table, str ) );

}   /* is_identifier() */


/**************************************************
*
*   FUNCTION:
*       is_keyword - "Is Keyword"
*
*   DESCRIPTION:
*       Checks whether a string is a keyword
*
*   RETURNS:
*       Returns TRUE if the string is a keyword
*       and FALSE if it isn't.
*
**************************************************/
boolean is_keyword
(
    char       *str     /* string to check      */
)
{
    return( is_in_map( __keyword_table, str ) );

}   /* is_keyword() */


/**************************************************
*
*   FUNCTION:
*       get_token_data - "Get Token Data"
*
*   DESCRIPTION:
*       Retrieves the token data if the token
*       exists in the symbol table.
*
**************************************************/
struct token_type *get_token_data
(
    char       *str     /* string to check      */
)
{
    void *ret_val = get( __keyword_table, str );
    if( NULL != ret_val )
    {
        return( (struct token_type *)ret_val );
    }

    return( (struct token_type *)get( __id_table, str ) );

}   /* get_token_data() */


/**************************************************
*
*   FUNCTION:
*       update_symbol_table - "Update Symbol Table"
*
*   DESCRIPTION:
*       Adds an entry to the symbol table.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns SYM_NO_ERROR if there were
*         no errors.
*       * Returns SYM_UPDATE_ERROR if the
*         table couldn't be updated.
*
**************************************************/
sym_table_error_t8 update_symbol_table
(
    char               *str,    /* string to add                    */
    struct token_type  *data    /* token corresponding to string    */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    map_error_code_t8 error;

    error = add_map( __id_table, str, data, sizeof( *data ) );
    if( ERR_NO_ERROR != error )
    {
        return( SYM_UPDATE_ERROR );
    }
    return( SYM_NO_ERROR );

}   /* update_symbol_table() */


/**************************************************
*
*   FUNCTION:
*       print_table - "Print Table"
*
*   DESCRIPTION:
*       Prints the symbol table.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns SYM_PRINT_ERROR if there was
*         an error encountered while printing
*         anything
*       * Returns SYM_NO_ERROR if no errors were
*         encountered
*
**************************************************/
sym_table_error_t8 print_table
(
    void
)
{
    if( ERR_NO_ERROR != show_map( __keyword_table, __print_entry ) )
    {
        return( SYM_PRINT_ERROR );
    }

    if( ERR_NO_ERROR != show_map( __id_table, __print_entry ) )
    {
        return( SYM_PRINT_ERROR );
    }

    return( SYM_NO_ERROR );

}   /* print_table() */


/**************************************************
*
*   FUNCTION:
*       unload_tables - "Unload Tables"
*
*   DESCRIPTION:
*       Unloads (frees) the symbol table
*
**************************************************/
void unload_tables
(
    void
)
{
    free_map( __keyword_table );
    free_map( __id_table );

}   /*unload_tables() */
