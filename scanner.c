#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "types.h"
#include "tokens.h"
#include "symbol_table.h"

#define __MAX_WORD_LEN 255

enum
{
    NOT_LETTER_OR_NUMBER,
    IS_LETTER,
    IS_NUMBER
};

static char     __cur_char  = '\0';
static char     __prev_char = '\0';
static char     __next_char = '\0';
static uint32   __cur_col   = 0;
static uint32   __cur_line  = 0;

inline boolean __is_lower( char test )
{
    return( ( test >= 'a' ) && ( test <= 'z' ) );
}

inline boolean __is_number( char test )
{
    return( ( test >= '0' ) && ( test <= '9' ) );
}

inline boolean __is_upper( char test )
{
    return( ( test >= 'A' ) && ( test <= 'Z' ) );
}

inline boolean __is_letter( char test )
{
    return( __is_upper( test ) || __is_lower( test ) );
}

scanner_error_t8 init_scanner()
{
    sym_table_error_t8 error = init_symbol_table();
    if( ( SYM_NO_ERROR != error )
     && ( SYM_ALREADY_INITIALIZED != error ) )
    {
        return( SCN_INIT_ERROR );
    }

    return( SCN_NO_ERROR );

}   /* init_scanner() */

scanner_error_t8 unload_scanner()
{
    unload_tables();
    return( SCN_NO_ERROR );

}

char __get_next_char( FILE *f )
{
    do
    {
        __cur_col++;
        __prev_char = __cur_char;
        __cur_char = __next_char;

        if( EOF != __next_char )
        {
            __next_char = fgetc( f );
        }
    } while( ( ' ' == __cur_char ) || ( '\t' == __cur_char ) );

    if( '\n' == __cur_char )
    {
        __cur_line++;
        __cur_col = 0;
    }

    return( __cur_char );

}

void __write_error( sint8 err_code, uint32 err_line, uint32 err_col )
{
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
}

void __write_token( uint8 token_class, sint8 misc_info, char *lexeme )
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
            fprintf( stdout, "LITERAL %s\n", lexeme );
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
}

scanner_error_t8  tokenize
(
    char *file_name
)
{
    uint32      cur_idx;
    FILE       *f;
    char        read_char;
    char        word[ __MAX_WORD_LEN ];
    struct token_type token;
    uint32 start_col;
    boolean errored;
    boolean is_float;
    boolean dot_seen;

    f = fopen( file_name, "r" );
    if( NULL == f )
    {
        return( SCN_OPEN_ERROR );
    }

    cur_idx    = 0;
    __cur_col  = 0;
    __cur_line = 0;
    read_char = __get_next_char( f );
    while( EOF != read_char )
    {
        if( '\0' == read_char )
        {
            read_char = __get_next_char( f );
            continue;
        }

        cur_idx = 0;
        word[ cur_idx ] = '\0';

        switch( read_char )
        {
            case '<':
                if( '=' == __next_char )
                {
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_LT_OPP, word );
                break;

            case '=':
                if( ( '<' == __prev_char ) && ( ( ' ' != __prev_char ) || ( '\t' != __prev_char ) || ( '\n' != __prev_char ) ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_LE_OPP, word );
                    break;
                }
                else if( ( '>' == __prev_char ) && ( ( ' ' != __prev_char ) || ( '\t' != __prev_char ) || ( '\n' != __prev_char ) ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_GT_OPP, word );
                    break;
                }
                else if( ( ':' == __prev_char ) && ( ( ' ' != __prev_char ) || ( '\t' != __prev_char ) || ( '\n' != __prev_char ) ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_ASSN_OPP, word );
                    break;
                }
                else if( ( '!' == __prev_char ) && ( ( ' ' != __prev_char ) || ( '\t' != __prev_char ) || ( '\n' != __prev_char ) ) )
                {
                    sprintf( word, "%c%c", __prev_char, __cur_char );
                    __write_token( TOK_BINARY_OPP, TOK_NE_OPP, word );
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_EQ_OPP, word );
                break;

            case '>':
                if( '=' == __next_char )
                {
                    break;
                }

                sprintf( word, "%c", __cur_char );
                __write_token( TOK_BINARY_OPP, TOK_GT_OPP, word );
                break;

            case '!':
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
                if( '=' == __next_char )
                {
                    break;
                }

                __write_error( SCN_INVALID_OP, __cur_line, __cur_col );
                break;
            case ' ':
            case '\t':
            case '\n':
                break;

            case '"':
                start_col = __cur_col;
                errored = FALSE;
                do
                {
                    if( cur_idx >= __MAX_WORD_LEN )
                    {
                        __write_error( SCN_BUFFER_OVERFLOW, __cur_line, start_col );
                        errored = TRUE;
                        break;
                    }

                    word[ cur_idx++ ] = read_char;
                    word[ cur_idx ] = '\0';

                    read_char = __get_next_char( f );
                    if( ( '\n' == read_char ) || ( EOF == read_char ) )
                    {
                        __write_error( SCN_INFINITE_STRING, __cur_line, start_col );
                        errored = TRUE;
                        break;
                    }
                } while( '"' != read_char );

                if( !errored )
                {
                    if( __is_letter( __next_char ) || __is_number( __next_char ) )
                    {
                        __write_error( SCN_INVALID_CONSTANT, __cur_line, start_col );
                    }
                    else
                    {
                        __write_token( TOK_LITERAL, TOK_STRING_TYPE, word );
                    }
                }

                cur_idx = 0;
                word[ cur_idx ] = '\0';
                break;

            default:
                errored = FALSE;
                if( __is_number( read_char ) || ( '.' == read_char ) )
                {
                    is_float = FALSE;
                    dot_seen = FALSE;
                    start_col = __cur_col;
                    do
                    {
                        if( '.' == read_char )
                        {
                            is_float = TRUE;
                            if( dot_seen )
                            {
                                __write_error( SCN_INVALID_FLOAT, __cur_line, __cur_col );
                                errored = TRUE;
                                break;
                            }

                            dot_seen = TRUE;
                        }

                        word[ cur_idx++ ] = read_char;
                        word[ cur_idx ] = '\0';

                        if( __is_letter( __next_char ) )
                        {
                            __write_error( SCN_INVALID_CONSTANT, __cur_line, __cur_col );
                            errored = TRUE;
                            break;
                        }
                        else if( !__is_number( __next_char ) && ( '.' != __next_char ) )
                        {
                            break;
                        }

                        read_char = __get_next_char( f );
                    } while( ( ' '  != read_char )
                          && ( '\t' != read_char )
                          && ( '\n' != read_char ) );

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

                    cur_idx = 0;
                    word[ cur_idx ] = '\0';
                    break;
                }
                else if( __is_letter( read_char ) || ( '_' == read_char ) )
                {
                    do
                    {
                        word[ cur_idx++ ] = read_char;
                        word[ cur_idx ] = '\0';
                        if( !__is_letter( __next_char ) && !__is_number( __next_char ) && ( '_' != __next_char ) )
                        {
                            if( '.' == __next_char )
                            {
                                __write_error( SCN_INVALID_IDENTIFIER, __cur_line, __cur_col + 1 );
                                errored = TRUE;
                                break;
                            }

                            break;
                        }

                        read_char = __get_next_char( f );

                    } while( ( ' '  != read_char )
                          && ( '\t' != read_char )
                          && ( '\n' != read_char ) );

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

                    cur_idx = 0;
                    word[ cur_idx ] = '\0';
                    break;
                }
                else
                {
                    __write_error( SCN_INVALID_OP, __cur_line, __cur_col );
                    break;
                }
        }
        read_char = __get_next_char( f );
     }

    fclose( f );
    return( SCN_NO_ERROR );

}   /* tokenize() */
