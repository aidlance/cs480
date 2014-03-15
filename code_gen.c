/**************************************************
*
*   MODULE NAME:
*       code_gen.c
*
*   DESCRIPTION:
*       Implementation for the code generator.
*       This generated Gforth code.
*
**************************************************/

/*-------------------------------------------------
                 GENERAL INCLUDES
-------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_gen.h"
#include "parser.h"
#include "symbol_table.h"
#include "tokens.h"
#include "types.h"

/*-------------------------------------------------
                   LITERAL CONSTANTS
-------------------------------------------------*/

#define FORTH_EXTENSION ".fs"       /* Gforth file extension    */
#define FNAME_MAX_LENGTH 255        /* max filename length      */
#define MAX_COMMAND_LENGTH 255      /* max command length       */

//#define __CGEN_DEBUG

/*-------------------------------------------------
                 GLOBAL VARIABLES
-------------------------------------------------*/

static boolean __in_let      = FALSE;       /* are we in a let statement?       */
static boolean __str_cat_set = FALSE;       /* are we concatenating a string?   */
static FILE *__cur_file      = NULL;        /* pointer to current out file      */

/*-------------------------------------------------
                FUNCTION PROTOTYPES
-------------------------------------------------*/

cgen_error_t8 __walk_thru_nodes
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

char *__get_forth_command
(
    Token          *tok,                /* token to process         */
    type_class_t8   tok_type            /* token's type (e.x. int)  */
);

cgen_error_t8 __gen_code_binop
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_assn
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_exp
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_id
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_if
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_if_branches
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_let
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_literal
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_mod
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_stdout
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_unop
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_var_type
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

cgen_error_t8 __gen_code_while
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
);

void __init_vars
(
    Node           *n                   /* current node             */
);

cgen_error_t8 __gen_code_start
(
    Node           *n                   /* current node             */
);

/*-------------------------------------------------
                    PROCEDURES
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       __gen_code_binop - "Generate Binary
*                           Operator Code"
*
*   DESCRIPTION:
*       Generates the code for most binary
*       operators in this language (exceptions,
*       of course, being mod and exp).
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there
*         were no errors
*       - Returns CGEN_SEM_ERROR if there
*         was a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_binop
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    boolean         is_str_cat;         /* are we concatenating a string    */
    cgen_error_t8   err_code;           /* error code                       */
    type_class_t8   old_type;           /* type of operator's left child    */

    /*---------------------------------
    Make user we have a valid node
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Confirm that we only have two
    children
    ---------------------------------*/
    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Set this--apparently it's used
    later
    ---------------------------------*/
    is_str_cat = __str_cat_set;

    /*---------------------------------
    Generate code for the left child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Set the old_type variable
    ---------------------------------*/
    old_type = *type;

    /*---------------------------------
    We need to do something completely
    different for string concatenation,
    and the method I'm using requires
    some pre- and post-processing of
    the node's children
    ---------------------------------*/
    if( TOK_STRING_TYPE == old_type )
    {
        /*-----------------------------
        If we're already concatenating
        a string, then we concatenate
        the child node immediately.

        Before I did it this way, the
        string concatenation wouldn't
        work properly if you were
        trying to concatenate more than
        one string (i.e."lets" + "have"
        + "fun" would turn into
        "havefunhavefun" rather than
        "letshavefun"). Performing
        an immediate concatenation
        partially alleviates the
        issue.
        -----------------------------*/
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

    /*---------------------------------
    Generate code for the right child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that types are matching.
    If an int is paired with a float,
    then the int is always cast to
    a float, a float operation is
    always going to be performed,
    and the result will always remain
    a float.

    For most other operations, mixed
    types (e.g. int and bool, float
    and string, etc.) are not allowed,
    so we throw a semantics error.

    Also, bear in mind that we are
    using a stack for the list of
    nodes, so for an operation
    like "3/2," the "2" would be the
    left child and the "3" would be the
    right child. This can be slightly
    problematic when we try to do
    anything with int->float
    conversions since Forth is a
    stack-based language. That's
    why you'll see some "swap" commands
    below.
    ---------------------------------*/
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

        /*-----------------------------
        I'm not sure why this works,
        but this is the only way I
        was able to get concatenation
        to work. If we're still
        concatenating strings, then we
        set the global flag to false.
        -----------------------------*/
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

    /*---------------------------------
    Make sure that all boolean
    operations are typed correctly. I
    had a bug where this wasn't
    happening, so I just explicitly
    set the expression type
    ---------------------------------*/
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

    /*---------------------------------
    If we're at the first string,
    then we make sure that we aren't
    concatenating anymore.
    ---------------------------------*/
    if( !is_str_cat  )
    {
        __str_cat_set = FALSE;
    }

    /*---------------------------------
    And return happily home
    ---------------------------------*/
    return( CGEN_NO_ERROR );

}   /* __gen_code_binop() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_unop - "Generate Unary
*                          Operator Code"
*
*   DESCRIPTION:
*       Generates code for a unary operator
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_unop
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we only have one child
    ---------------------------------*/
    if( 1 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for our child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure we are properly typed
    ---------------------------------*/
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

    /*---------------------------------
    And return
    ---------------------------------*/
    //fprintf( __cur_file, "\n" );
    return( CGEN_NO_ERROR );

}   /* __gen_code_unop() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_literal - "Generate Literal
*                             Code"
*
*   DESCRIPTION:
*       Generates code for the literals
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_literal
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    char       *str;            /* string for something     */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we have no children
    ---------------------------------*/
    if( 0 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Write to the file and return
    ---------------------------------*/
    *type = n->tok.type;
    str = __get_forth_command( &( n->tok ), *type );
    fprintf( __cur_file, "%s ", NULL == str ? "" : str );
    return( CGEN_NO_ERROR );

}   /* __gen_code_literal() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_exp - "Generate Exponential
*                         Operator Code"
*
*   DESCRIPTION:
*       Generates code for the exponent
*       operator
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
*   NOTES:
*       Regardless of the children, this converts
*       everything to floats and uses the
*       Gforth float exponentiation (f**) to
*       get things done. If we're dealing with
*       two ints, then the float result is then
*       converted back to an integer.
*
**************************************************/
cgen_error_t8 __gen_code_exp
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;       /* error code           */
    type_class_t8   old_type;       /* left child's type    */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we have only 2 children
    ---------------------------------*/
    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for the left child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    old_type = *type;

    /*---------------------------------
    Generate code for the right child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that we are properly
    typed
    ---------------------------------*/
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

    /*---------------------------------
    And return
    ---------------------------------*/
    //fprintf( __cur_file, "\n" );
    return( CGEN_NO_ERROR );

}   /* __gen_code_exp() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_if_branches - "Generate If
*                                 Branch Code"
*
*   DESCRIPTION:
*       Generates code for if branches
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_if_branches
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Make sure that we have the right
    number of children
    ---------------------------------*/
    if( ( 1 != n->num_children ) && ( 2 != n->num_children ) )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for he first branch
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Generate code for the else branch
    ---------------------------------*/
    if( 2 == n->num_children )
    {
        fprintf( __cur_file, "else " );
        err_code = __walk_thru_nodes( n->last_child->tree_next, type );
        if( CGEN_NO_ERROR != err_code )
        {
            return( err_code );
        }
    }

    /*---------------------------------
    And return
    ---------------------------------*/
    return( CGEN_NO_ERROR );

}   /* __gen_code_if_branches() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_if - "Generate if
*                        Statement Code"
*
*   DESCRIPTION:
*       Generates code for the if
*       statement
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_if
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we only have 2 children
    ---------------------------------*/
    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for the if condition
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    If the type doesn't evaluate to
    bool, throw a semantics error
    ---------------------------------*/
    if( TOK_BOOL_TYPE != *type )
    {
        return( CGEN_SEM_ERROR );
    }

    fprintf( __cur_file, "if " );

    /*---------------------------------
    Generate code for the if branches
    ---------------------------------*/
    err_code = __gen_code_if_branches( n->last_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Write to file and return
    ---------------------------------*/
    fprintf( __cur_file, "then " );
    return( CGEN_NO_ERROR );

}   /* __gen_code_if() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_while - "Generate While
*                           Statement Code"
*
*   DESCRIPTION:
*       Generates code for the while
*       statement
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_while
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we only have 2 children
    ---------------------------------*/
    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for our looping
    flag condition (when do we stop?)
    ---------------------------------*/
    fprintf( __cur_file, "begin " );
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that it evaluates to bool
    ---------------------------------*/
    if( TOK_BOOL_TYPE != *type )
    {
        return( CGEN_SEM_ERROR );
    }

    fprintf( __cur_file, "while " );

    /*---------------------------------
    Generate code for the loop body
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Write to file and return
    ---------------------------------*/
    fprintf( __cur_file, "repeat " );
    return( CGEN_NO_ERROR );

}   /* __gen_code_while() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_mod - "Generate Modulus
*                          Operator Code"
*
*   DESCRIPTION:
*       Generates code for the modulus
*       operator
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_mod
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */
    type_class_t8   old_type;   /* type of left child   */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we only have 2 children
    ---------------------------------*/
    if( 2 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for the left child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    old_type = *type;

    /*---------------------------------
    Generate code for the right child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->next->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that we are properly
    typed

    NOTE:
    I have implemented float modulus
    to be (a/b)-floor(a/b)
    ---------------------------------*/
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

    /*---------------------------------
    And return
    ---------------------------------*/
    //fprintf( __cur_file, "\n" );
    return( CGEN_NO_ERROR );

}   /* __gen_code_mod() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_stdout - "Generate Print
*                            Statement Code"
*
*   DESCRIPTION:
*       Generates code for the print
*       statement
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_stdout
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Verify that we only have one child
    ---------------------------------*/
    if( 1 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for our child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Write to file and return
    ---------------------------------*/
    fprintf( __cur_file, "%s cr ", __get_forth_command( &( n->tok ), *type ) );
    return( CGEN_NO_ERROR );

}   /* __gen_code_stdout() */


/**************************************************
*
*   FUNCTION:
*       __init_vars - "Initialize Variables"
*
*   DESCRIPTION:
*       Initializes the stack for variable
*       declarations. Values need to get pushed
*       to the stack so that we won't get a
*       stack underflow.
*
**************************************************/
void __init_vars
(
    Node           *n                   /* current node             */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Link       *cur_link;       /* pointer to current link  */

    /*---------------------------------
    Check for null reference
    (base case)
    ---------------------------------*/
    if( NULL == n )
    {
        return;
    }

    /*---------------------------------
    Loop through all children and
    recursively call this function
    ---------------------------------*/
    cur_link = n->first_child;
    while( NULL != cur_link )
    {
        __init_vars( cur_link->tree_next );
        cur_link = cur_link->next;
    }

    /*---------------------------------
    Write values to the stack depending
    on the variable's type
    ---------------------------------*/
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

}   /* __init_vars() */



/**************************************************
*
*   FUNCTION:
*       __gen_code_let - "Generate Let
*                         Statement Code"
*
*   DESCRIPTION:
*       Generates code for the let
*       statement (for variable declarations)
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_let
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;       /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Make sure we have only one child
    ---------------------------------*/
    if( 1 != n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Set the __in_let flag and write
    some stuff to the stack--all
    variables are initialized to zero.

    Things need to be written to the
    stack so that when we actually
    declare the variables in Gforth,
    we won't get a  stack underflow
    error
    ---------------------------------*/
    __in_let = TRUE;
    __init_vars( n->first_child->tree_next );
    fprintf( __cur_file, "{ " );

    /*---------------------------------
    Generate the code for our children
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Reset the flag and return
    ---------------------------------*/
    fprintf( __cur_file, "} " );
    __in_let = FALSE;
    return( CGEN_NO_ERROR );

}   /* __gen_code_let() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_var_type - "Generate Variable
*                              Type Code"
*
*   DESCRIPTION:
*       Generates code for a variable's type
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_var_type
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;       /* error code           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    If we aren't in a let statement,
    throw a semantics error.

    NOTE: This should already be
          caught by the parser. This
          has been added as a "just in
          case" precaution
    ---------------------------------*/
    if( !__in_let )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Make sure that we don't have any
    children
    ---------------------------------*/
    if( 0 < n->num_children )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Set the statement type based on
    what keyword was used in the
    input file
    ---------------------------------*/
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

    /*---------------------------------
    Write to file and return
    ---------------------------------*/
    fprintf( __cur_file, "%s ", __get_forth_command( &( n->tok ), *type ) );
    return( CGEN_NO_ERROR );

}   /* __gen_code_var_type() */



/**************************************************
*
*   FUNCTION:
*       __gen_code_id - "Generate Identifier Code"
*
*   DESCRIPTION:
*       Generates code for an identifier
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_id
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    struct token_type  *tok;        /* token in symbol table    */
    cgen_error_t8       err_code;   /* error code               */

    /*---------------------------------
    Make sure our reference is valid
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    If we have more than one child,
    then we're in a let statement,
    so process our children before
    continuing to process ourselves
    ---------------------------------*/
    if( n->num_children > 0 )
    {
        /*-----------------------------
        Or throw an error
        -----------------------------*/
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

    /*---------------------------------
    Grab the token data from the
    symbol table.
    ---------------------------------*/
    tok = (struct token_type *)( n->tok.table_hndl );

    /*---------------------------------
    If we aren't in a let statement
    and the variable is undeclared
    (i.e. has no type associated
    with it), we throw an error.
    Otherwise, we set the type of
    the variable and return.
    ---------------------------------*/
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

    /*---------------------------------
    Otherwise, the identifier is good
    to go, so write it to the file
    and return.
    ---------------------------------*/
    tok->id.id_type = *type;
    fprintf( __cur_file, "%s ", tok->id.in_str );
    return( CGEN_NO_ERROR );

}   /* __gen_code_id() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_assn - "Generate Assignment
*                          Statement Code"
*
*   DESCRIPTION:
*       Generates code for the assignment
*       statement
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were no
*         errors
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error
*
**************************************************/
cgen_error_t8 __gen_code_assn
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8       err_code;   /* error code               */
    struct token_type  *tok;        /* token in symbol table    */

    /*---------------------------------
    Make sure that our node is valid
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Make sure that we only have two
    children
    ---------------------------------*/
    if( n->num_children != 2 )
    {
        return( CGEN_SEM_ERROR );
    }

    /*---------------------------------
    Generate code for our left child
    ---------------------------------*/
    err_code = __walk_thru_nodes( n->first_child->tree_next, type );
    if( CGEN_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    The left child should be the
    value we're assigning to the
    identifier, so we need to grab
    the identifier from the symbol
    table. The types need to be the
    the same (unless we're dealing
    with floats and ints--in that
    case, we convert an int to a float
    if we have an int value and a
    float variable or vice-versa).
    If they aren't, we throw a
    semantics error.
    ---------------------------------*/
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

    /*---------------------------------
    I'm using Gforth local variables
    because they're easier for me to
    deal with. It's strongly
    discouraged, but it works.
    "To" is Gforth's version of our
    ":="
    ---------------------------------*/
    fprintf( __cur_file, "to " );

    /*---------------------------------
    Generate code for the right child
    (the identifier)
    ---------------------------------*/
    return( __gen_code_id( n->first_child->next->tree_next, type ) );

}   /* __gen_code_assn() */


/**************************************************
*
*   FUNCTION:
*       __walk_thru_nodes - "Walk Through Nodes"
*
*   DESCRIPTION:
*       Recursively walks through the nodes
*       in the parse tree and determines which
*       function to call based on node's
*       token class.
*
*   RETURNS:
*       This returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there are
*         no semantic errors in the parse tree
*       - Returns CGEN_SEM_ERROR if there was
*         a semantics error.
*
**************************************************/
cgen_error_t8 __walk_thru_nodes
(
    Node           *n,                  /* current node             */
    type_class_t8  *type                /* previous expression type */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint            num_constants;      /* number of constants in   */
                                        /*  current tree depth      */
    cgen_error_t8   err_code;           /* errors                   */
    Link           *cur_link;           /* pointer to current link  */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Initialize this guy--we haven't
    seen any literals or identifiers
    yet
    ---------------------------------*/
    num_constants = 0;

    /*---------------------------------
    Loop through this node's list
    of children making sure to
    increment num_constants every time
    we see an identifier or literal.
    If there are more than two of
    these guys, we throw a semantics
    error.

    NOTE:
    The way we have this set up,
    [a b [c]] is not accepted, and
    [a [b] c] is. If there is a list
    separating identifiers and
    literals, then we can accept.
    ---------------------------------*/
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

        /*---------------------------=-
        Confirm that there is only one
        identifier or constant at this
        depth
        -----------------------------*/
        if( 1 < num_constants )
        {
            return( CGEN_SEM_ERROR );
        }

        /*-----------------------------
        Confirm that there were no
        errors while processing the
        node
        -----------------------------*/
        if( CGEN_NO_ERROR != err_code )
        {
            return( err_code );
        }

        cur_link = cur_link->next;
    }

    /*---------------------------------
    If we got this far, then there
    have been no errors.
    ---------------------------------*/
    return( CGEN_NO_ERROR );

}   /* __walk_thru_nodes() */


/**************************************************
*
*   FUNCTION:
*       __get_forth_command - "Get Forth Command"
*
*   DESCRIPTION:
*       Takes a token and produces Gforth code
*       for the token.
*
*   RETURNS:
*       This returns a string if the token is
*       valid (i.e. if it can be translated to
*       Gforth), and NULL if the token cannot
*       be translated.
*
**************************************************/
char *__get_forth_command
(
    Token          *tok,                /* token to process         */
    type_class_t8   tok_type            /* token's type (e.x. int)  */
)
{
    /*---------------------------------
    Local variable constants
    ---------------------------------*/
    static char gforth_command[ MAX_COMMAND_LENGTH ];   /* buffer for the current command */

    /*---------------------------------
    Local variables
    ---------------------------------*/
    char    prefix;         /* Gforth command stack prefix  */
    boolean exp_seen;       /* was there an "e" in a float? */
    uint    i;              /* for-loop iterator            */

    /*---------------------------------
    Determine the command stack prefix
    based on the provided token type.
    ---------------------------------*/
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

    /*---------------------------------
    Statically translate the command
    taking into account the stack
    prefix.

    These are pretty self-explanatory
    if you know Gforth.
    ---------------------------------*/
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

            /*-------------------------
            This checks if the literal
            was a float. If the float
            value doesn't already have
            an "e" (i.e. like 2.0e),
            then we append it to the
            end of the Gforth command.
            -------------------------*/
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

    /*---------------------------------
    Return the pointer to our command
    buffer
    ---------------------------------*/
    return( gforth_command );

}   /* __get_forth_command() */


/**************************************************
*
*   FUNCTION:
*       __gen_code_start - "Generate Code Start"
*
*   DESCRIPTION:
*       Starts the recursive code generation
*       process
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were
*         no errors
*       - Returns CGEN_SEM_ERROR if there were
*         semantic errors
*
**************************************************/
cgen_error_t8 __gen_code_start
(
    Node           *n                   /* current node             */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code           */
    type_class_t8   type;       /* type code            */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return( CGEN_NO_ERROR );
    }

    /*---------------------------------
    Local variable initializations
    ---------------------------------*/
    type = TOK_NUM_TYPES;

    /*---------------------------------
    Generate program's code
    ---------------------------------*/
    fprintf( __cur_file, ": main " );
    err_code = __walk_thru_nodes( n, &type );
    fprintf( __cur_file, "; \n\nmain\n" );

    /*---------------------------------
    And return
    ---------------------------------*/
    return( err_code );

}   /* __gen_code_start() */


/**************************************************
*
*   FUNCTION:
*       gen_code - "Generate Code"
*
*   DESCRIPTION:
*       Generates code for a program
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns CGEN_NO_ERROR if there were
*         no errors
*       - Returns CGEN_NULL_REF if the filename
*         is invalid
*       - Returns CGEN_OPEN_ERROR if there were
*         errors while opening the file
*       - Returns CGEN_SEM_ERROR if there were
*         semantic errors in the program
*
**************************************************/
cgen_error_t8 gen_code
(
    char *input_filename        /* input file's filename    */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    cgen_error_t8   err_code;   /* error code                       */
    char           *cur_in;     /* current char in input_filename   */
    char           *cur_out;    /* current char in output_filename  */
    char            output_filename[ FNAME_MAX_LENGTH ];
                                /* output file's filename           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == input_filename )
    {
        return( CGEN_NULL_REF  );
    }

    /*---------------------------------
    Copy the filename over to the
    output file's filename until
    we reach a null-terminator or
    the file's extension
    ---------------------------------*/
    cur_in = input_filename;
    cur_out = output_filename;
    while( ( '\0' != *cur_in ) && ( '.' != *cur_in ) )
    {
        *(cur_out++) = *(cur_in++);
    }

    /*---------------------------------
    Add the Gforth file extension
    ---------------------------------*/
    *cur_out = '\0';
    strcat( output_filename, FORTH_EXTENSION );

    /*---------------------------------
    Attempt to create and open the file
    ---------------------------------*/
    __cur_file = fopen( output_filename, "w" );
    if( NULL == __cur_file )
    {
        return( CGEN_OPEN_ERROR );
    }

    /*---------------------------------
    Generate the code for the file
    ---------------------------------*/
    err_code = __gen_code_start( get_parse_tree()->top );

    /*---------------------------------
    Close the file
    ---------------------------------*/
    fclose( __cur_file );

#ifndef __CGEN_DEBUG
    /*---------------------------------
    Delete the file if we ran into
    some errors
    ---------------------------------*/
    if( CGEN_NO_ERROR != err_code )
    {
        remove( output_filename );
    }
#endif

    /*---------------------------------
    And return
    ---------------------------------*/
    return( err_code );

}   /* gen_code() */
