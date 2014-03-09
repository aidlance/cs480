/***************************************************************************************//**
 *
 *  MODULE NAME:
 *      [ filename ] - [ short description ]
 *
 *  DESCRIPTION:
 *      [ description of file's usability ]
 *
 ******************************************************************************************/

/*------------------------------------------------------------------------------------------
                                     GENERAL INCLUDES
------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_gen.h"
#include "parser.h"
#include "symbol_table.h"
#include "tokens.h"
#include "types.h"

/*------------------------------------------------------------------------------------------
                                     LITERAL CONSTANTS
------------------------------------------------------------------------------------------*/

#define FORTH_EXTENSION ".fs"
#define FNAME_MAX_LENGTH 255
#define MAX_COMMAND_LENGTH 255

#define NUM_CHILDREN_IF_THEN 2
#define NUM_CILDREN_IF_ELSE  3

//#define __CGEN_DEBUG

/*------------------------------------------------------------------------------------------
                                          TYPES
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                    VARIABLE CONSTANTS
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                        MACROS
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                      PROCEDURES
------------------------------------------------------------------------------------------*/
static boolean __in_let = FALSE;
static boolean __str_cat_set = FALSE;
static FILE *__cur_file = NULL;

typedef struct
{
    uint num_strings;
    uint num_bools;
    uint num_floats;
    uint num_ints;
} __var_counter;

FILE *__open_outfile
(
    char *input_filename
);

cgen_error_t8 __walk_thru_nodes( Node *n, type_class_t8 *type );
char *__get_forth_command
(
    Token *tok,
    type_class_t8 tok_type
);

cgen_error_t8 __gen_code_binop( Node *n, type_class_t8 *type )
{
    boolean is_str_cat = __str_cat_set;
    cgen_error_t8 err_code;
    type_class_t8 old_type;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    old_type = *type;
    if( TOK_STRING_TYPE == old_type )
    {
        if( is_str_cat )
        {
            fprintf( __cur_file, "pad +place pad count " );
        }
        else
        {
            __str_cat_set = TRUE;
            fprintf( __cur_file, "pad place pad count " );
        }
    }

    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( TOK_INT_TYPE == old_type )
    {
        switch( *type )
        {
            case TOK_REAL_TYPE:
                fprintf( __cur_file, "s>f fswap %s ", __get_forth_command( &( n->tok ), *type ) );
                break;

            case TOK_INT_TYPE:
                fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );
                break;

            case TOK_BOOL_TYPE:
            default:
                return( CGEN_SEM_ERROR );
        }
    }
    else if( TOK_REAL_TYPE == old_type )
    {
        switch( *type )
        {
            case TOK_REAL_TYPE:
                fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );
                break;

            case TOK_INT_TYPE:
                *type = TOK_REAL_TYPE;
                fprintf( __cur_file, "s>f %s ", __get_forth_command( &( n->tok ), *type ) );
                break;

            case TOK_BOOL_TYPE:
            default:
                return( CGEN_SEM_ERROR );
        }
    }
    else if( TOK_BOOL_TYPE == old_type )
    {
        if( *type != old_type )
        {
            return( CGEN_SEM_ERROR );
        }

        fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );
    }
    else if( TOK_STRING_TYPE == old_type )
    {
        if( *type != old_type )
        {
            return( CGEN_SEM_ERROR );
        }

        if( TOK_ADD_OPP != n->tok.binop )
        {
            return( CGEN_SEM_ERROR );
        }

        if( __str_cat_set )
        {
            __str_cat_set = FALSE;
            fprintf( __cur_file, "pad +place pad count " );
        }
    }
    else
    {
        return( CGEN_SEM_ERROR );
    }

    switch( n->tok.binop )
    {
        case TOK_LT_OPP:
        case TOK_LE_OPP:
        case TOK_GE_OPP:
        case TOK_GT_OPP:
        case TOK_EQ_OPP:
        case TOK_NE_OPP:
        case TOK_AND_OPP:
        case TOK_OR_OPP:
            *type = TOK_BOOL_TYPE;
            break;
    }

    //fprintf( __cur_file, "\n" );
    if( !is_str_cat  )
    {
        __str_cat_set = FALSE;
    }
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_unop( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( NULL != n->first_child->next )
    {
        return( CGEN_SEM_ERROR );
    }

    if( TOK_REAL_TYPE == *type )
    {
        switch( n->tok.unop )
        {
            case TOK_SIN_OPP:
            case TOK_COS_OPP:
            case TOK_TAN_OPP:
            case TOK_NEG_OPP:
                fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );

            case TOK_POS_OPP:
                break;

            default:
                return( CGEN_SEM_ERROR );
        }
    }
    else if( TOK_INT_TYPE == *type )
    {
        switch( n->tok.unop )
        {
            case TOK_SIN_OPP:
            case TOK_COS_OPP:
            case TOK_TAN_OPP:
                fprintf( __cur_file, "s>f %s ", __get_forth_command( &( n->tok ), *type ) );
                *type = TOK_REAL_TYPE;
                break;

            case TOK_NEG_OPP:
                fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );

            case TOK_POS_OPP:
                break;

            default:
                return( CGEN_SEM_ERROR );
        }
    }
    else if( TOK_BOOL_TYPE == *type )
    {
        if( TOK_NOT_OPP != n->tok.unop )
        {
            return( CGEN_SEM_ERROR );
        }

        fprintf( __cur_file, "invert " );
        return( CGEN_NO_ERROR );
    }
    else
    {
        return( CGEN_SEM_ERROR );
    }

    //fprintf( __cur_file, "\n" );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_literal( Node *n, type_class_t8 *type )
{
    char *str;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( NULL != n->first_child )
    {
        return( CGEN_SEM_ERROR );
    }

    *type = n->tok.type;
    str = __get_forth_command( &( n->tok ), *type );
    fprintf( __cur_file, "%s ", NULL == str ? "" : str );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_exp( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;
    type_class_t8 old_type;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    old_type = *type;

    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( NULL != n->first_child->next->next )
    {
        return( CGEN_SEM_ERROR );
    }

    if( TOK_INT_TYPE == old_type )
    {
        if( TOK_REAL_TYPE == *type )
        {
            fprintf( __cur_file, "s>f fswap f** " );
        }
        else if( TOK_INT_TYPE == *type )
        {
            fprintf( __cur_file, "s>f s>f fswap f** f>s " );
        }
        else
        {
            return( CGEN_SEM_ERROR );
        }
    }
    else if( TOK_REAL_TYPE == old_type )
    {
        if( TOK_REAL_TYPE == *type )
        {
            fprintf( __cur_file, "f** " );
        }
        else if( TOK_INT_TYPE == *type )
        {
            fprintf( __cur_file, "s>f f** " );
            *type = TOK_REAL_TYPE;
        }
        else
        {
            return( CGEN_SEM_ERROR );
        }
    }
    else
    {
        return( CGEN_SEM_ERROR );
    }

    //fprintf( __cur_file, "\n" );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_if_branches( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_SEM_ERROR );
    }

    if( ( 1 != n->num_children ) && ( 2 != n->num_children ) )
    {
        return( CGEN_SEM_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( 2 == n->num_children )
    {
        fprintf( __cur_file, "else " );
        err_code = __walk_thru_nodes( n->last_child->tree_next, type );
        if( CGEN_NO_ERROR != err_code )
        {
            return( err_code );
        }
    }

    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_if( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( NUM_CHILDREN_IF_THEN != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( TOK_BOOL_TYPE != *type )
    {
        return( CGEN_SEM_ERROR );
    }

    fprintf( __cur_file, "if " );
    err_code = __gen_code_if_branches( n->last_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    fprintf( __cur_file, "then " );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_while( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    fprintf( __cur_file, "begin " );
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( TOK_BOOL_TYPE != *type )
    {
        return( CGEN_SEM_ERROR );
    }

    fprintf( __cur_file, "while " );

    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    fprintf( __cur_file, "repeat " );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_mod( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;
    type_class_t8 old_type;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    old_type = *type;

    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( NULL != n->first_child->next->next )
    {
        return( CGEN_SEM_ERROR );
    }

    if( TOK_INT_TYPE == old_type )
    {
        if( TOK_REAL_TYPE == *type )
        {
            fprintf( __cur_file, "s>f fswap f/ fdup floor f- fabs " );
        }
        else if( TOK_INT_TYPE == *type )
        {
            fprintf( __cur_file, "mod " );
        }
        else
        {
            return( CGEN_SEM_ERROR );
        }
    }
    else if( TOK_REAL_TYPE == old_type )
    {
        if( TOK_REAL_TYPE == *type )
        {
            fprintf( __cur_file, "f/ fdup floor f- fabs " );
        }
        else if( TOK_INT_TYPE == *type )
        {
            fprintf( __cur_file, "s>f f/ fdup floor f- fabs " );
            *type = TOK_REAL_TYPE;
        }
        else
        {
            return( CGEN_SEM_ERROR );
        }
    }
    else
    {
        return( CGEN_SEM_ERROR );
    }

    //fprintf( __cur_file, "\n" );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_stdout( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( 1 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    fprintf( __cur_file, "%s cr ", __get_forth_command( &( n->tok ), *type ) );
    return( CGEN_NO_ERROR );

}

void __init_vars( Node *n )
{
    Link *cur_link;

    if( NULL == n )
    {
        return;
    }

    cur_link = n->first_child;
    while( NULL != cur_link )
    {
        __init_vars( cur_link->tree_next );
        cur_link = cur_link->next;
    }

    if( TOK_RESERVED_WORD == n->tok.tok_class )
    {
        switch( n->tok.stmt )
        {
            case TOK_REAL:
                fprintf( __cur_file, "0.e " );
                break;

            case TOK_STRING:
                fprintf( __cur_file, "0 " );

            case TOK_INT:
            case TOK_BOOL:
                fprintf( __cur_file, "0 " );
                break;
        }
    }

}

cgen_error_t8 __gen_code_let( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( 1 > n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    __in_let = TRUE;
    __init_vars( n->first_child->tree_next );
    fprintf( __cur_file, "{ " );

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    fprintf( __cur_file, "} " );
    __in_let = FALSE;
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_var_type( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( !__in_let )
    {
        return( CGEN_SEM_ERROR );
    }

    if( 0 < n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    switch( n->tok.stmt )
    {
        case TOK_INT:
            *type = TOK_INT_TYPE;
            break;

        case TOK_BOOL:
            *type = TOK_BOOL_TYPE;
            break;

        case TOK_STRING:
            *type = TOK_STRING_TYPE;
            break;

        case TOK_REAL:
            *type = TOK_REAL_TYPE;
            break;

        default:
            return( CGEN_SEM_ERROR );
    }

    fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_id( Node *n, type_class_t8 *type )
{
    struct token_type *tok;
    cgen_error_t8 err_code;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( n->num_children > 0 )
    {
        if( n->num_children > 1 )
        {
            return( CGEN_SEM_ERROR );
        }

        err_code = __walk_thru_nodes( n->first_child->tree_next, type );
        if( CGEN_NO_ERROR != err_code )
        {
            return( err_code );
        }
    }

    tok = (struct token_type *)( n->tok.table_hndl );
    if( !__in_let )
    {
        if( TOK_NO_TYPE == tok->id.id_type )
        {
            return( CGEN_SEM_ERROR );
        }

        *type = tok->id.id_type;
        fprintf( __cur_file, "%s ", tok->id.in_str );
        return( CGEN_NO_ERROR );
    }

    tok->id.id_type = *type;
    fprintf( __cur_file, "%s ", tok->id.in_str );
    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_assn( Node *n, type_class_t8 *type )
{
    cgen_error_t8 err_code;
    struct token_type *tok;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    if( n->num_children != 2 )
    {
        return( CGEN_SEM_ERROR );
    }

    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    tok = (struct token_type *)( n->first_child->next->tree_next->tok.table_hndl );
    if( tok->id.id_type != *type )
    {
        switch( *type )
        {
        case TOK_INT_TYPE:
            if( TOK_REAL_TYPE != tok->id.id_type )
            {
                return( CGEN_SEM_ERROR );
            }
            fprintf( __cur_file, "s>f " );
            break;

        case TOK_REAL_TYPE:
            if( TOK_INT_TYPE != tok->id.id_type )
            {
                return( CGEN_SEM_ERROR );
            }
            fprintf( __cur_file, "f>s " );
            break;

        case TOK_STRING_TYPE:
        case TOK_BOOL_TYPE:
        default:
            return( CGEN_SEM_ERROR );
        }
    }

    fprintf( __cur_file, "to " );

    return( __gen_code_id( n->first_child->next->tree_next, type ) );

}

cgen_error_t8 __walk_thru_nodes( Node *n, type_class_t8 *type )
{
    uint num_constants;
    cgen_error_t8 err_code;
    Link *cur_link;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    num_constants = 0;

    cur_link = n->first_child;
    while( NULL != cur_link )
    {
        switch( cur_link->tree_next->tok.tok_class )
        {
            case TOK_ASSN_CLASS:
                err_code = __gen_code_assn( cur_link->tree_next, type );
                break;

            case TOK_BINARY_OPP:
                switch( cur_link->tree_next->tok.binop )
                {
                    case TOK_EXP_OPP:
                        err_code = __gen_code_exp( cur_link->tree_next, type );
                        break;

                    case TOK_MOD_OPP:
                        err_code = __gen_code_mod( cur_link->tree_next, type );
                        break;

                    case TOK_ADD_OPP:
                    case TOK_SUB_OPP:
                    case TOK_MUL_OPP:
                    case TOK_DIV_OPP:
                    case TOK_AND_OPP:
                    case TOK_OR_OPP:
                    case TOK_EQ_OPP:
                    case TOK_LE_OPP:
                    case TOK_GE_OPP:
                    case TOK_LT_OPP:
                    case TOK_GT_OPP:
                    case TOK_NE_OPP:
                        err_code = __gen_code_binop( cur_link->tree_next, type );
                        break;

                    default:
                        return( CGEN_SEM_ERROR );
                }
                break;

            case TOK_UNARY_OPP:
                err_code = __gen_code_unop( cur_link->tree_next, type );
                break;

            case TOK_LITERAL:
                ++num_constants;
                err_code = __gen_code_literal( cur_link->tree_next, type );
                break;

            case TOK_IDENT:
                ++num_constants;
                err_code = __gen_code_id( cur_link->tree_next, type );
                break;

            case TOK_RESERVED_WORD:
                switch( cur_link->tree_next->tok.stmt )
                {
                    case TOK_WHILE:
                        err_code = __gen_code_while( cur_link->tree_next, type );
                        break;

                    case TOK_IF:
                        err_code = __gen_code_if( cur_link->tree_next, type );
                        break;

                    case TOK_LET:
                        err_code = __gen_code_let( cur_link->tree_next, type );
                        break;

                    case TOK_INT:
                    case TOK_BOOL:
                    case TOK_REAL:
                    case TOK_STRING:
                        err_code = __gen_code_var_type( cur_link->tree_next, type );
                        break;

                    case TOK_STDOUT:
                        err_code = __gen_code_stdout( cur_link->tree_next, type );
                        break;

                    default:
                        return( CGEN_SEM_ERROR );
                }
                break;

            case TOK_LIST_TYPE:
                num_constants = 0;
                err_code = CGEN_NO_ERROR;
                break;

            case TOK_NO_CLASS:
                err_code = __walk_thru_nodes( cur_link->tree_next, type );
                break;

            default:
                return( CGEN_SEM_ERROR );
        }

        if( 1 < num_constants )
        {
            return( CGEN_SEM_ERROR );
        }

        if( CGEN_NO_ERROR!= err_code )
        {
            return( err_code );
        }

        cur_link = cur_link->next;
    }

    return( CGEN_NO_ERROR );

}

cgen_error_t8 __gen_code_start( Node *n )
{
    cgen_error_t8 err_code;
    type_class_t8 type;

    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    type = TOK_NUM_TYPES;

    fprintf( __cur_file, ": main " );
    err_code = __walk_thru_nodes( n, &type );
    fprintf( __cur_file, "; \n\nmain\n" );

    return( err_code );

}   /* __gen_code() */

cgen_error_t8 gen_code
(
    char *input_filename
)
{
    cgen_error_t8 err_code;
    char *cur_in;
    char *cur_out;
    char output_filename[ FNAME_MAX_LENGTH ];

    if( NULL == input_filename )
    {
        return( CGEN_NULL_REF  );
    }

    cur_in = input_filename;
    cur_out = output_filename;
    while( ( '\0' != *cur_in ) && ( '.' != *cur_in ) )
    {
        *(cur_out++) = *(cur_in++);
    }

    *cur_out = '\0';
    strcat( output_filename, FORTH_EXTENSION );

    __cur_file = fopen( output_filename, "w" );
    if( NULL == __cur_file )
    {
        return( CGEN_OPEN_ERROR );
    }

    err_code = __gen_code_start( get_parse_tree()->top );
    
    fclose( __cur_file );

#ifndef __CGEN_DEBUG
    if( CGEN_NO_ERROR != err_code )
    {
        remove( output_filename );
    }
#endif

    return( err_code );

}   /* gen_code() */

char *__get_forth_command
(
    Token *tok,
    type_class_t8 tok_type
)
{
    static char gforth_command[ MAX_COMMAND_LENGTH ];
    char prefix;
    boolean exp_seen;
    uint i;

    switch( tok_type )
    {
        case TOK_INT_TYPE:
            prefix = ' ';
            break;

        case TOK_REAL_TYPE:
            prefix = 'f';
            break;
        case TOK_BOOL_TYPE:
            prefix = ' ';
            break;

        case TOK_STRING_TYPE:
            prefix = 's';
            break;

        default:
            fprintf( stderr, "Token type prefix not valid: %i\n", tok_type );
            return( NULL );
    }

    switch( tok->tok_class )
    {
        case TOK_BINARY_OPP:
            switch( tok->binop )
            {
                case TOK_ADD_OPP:
                    sprintf( gforth_command, "%c+ ", prefix );
                    break;

                case TOK_SUB_OPP:
                    sprintf( gforth_command, "%c- ", prefix );
                    break;

                case TOK_MUL_OPP:
                    sprintf( gforth_command, "%c* ", prefix );
                    break;

                case TOK_DIV_OPP:
                    sprintf( gforth_command, "%c/ ", prefix );
                    break;

                case TOK_EXP_OPP:
                case TOK_MOD_OPP:
                    return( NULL );

                case TOK_AND_OPP:
                    sprintf( gforth_command, "and " );
                    break;

                case TOK_OR_OPP:
                    sprintf( gforth_command, "or " );
                    break;

                case TOK_EQ_OPP:
                    sprintf( gforth_command, "%c= ", prefix );
                    break;

                 case TOK_LE_OPP:
                    sprintf( gforth_command, "%c<= ", prefix );
                    break;

                case TOK_GE_OPP:
                    sprintf( gforth_command, "%c>= ", prefix );
                    break;

                case TOK_LT_OPP:
                    sprintf( gforth_command, "%c< ", prefix );
                    break;

                case TOK_GT_OPP:
                    sprintf( gforth_command, "%c> ", prefix );
                    break;

                case TOK_NE_OPP:
                    sprintf( gforth_command, "%c<> ", prefix );
                    break;

                default:
                    fprintf( stderr, "Invalid binop code: %i\n", tok->binop );
                    return( NULL );
            }
            break;

        case TOK_UNARY_OPP:
            switch( tok->unop )
            {
                case TOK_SIN_OPP:
                    sprintf( gforth_command, "fsin " );
                    break;

                case TOK_COS_OPP:
                    sprintf( gforth_command, "fcos " );
                    break;

                case TOK_TAN_OPP:
                    sprintf( gforth_command, "ftan " );
                    break;

                case TOK_NOT_OPP:
                    sprintf( gforth_command, "invert" );
                    break;

                case TOK_POS_OPP:
                    return( NULL );

                case TOK_NEG_OPP:
                    sprintf( gforth_command, "%cnegate ", prefix );
                    break;

                default:
                    fprintf( stderr, "Invalid unop code: %i\n", tok->unop );
                    return( NULL );
            }
            break;

        case TOK_LITERAL:
            switch( tok->type )
            {
                case TOK_INT_TYPE:
                case TOK_REAL_TYPE:
                case TOK_BOOL_TYPE:
                    sprintf( gforth_command, "%s", tok->literal_str );
                    break;

                case TOK_STRING_TYPE:
                    sprintf( gforth_command, "s\" %s\"", tok->literal_str );
                    break;

                default:
                    fprintf( stderr, "Invalid literal type: %i\n", tok->type );
                    return( NULL );
            }

            if( TOK_REAL_TYPE == tok->type )
            {
                exp_seen = FALSE;
                for( i = 0; i < strlen( gforth_command ); ++i )
                {
                    if( ( 'e' == gforth_command[ i ] ) || ( 'E' == gforth_command[ i ] ) )
                    {
                        exp_seen = TRUE;
                        break;
                    }
                }

                if( !exp_seen )
                {
                    strcat( gforth_command, "e" );
                }
            }
            break;

        case TOK_RESERVED_WORD:
            switch( tok->stmt )
            {
                case TOK_WHILE:
                    sprintf( gforth_command, "while" );
                    break;

                case TOK_IF:
                    sprintf( gforth_command, "if" );
                    break;

                case TOK_BOOL:
                    sprintf( gforth_command, "\0" );
                    break;

                case TOK_INT:
                    sprintf( gforth_command, "\0" );
                    break;

                case TOK_STRING:
                    sprintf( gforth_command, "\0" );
                    break;

                case TOK_REAL:
                    sprintf( gforth_command, "F:" );
                    break;

                case TOK_STDOUT:
                    switch( tok_type )
                    {
                        case TOK_INT_TYPE:
                            sprintf( gforth_command, "dup . " );
                            break;

                        case TOK_REAL_TYPE:
                            sprintf( gforth_command, "fdup f. " );
                            break;

                        case TOK_BOOL_TYPE:
                            sprintf( gforth_command, "dup . " );
                            break;

                        default:
                            sprintf( gforth_command, "2dup type " );
                            break;
                    }
                    break;

                case TOK_LET:
                    return( NULL );

                default:
                    fprintf( stderr, "Invalid reserved word: %i\n", tok->stmt );
                    return( NULL );
            }
            break;

        case TOK_IDENT:
            return( NULL );

        case TOK_ASSN_CLASS:
            sprintf( gforth_command, "to" );
            break;

        case TOK_LIST_TYPE:
            return( NULL );

        default:
            return( NULL );
    }

    return( gforth_command );
}
