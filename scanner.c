/**************************************************
*
*   MODULE:
*       scanner.c
*
*   DESCRIPTION:
*       Implementation of the lexical scanner
*
**************************************************/

/*-------------------------------------------------
                 PROJECT INCLUDES
-------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "symbol_table.h"
#include "tokens.h"
#include "types.h"

/*-------------------------------------------------
                LITERAL CONSTANTS
-------------------------------------------------*/

#define __MAX_WORD_LEN 255              /* maximum length allowed in a word     */

/*-------------------------------------------------
                GLOBAL VARIABLES
-------------------------------------------------*/

static char     __cur_char  = '\0';     /* current character from input file    */
static char     __prev_char = '\0';     /* previous character from input file   */
static char     __next_char = '\0';     /* next character in the input file     */
static uint32   __cur_col   = 0;        /* current column counter               */
static uint32   __cur_line  = 0;        /* current line counter                 */
static uint32   __num_errors= 0;        /* number of encountered scanning errors*/
static boolean  __space_seen = FALSE;   /* did we encounter a whitespace char?  */

/*-------------------------------------------------
                FUNCTION PROTOTYPES
-------------------------------------------------*/

inline boolean __is_letter
(
    char        test        /* character to test    */
);

inline boolean __is_lower
(
    char        test        /* character to test    */
);

inline boolean __is_number
(
    char        test        /* character to test    */
);

inline boolean __is_upper
(
    char        test        /* character to test    */
);

char __get_next_char
(
    FILE       *f           /* file to process      */
);

void __write_error
(
    sint8       err_code,   /* error code               */
    uint32      err_line,   /* line error occurred on   */
    uint32      err_col     /* column error occurred on */
);

void __write_token
(
    uint8       token_class,/* token class          */
    sint8       misc_info,  /* other info           */
    char       *lexeme      /* lexeme to print      */
);

/*-------------------------------------------------
                        MACROS
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       __is_letter - "Is Letter"
*
*   DESCRIPTION:
*       Determines whether a character is a
*       letter.
*
*   RETURNS:
*       Returns TRUE if test is a letter,
*       and FALSE if it isn't.
*
**************************************************/
inline boolean __is_letter
(
    char        test        /* character to test    */
)
{
    return( __is_upper( test ) || __is_lower( test ) );

}   /* __is_letter() */


/**************************************************
*
*   FUNCTION:
*       __is_lower - "Is Lower"
*
*   DESCRIPTION:
*       Determines whether a character is a
*       lowercase letter.
*
*   RETURNS:
*       Returns TRUE if test is lowercase,
*       and FALSE if it's uppercase.
*
**************************************************/
inline boolean __is_lower
(
    char        test        /* character to test    */
)
{
    return( ( test >= 'a' ) && ( test <= 'z' ) );

}   /* __is_lower() */


/**************************************************
*
*   FUNCTION:
*       __is_number - "Is Number"
*
*   DESCRIPTION:
*       Determines whether a character is a
*       number.
*
*   RETURNS:
*       Returns TRUE if test is a number,
*       and FALSE if it isn't.
*
**************************************************/
inline boolean __is_number
(
    char        test        /* character to test    */
)
{
    return( ( test >= '0' ) && ( test <= '9' ) );

}   /* __is_number() */


/**************************************************
*
*   FUNCTION:
*       __is_upper - "Is Upper"
*
*   DESCRIPTION:
*       Determines whether a character is an
*       uppercase letter.
*
*   RETURNS:
*       Returns TRUE if test is uppercase,
*       and FALSE if it isn't.
*
**************************************************/
inline boolean __is_upper
(
    char        test        /* character to test    */
)
{
    return( ( test >= 'A' ) && ( test <= 'Z' ) );

}   /* __is_upper() */

/*-------------------------------------------------
                      PROCEDURES
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       __get_next_char - "Get Next Character"
*
*   DESCRIPTION:
*       Grabs the next character from a file
*
*   RETURNS:
*       Returns the next character in a file
*
**************************************************/
char __get_next_char
(
    FILE       *f           /* file to process      */
)
{
    __cur_col++;
    __prev_char = __cur_char;
    __cur_char = __next_char;

    if( ( '\t' == __cur_char ) || ( ' ' == __cur_char ) || ( '\n' == __cur_char ) )
    {
        __space_seen = TRUE;
    }
    else
    {
        __space_seen = FALSE;
    }

    if( EOF != __next_char )
    {
        __next_char = fgetc( f );
    }

    /*---------------------------------
    Update the current line and
    current column counters
    ---------------------------------*/
    if( '\n' == __cur_char )
    {
        __cur_line++;
        __cur_col = 0;
    }

    return( __cur_char );

}   /* __get_next_char() */


/**************************************************
*
*   FUNCTION:
*       __write_error - "Write Error"
*
*   DESCRIPTION:
*       Writes an error message to stderr
*
**************************************************/
void __write_error
(
    sint8       err_code,   /* error code               */
    uint32      err_line,   /* line error occurred on   */
    uint32      err_col     /* column error occurred on */
)
{
    ++__num_errors;

    switch( err_code )
    {
        case SCN_INVALID_OP:
            fprintf( stderr, "SCANNER ERROR: Invalid operation. %lu:%lu\n", err_line, err_col );
            break;

        case SCN_BUFFER_OVERFLOW:
            fprintf( stderr, "SCANNER ERROR: Buffer overflow. %lu:%lu\n", err_line, err_col );
            break;

        case SCN_INFINITE_STRING:
            fprintf( stderr, "SCANNER ERROR: String constant doesn't end. %lu:%lu\n", err_line, err_col );
            break;

        case SCN_INVALID_CONSTANT:
            fprintf( stderr, "SCANNER ERROR: Invalid constant. %lu:%lu\n", err_line, err_col );
            break;

        case SCN_INVALID_FLOAT:
            fprintf( stderr, "SCANNER_ERROR: Invalid float constant. %lu:%lu\n", err_line, err_col );
            break;

        case SCN_INVALID_IDENTIFIER:
            fprintf( stderr, "SCANNER ERROR: Invalid identifier. %lu:%lu\n", err_line, err_col );
            break;

        default:
            fprintf( stderr, "SCANNER ERROR: Error code nonexistent.");
            break;
    }

}   /* __write_error() */


/**************************************************
*
*   FUNCTION:
*       __write_token - "Write Token"
*
*   DESCRIPTION:
*       Writes token data to stdout
*
**************************************************/
void __write_token
(
    uint8       token_class,/* token class          */
    sint8       misc_info,  /* other info           */
    char       *lexeme      /* lexeme to print      */
)
{
    switch( token_class )
    {
        case TOK_BINARY_OPP:
            fprintf( stdout, "BINOP %s\n", lexeme );
            break;

        case TOK_UNARY_OPP:
            fprintf( stdout, "UNOP %s\n", lexeme );
            break;

        case TOK_IDENT:
            fprintf( stdout, "IDENTIFIER %s\n", lexeme );
            break;

        case TOK_LITERAL:
            switch( misc_info )
            {
                case TOK_INT_TYPE:
                    fprintf( stdout, "LITERAL int %s\n", lexeme );
                    break;

                case TOK_REAL_TYPE:
                    fprintf( stdout, "LITERAL float %s\n", lexeme );
                    break;

                case TOK_STRING_TYPE:
                    fprintf( stdout, "LITERAL string %s\n", lexeme );
                    break;

                case TOK_BOOL_TYPE:
                    fprintf( stdout, "LITERAL bool %s\n", lexeme );
                    break;

                default:
                    fprintf( stderr, "Invalid generic type code\n" );
            }
            break;

        case TOK_LIST_TYPE:
            fprintf( stdout, "LIST %s\n", lexeme );
            break;

        case TOK_RESERVED_WORD:
            fprintf( stdout, "KEYWORD %s\n", lexeme );
            break;

        default:
            fprintf( stderr, "Unknown token class.\n" );
            break;
    }

}   /* __write_token() */


/**************************************************
*
*   FUNCTION:
*       get_total_scanner_errors - "Get Total
*                                   [Number of]
*                                   Scanner Errors"
*
*   DESCRIPTION:
*       Returns the total number of scanner
*       errors
*
**************************************************/
uint32 get_total_scanner_errors
(
    void
)
{
    return( __num_errors );

}   /* get_total_scanner_errors() */


/**************************************************
*
*   FUNCTION:
*       init_scanner - "Initialize Scanner"
*
*   DESCCRIPTION:
*       Initializes the scanner for use
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns SCN_NO_ERROR if there were
*         no errors
*       * Returns SCN_INIT_ERROR if there were
*         problems initializing the scanner
*
**************************************************/
scanner_error_t8 init_scanner
(
    void
)
{
    /*---------------------------------
    Initialize the symbol table
    ---------------------------------*/
    sym_table_error_t8 error = init_symbol_table();
    if( ( SYM_NO_ERROR != error )
     && ( SYM_ALREADY_INITIALIZED != error ) )
    {
        return( SCN_INIT_ERROR );
    }

    return( SCN_NO_ERROR );

}   /* init_scanner() */


/**************************************************
*
*   FUNCTION:
*       tokenize - "Tokenize"
*
*   DESCRIPTION:
*       Tokenizes a file. All of the proccessed
*       tokens are written to stdout.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns SCN_NO_ERROR if there were
*         no errors
*       * Returns SCN_OPEN_ERROR if there was
*         an issue opening a file
*
*   TODO:
*       CLEAN THIS UP!
*
**************************************************/
scanner_error_t8  tokenize
(
    char       *file_name   /* file to scan     */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32      cur_idx;                /* current index in word buffer */
    boolean     dot_seen;               /* have we already seen a       */
                                        /*  decimal point (in a float)? */
    boolean     exp_seen;               /* did we see an 'e' in a float?*/
    boolean     errored;                /* did we see a scanning error? */
    FILE       *f;                      /* file pointer                 */
    boolean     is_float;               /* are we processing a float?   */
    char        read_char;              /* character read from file     */
    boolean     skip;
    uint32      start_col;              /* starting column              */
    struct token_type
                token;                  /* identifier token to create   */
    char        word[ __MAX_WORD_LEN ]; /* word buffer                  */

    /*---------------------------------
    Attempt to open the file
    ---------------------------------*/
    f = fopen( file_name, "r" );
    if( NULL == f )
    {
        return( SCN_OPEN_ERROR );
    }

    /*---------------------------------
    Initialize some of the variables
    ---------------------------------*/
    cur_idx    = 0;
    __cur_col  = 1;
    __cur_line = 1;

    /*---------------------------------
    Grab the first character from
    the file and continue to read
    until the end of file is reached
    ---------------------------------*/
    read_char = __get_next_char( f );
    while( EOF != read_char )
    {
        skip = FALSE;

        /*-----------------------------
        Check if we have a NULL
        terminator
        -----------------------------*/
        if( '\0' == read_char )
        {
            read_char = __get_next_char( f );
            continue;
        }

        /*-----------------------------
        Ensure that our index and
        buffer are ready for some
        work
        -----------------------------*/
        cur_idx = 0;
        word[ cur_idx ] = '\0';

        /*-----------------------------
        and now for the ugly part...
        -----------------------------*/
        switch( read_char )
        {
            case '<':
                /*--------------------
                Check if we will have
                a '<=' symbol
                ---------------------*/
                if( '=' == __next_char )
                {
                    break;
                }

                /*---------------------
                Create the token string
                and write the token
                ---------------------*/
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_LT_OPP, word );
                break;

            case '=':
                /*---------------------
                These check whether the
                previous character was
                any symbol preceding
                an equal sign (i.e.
                <=, >=, !=, :=) <-- not
                planned
                ---------------------*/
                if( ( '<' == __prev_char )
                  && !( __space_seen ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_LE_OPP, word );
                    break;
                }
                else if( ( '>' == __prev_char )
                     && !( __space_seen ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_GT_OPP, word );
                    break;
                }
                else if( ( ':' == __prev_char  )
                     && !( __space_seen ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_ASSN_OPP, word );
                    break;
                }
                else if( ( '!' == __prev_char )
                     && !( __space_seen ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_NE_OPP, word );
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_EQ_OPP, word );
                break;

            case '>':
                /*---------------------
                Check if the symbol
                will be a <= sign
                ---------------------*/
                if( '=' == __next_char )
                {
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_GT_OPP, word );
                break;

            case '!':
                /*---------------------
                Check if the symbol
                will be a != sign
                ---------------------*/
                if( '=' == __next_char )
                {
                    break;
                }

                __write_error( SCN_INVALID_OP, __cur_line, __cur_col );
                break;

            case '%':
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_MOD_OPP, word );
                break;

            case '^':
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_EXP_OPP, word );
                break;

            case '-':
                /*---------------------
                If a sub op is directly
                adjacent to a letter
                or a number, then it
                becomes a unary op.
                ---------------------*/
                if( __is_number( __next_char ) || ( __is_letter( __next_char ) ) )
                {
                    sprintf( word, "%c", __cur_char );
                    __write_token( TOK_UNARY_OPP, TOK_NEG_OPP, word );
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_SUB_OPP, word );
                break;

            case '+':
                /*---------------------
                If an add op is directly
                adjacent to a letter
                or a number, then it
                becomes a unary op.
                ---------------------*/
                if( __is_number( __next_char ) || __is_letter( __next_char ) )
                {
                    sprintf( word, "%c", __cur_char );
                    __write_token( TOK_UNARY_OPP, TOK_POS_OPP, word );
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_ADD_OPP, word );
                break;

            case '*':
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_MUL_OPP, word );
                break;

            case '/':
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_DIV_OPP, word );
                break;

            case '[':
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_LIST_TYPE, TOK_LIST_BEGIN, word );
                break;

            case ']':
                sprintf( word, "%c", __cur_char );
                __write_token( TOK_LIST_TYPE, TOK_LIST_END, word );
                break;

            case ':':
                /*---------------------
                Unless the colon is
                followed by an equal
                sign, it isn't in our
                grammar.
                ---------------------*/
                if( '=' == __next_char )
                {
                    break;
                }

                __write_error( SCN_INVALID_OP, __cur_line, __cur_col );
                break;
            case ' ':
            case '\t':
            case '\n':
                /*---------------------
                We break for whitespace
                ---------------------*/
                break;

            case '"':
                /*---------------------
                If a string symbol is
                encountered, read from
                the file until we either
                see the corresponding
                end string, we hit a
                newline, or we blow our
                buffer.
                ---------------------*/
                start_col = __cur_col;
                errored = FALSE;
                do
                {
                    /*-----------------
                    Check for buffer
                    overflow
                    -----------------*/
                    if( cur_idx >= __MAX_WORD_LEN )
                    {
                        __write_error( SCN_BUFFER_OVERFLOW, __cur_line, start_col );
                        errored = TRUE;
                        /*-------------
                        Grab all chars
                        until the string
                        ends
                        -------------*/
                        do
                        {
                            read_char = __get_next_char( f );
                        } while( !__space_seen && ( '"' != read_char ) );
                        break;
                    }

                    /*-----------------
                    Copy the character
                    to the word buffer
                    -----------------*/
                    word[ cur_idx++ ] = read_char;
                    word[ cur_idx ] = '\0';

                    /*-----------------
                    Get the next char
                    -----------------*/
                    read_char = __get_next_char( f );

                    /*-----------------
                    Check for newlines
                    or end of file
                    -----------------*/
                    if( ( '\n' == read_char ) || ( EOF == read_char ) )
                    {
                        __write_error( SCN_INFINITE_STRING, __cur_line, start_col );
                        errored = TRUE;
                        break;
                    }
                } while( '"' != read_char );

                /*---------------------
                Only write the token
                if there wasn't an
                error
                ---------------------*/
                if( !errored )
                {
                    if( __is_letter( __next_char ) || __is_number( __next_char ) )
                    {
                        __write_error( SCN_INVALID_CONSTANT, __cur_line, start_col );
                    }
                    else
                    {
                        word[ cur_idx++ ] = '"';
                        word[ cur_idx ]   = '\0';
                        __write_token( TOK_LITERAL, TOK_STRING_TYPE, word );
                    }
                }

                /*---------------------
                Reset the word buffer
                ---------------------*/
                cur_idx = 0;
                word[ cur_idx ] = '\0';
                break;

            default:
                errored = FALSE;
                if( __is_number( read_char ) || ( '.' == read_char ) )
                {
                    /*-----------------
                    Read in a numeric
                    literal
                    -----------------*/
                    is_float = FALSE;
                    dot_seen = FALSE;
                    exp_seen = FALSE;
                    start_col = __cur_col;
                    do
                    {
                        /*-------------
                        Check if we've
                        seen multiple
                        decimal points
                        -------------*/
                        if( '.' == read_char )
                        {
                            is_float = TRUE;
                            if( dot_seen )
                            {
                                __write_error( SCN_INVALID_FLOAT, __cur_line, __cur_col );
                                /*---------------
                                Read until we
                                get to the next
                                token
                                ---------------*/
                                while( !( __space_seen )
                                     && ( __is_number( __next_char )
                                       || __is_letter( __next_char )
                                       || ( '.' == __next_char ) ) )
                                {
                                    read_char = __get_next_char( f );
                                }
                                errored = TRUE;
                                break;
                            }

                            dot_seen = TRUE;
                        }

                        if( ( 'e' == read_char ) || ( 'E' == read_char ) )
                        {
                            is_float = TRUE;
                            if( exp_seen )
                            {
                                __write_error( SCN_INVALID_FLOAT, __cur_line, start_col );
                                while( !( __space_seen )
                                     && ( __is_number( __next_char )
                                       || __is_letter( __next_char )
                                       || ( '.' == __next_char ) ) )
                                {
                                    read_char = __get_next_char( f );
                                }

                                errored = TRUE;
                                break;
                            }

                            exp_seen = TRUE;
                        }

                        if( ( ( '+' == __cur_char ) || ( '-' == __cur_char ) )
                          && !( __is_letter( __next_char ) )
                          && !( __is_number( __next_char ) ) )
                        {
                            skip = TRUE;
                            break;
                        }

                        /*-------------
                        Copy the char
                        to the word
                        buffer
                        -------------*/
                        word[ cur_idx++ ] = read_char;
                        word[ cur_idx ] = '\0';

                        /*-------------
                        Check if a
                        letter or a
                        different symbol
                        is the next char
                        -------------*/
                        if( __is_letter( __next_char )
                            && ( __next_char != 'e' )
                            && ( __next_char != 'E' ) )
                        {
                            __write_error( SCN_INVALID_CONSTANT, __cur_line, __cur_col );
                            /*-------------------
                            Read until we get to
                            the next token
                            -------------------*/
                            while( !( __space_seen )
                                 && ( __is_number( __next_char )
                                   || __is_letter( __next_char )
                                   || ( '.' == __next_char ) ) )
                                {
                                    read_char = __get_next_char( f );
                                }
                            errored = TRUE;
                            break;
                        }
                        else if( ( ( '+' == __next_char )
                                || ( '-' == __next_char ) )
                              && ( 'e' != __cur_char  )
                              && ( 'E' != __cur_char  ) )
                        {
                            break;
                        }
                        else if( !__is_number( __next_char )
                            && ( '.' != __next_char )
                            && ( __next_char != 'e' )
                            && ( __next_char != 'E' )
                            && ( __next_char != '+' )
                            && ( __next_char != '-' ) )
                        {
                            break;
                        }

                        /*-------------
                        Get the next
                        character
                        -------------*/
                        read_char = __get_next_char( f );
                    } while( !__space_seen );

                    /*-----------------
                    Only write the
                    constant if we
                    didn't blow up
                    -----------------*/
                    if( !errored )
                    {
                        if( is_float )
                        {
                            __write_token( TOK_LITERAL, TOK_REAL_TYPE, word );
                        }
                        else
                        {
                            __write_token( TOK_LITERAL, TOK_INT_TYPE, word );
                        }
                    }

                    /*-----------------
                    Reset the word buf
                    -----------------*/
                    cur_idx = 0;
                    word[ cur_idx ] = '\0';
                    break;
                }
                else if( __is_letter( read_char ) || ( '_' == read_char ) )
                {
                    /*-----------------
                    Read an identifier
                    from the file
                    -----------------*/
                    do
                    {
                        /*-------------
                        Copy the char
                        to the word buf
                        -------------*/
                        word[ cur_idx++ ] = read_char;
                        word[ cur_idx ] = '\0';

                        /*-------------
                        If the next char
                        isn't a letter,
                        number, or underscore
                        character, then the
                        identifier is
                        done being scanned
                        -------------*/
                        if( !__is_letter( __next_char )
                         && !__is_number( __next_char )
                         && ( '_' != __next_char      ) )
                        {
                            if( '.' == __next_char )
                            {
                                __write_error( SCN_INVALID_IDENTIFIER, __cur_line, __cur_col + 1 );
                                /*-------------
                                Grab all chars
                                until the
                                identifier str
                                ends
                                -------------*/
                                while( !( __space_seen )
                                     && ( __is_number( __next_char )
                                       || __is_letter( __next_char )
                                       || ( '.' == __next_char ) ) )
                                {
                                    read_char = __get_next_char( f );
                                }
                                errored = TRUE;
                                break;
                            }

                            break;
                        }

                        /*-------------
                        Grab the next
                        character
                        -------------*/
                        read_char = __get_next_char( f );

                    } while( !__space_seen );

                    /*-----------------
                    Add the identifier
                    to the symbol table
                    and write the token
                    only if we haven't
                    hit an error
                    -----------------*/
                    if( !errored )
                    {
                        if( TOK_NO_CLASS == is_keyword( word ) )
                        {
                            token.token_class = TOK_IDENT;
                            strcpy( token.id.in_str, word );
                            update_symbol_table( word, &token );
                            __write_token( TOK_IDENT, -1, word );
                        }
                        else
                        {
                            __write_token( TOK_RESERVED_WORD, -1, word );
                        }
                    }

                    /*-----------------
                    Reset the word buf
                    -----------------*/
                    cur_idx = 0;
                    word[ cur_idx ] = '\0';
                    break;
                }
                else
                {
                    /*-----------------
                    Character isn't in
                    language
                    -----------------*/
                    __write_error( SCN_INVALID_OP, __cur_line, __cur_col );
                    break;
                }
        }

        /*-----------------------------
        Grab next character from the
        file
        -----------------------------*/
        if( !skip )
        {
            read_char = __get_next_char( f );
        }
     }

    /*---------------------------------
    Close the file and return
    ---------------------------------*/
    fclose( f );
    return( SCN_NO_ERROR );

}   /* tokenize() */


/**************************************************
*
*   FUNCTION:
*       unload_scanner - "Unload Scanner"
*
*   DESCRIPTION:
*       Tears down the scanner.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns SCN_NO_ERROR if there were no
*         errors
*
**************************************************/
scanner_error_t8 unload_scanner
(
    void
)
{
    unload_tables();
    return( SCN_NO_ERROR );

}   /* unload_scanner() */
