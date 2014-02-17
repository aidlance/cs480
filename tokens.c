#include <stdio.h>

#include "tokens.h"

token_error_t8 print_minimal_token
(
    Token *tok
)
{
    if( NULL == tok )
    {
        return( TOK_NULL_REF );
    }

    switch( tok->tok_class )
    {
        case TOK_BINARY_OPP:
            switch( tok->binop )
            {
                case TOK_ADD_OPP:
                    fprintf( stdout, "BINOP (+)\n" );
                    break;
                case TOK_SUB_OPP:
                    fprintf( stdout, "BINOP (-)\n" );
                    break;
                case TOK_MUL_OPP:
                    fprintf( stdout, "BINOP (*)\n" );
                    break;
                case TOK_DIV_OPP:
                    fprintf( stdout, "BINOP (/)\n" );
                    break;
                case TOK_EXP_OPP:
                    fprintf( stdout, "BINOP (^)\n" );
                    break;
                case TOK_MOD_OPP:
                    fprintf( stdout, "BINOP (\%)\n" );
                    break;
                case TOK_AND_OPP:
                    fprintf( stdout, "BINOP (and)\n" );
                    break;
                case TOK_OR_OPP:
                    fprintf( stdout, "BINOP (or)\n" );
                    break;
                case TOK_EQ_OPP:
                    fprintf( stdout, "BINOP (=)\n" );
                    break;
                 case TOK_LE_OPP:
                    fprintf( stdout, "BINOP (<=)\n" );
                    break;
                case TOK_GE_OPP:
                    fprintf( stdout, "BINOP (>=)\n" );
                    break;
                case TOK_LT_OPP:
                    fprintf( stdout, "BINOP (<)\n" );
                    break;
                case TOK_GT_OPP:
                    fprintf( stdout, "BINOP (>)\n" );
                    break;
                case TOK_NE_OPP:
                    fprintf( stdout, "BINOP (!=)\n" );
                    break;
                case TOK_STR_CAT_OPP:
                    fprintf( stdout, "BINOP (+)\n" );
                    break;
                default:
                    fprintf( stderr, "Invalid binop code: %i\n", tok->binop );
                    return( TOK_INVALID_TOKEN );
            }
            break;
        case TOK_UNARY_OPP:
            switch( tok->unop )
            {
                case TOK_SIN_OPP:
                    fprintf( stdout, "UNOP (sin)\n" );
                    break;
                case TOK_COS_OPP:
                    fprintf( stdout, "UNOP (cos)\n" );
                    break;
                case TOK_TAN_OPP:
                    fprintf( stdout, "UNOP (tan)\n" );
                    break;
                case TOK_NOT_OPP:
                    fprintf( stdout, "UNOP (not)\n" );
                    break;
                case TOK_POS_OPP:
                    fprintf( stdout, "UNOP (+)\n" );
                    break;
                case TOK_NEG_OPP:
                    fprintf( stdout, "UNOP (-)\n" );
                    break;
                default:
                    fprintf( stderr, "Invalid unop code: %i\n", tok->unop );
                    return( TOK_INVALID_TOKEN );
            }
            break;
        case TOK_LITERAL:
            switch( tok->type )
            {
                case TOK_INT_TYPE:
                    fprintf( stdout, "LITERAL (int: %s)\n", tok->literal_str );
                    break;
                case TOK_REAL_TYPE:
                    fprintf( stdout, "LITERAL (float: %s)\n", tok->literal_str );
                    break;
                case TOK_STRING_TYPE:
                    fprintf( stdout, "LITERAL (string: %s)\n", tok->literal_str );
                    break;
                case TOK_BOOL_TYPE:
                    fprintf( stdout, "LITERAL (bool: %s)\n", tok->literal_str );
                    break;
                default:
                    fprintf( stderr, "Invalid literal type: %i\n", tok->type );
                    return( TOK_INVALID_TOKEN );
            }
            break;
        case TOK_RESERVED_WORD:
            switch( tok->stmt )
            {
                case TOK_WHILE:
                    fprintf( stdout, "while\n" );
                    break;
                case TOK_IF:
                    fprintf( stdout, "if\n" );
                    break;
                case TOK_BOOL:
                    fprintf( stdout, "bool\n" );
                    break;
                case TOK_INT:
                    fprintf( stdout, "int\n" );
                    break;
                case TOK_STRING:
                    fprintf( stdout, "string\n" );
                    break;
                case TOK_REAL:
                    fprintf( stdout, "float\n" );
                    break;
                case TOK_STDOUT:
                    fprintf( stdout, "stdout\n" );
                    break;
                case TOK_LET:
                    fprintf( stdout, "let\n" );
                    break;
                default:
                    fprintf( stderr, "Invalid reserved word: %i\n", tok->stmt );
                    return( TOK_INVALID_TOKEN );
            }
            break;
        case TOK_IDENT:
            fprintf( stdout, "id\n" );
            break;
        case TOK_ASSN_CLASS:
            fprintf( stdout, ":=\n" );
            break;
        case TOK_LIST_TYPE:
            switch( tok->list )
            {
                case TOK_LIST_BEGIN:
                    fprintf( stdout, "[\n" );
                    break;
                case TOK_LIST_END:
                    fprintf( stdout, "]\n" );
                    break;
                default:
                    fprintf( stderr, "Invalid list type: %i\n", tok->list );
                    return( TOK_INVALID_TOKEN );
            }
            break;
    }

    return( TOK_NO_ERROR );

}   /* print_minimal_token() */
