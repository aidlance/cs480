/**************************************************
*
*  MODULE NAME:
*      parser.c
*
*  DESCRIPTION:
*      Parser implementation
*
**************************************************/

/*-------------------------------------------------
                  PROJECT INCLUDES
-------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"
#include "symbol_table.h"
#include "tokens.h"
#include "types.h"

/*-------------------------------------------------
                  LITERAL CONSTANTS
-------------------------------------------------*/

#define FORTH_EXTENSION ".fs"
#define MAX_FILENAME_LENGTH 255
//#define __DEBUG_PRINTS_PARSER

typedef uint8 parser_state_t8;
enum
{
    PARSER_INIT,            /* parser init state        */
    PARSER_LOAD_FILE,       /* file loading state       */
    PARSER_PARSE_FILE,      /* file parsing state       */
    PARSER_GEN_CODE,        /* code generation state    */
    PARSER_CLOSE_FILE       /* file closing state       */
};

/*-------------------------------------------------
                   GLOBAL VARIABLES
-------------------------------------------------*/

static parser_state_t8 __parser_state = PARSER_INIT;

static Token   __cur_token;     /* current token                        */
static Token   __next_token;    /* next token to be read                */
static Token   __prev_token;    /* previously read token--not needed    */
static boolean __end_reached;   /* have we reached the end of the file? */
static Tree   *__parse_tree;    /* pointer to the parse tree            */

static FILE   *__cur_out_file = NULL;  /* pointer to output file               */

/*-------------------------------------------------
                FUNCTION PROTOTYPES
-------------------------------------------------*/

parser_error_t8 __add_link
(
    Node       *parent_node,    /* pointer to parent node   */
    Node       *child_node      /* pointer to child node    */
);

parser_error_t8 __add_node
(
    Node       *parent,         /* pointer to parent node   */
    Token      *tok,            /* new node's token data    */
    Node      **child           /* pointer to new node      */
);

parser_error_t8 __Binop
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Constant
(
    Node       *parent          /* pointer to parent node   */
);

Link *__create_link
(
    void
);

Node *__create_node
(
    void
);

Tree *__create_tree
(
    void
);

parser_error_t8 __Expr
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Expr2
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Exprlist
(
    Node       *parent          /* pointer to parent node   */
);

void __free_parse_tree
(
    void
);

void __free_parse_tree_nodes
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __get_next_token
(
    Token     **tok             /* read token (same as __cur_token  */
);

parser_error_t8 __If
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __If2
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __init_link
(
    Link       *l               /* link to initialize       */
);

parser_error_t8 __init_tree
(
    Tree       *t               /* tree to initialize       */
);

parser_error_t8 __Let
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Name
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Oper
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Oper2
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Oper3
(
    Node       *parent          /* pointer to parent node   */
);

Token *__peek
(
    void
);

parser_error_t8 __Print
(
    Node       *parent          /* pointer to parent node   */
);

void __print_error
(
    parser_error_t8     err_code    /* error code to print  */
);

void __print_tree
(
    Node       *n,              /* current node             */
    uint        depth           /* recursion depth          */
);

parser_error_t8 __S
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __S2
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Stmt
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __T
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Type
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Unop
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __Varlist
(
    Node       *parent          /* pointer to parent node   */
);

parser_error_t8 __While
(
    Node       *parent          /* pointer to parent node   */
);

/*-------------------------------------------------
                      PROCEDURES
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       __add_link - "Add Link"
*
*   DESCRIPTION:
*       Links a parent node to a child node
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns PARSER_NO_ERROR if there
*         were no errors
*       * Returns PARSER_NO_MEMORY if there
*         wasn't enough space to create a
*         new link.
*       * Returns PARSER_NULL_REF if the parent
*         node pointer is NULL
*
**************************************************/
parser_error_t8 __add_link
(
    Node       *parent_node,    /* parent node          */
    Node       *child_node      /* child node           */
)
{
    Link       *new_link;       /* new link             */

    /*---------------------------------
    Check for valid parent and child
    handles
    ---------------------------------*/
    if( ( NULL == parent_node ) || ( NULL == child_node ) )
    {
        return( PARSER_NULL_REF );
    }

    /*---------------------------------
    Create the new link
    ---------------------------------*/
    new_link = __create_link();
    if( NULL == new_link )
    {
        return( PARSER_NO_MEMORY );
    }

    /*---------------------------------
    Link the child to the parent
    ---------------------------------*/
    new_link->tree_next = child_node;
    if( NULL == parent_node->first_child )
    {
        new_link->next           = NULL;
        parent_node->last_child  = new_link;
        parent_node->first_child = new_link;
    }
    else
    {
        new_link->next                = parent_node->last_child->next;
        parent_node->last_child->next = new_link;
        parent_node->last_child       = new_link;
    }

    ++parent_node->num_children;

    return( PARSER_NO_ERROR );

}   /* __add_link() */


/**************************************************
*
*   FUNCTION:
*       __add_node - "Add Node"
*
*   DESCRIPTION:
*       Adds a node to the tree as a child
*       of the supplied parent node.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * Returns PARSER_NO_ERROR if there
*         were no errors
*       * Returns PARSER_NO_MEMORY if there
*         wasn't enough memory to create the
*         new node
*       * Returns PARSER_NULL_REF if the parent
*         node pointer is NULL
*
**************************************************/
parser_error_t8 __add_node
(
    Node       *parent,         /* parent node          */
    Token      *tok,            /* new node's token     */
    Node      **child           /* pointer to new node  */
)
{
    Node       *new_node;       /* new node             */

    if( NULL != child )
    {
        *child = NULL;
    }

    /*---------------------------------
    Confirm that the pointer to the
    parent is valid
    ---------------------------------*/
    if( NULL == parent )
    {
        return( PARSER_NULL_REF );
    }

    /*---------------------------------
    Create a new node
    ---------------------------------*/
    new_node = __create_node();
    if( NULL == new_node )
    {
        return( PARSER_NO_MEMORY );
    }

    /*---------------------------------
    Initialize the node's values
    ---------------------------------*/
    new_node->first_child  = NULL;
    new_node->last_child   = NULL;
    new_node->num_children = 0;

    /*---------------------------------
    If the token pointer is NULL, then
    set the node's class to
    TOK_NO_CLASS. Otherwise, copy over
    the token data.
    ---------------------------------*/
    if( NULL == tok )
    {
        memset( &( new_node->tok ), 0, sizeof( new_node->tok ) );
        new_node->tok.tok_class = TOK_NO_CLASS;
    }
    else
    {
        memcpy( &(new_node->tok), tok, sizeof( *tok ) );
    }

    if( NULL != child )
    {
        *child = new_node;
    }

    /*---------------------------------
    Link the new node to the parent
    ---------------------------------*/
    return( __add_link( parent, new_node ) );

}   /* __add_node() */


/**************************************************
*
*   FUNCTION:
*       __create_link - "Create Link"
*
*   DESCRIPTION:
*       Creates a new link
*
*   RETURNS:
*       Returns a pointer to a new link
*       or NULL if a link couldn't be made.
*
**************************************************/
Link *__create_link
(
    void
)
{
    return( (Link *)malloc( sizeof( Link ) ) );

}   /* __create_link() */


/**************************************************
*
*   FUNCTION:
*       __create_node - "Create Node"
*
*   DESCRIPTION:
*       Creates a new node
*
*   RETURNS:
*       Returns a pointer to a new node
*       or NULL if a node couldn't be made.
*
**************************************************/
Node *__create_node
(
    void
)
{
    return( (Node *)malloc( sizeof( Node ) ) );

}   /* __create_node() */


/**************************************************
*
*   FUNCTION:
*       __create_tree - "Create Tree"
*
*   DESCRIPTION:
*       Creates a new tree
*
*   RETURNS:
*       Returns a pointer to a new tree
*       or NULL if a tree couldn't be made.
*
**************************************************/
Tree *__create_tree
(
    void
)
{
    return( (Tree *)malloc( sizeof( Tree ) ) );

}   /* __create_tree() */


/**************************************************
*
*   FUNCTION:
*       __free_parse_tree_nodes - "Free Parse
*                                  Tree's nodes"
*
*   DESCRIPTION:
*       Frees the nodes of the parse tree
*
**************************************************/
void __free_parse_tree_nodes
(
    Node       *n       /* current node         */
)
{
    Link       *cur;    /* current node link    */
    Link       *prev;   /* previous node link   */

    /*---------------------------------
    Check for NULL node pointer
    (base case)
    ---------------------------------*/
    if( NULL == n )
    {
        return;
    }

    /*---------------------------------
    Iterate through the list of nodes
    and recursively call this function
    to free their children
    ---------------------------------*/
    cur = n->first_child;
    while( NULL != cur )
    {
        prev = cur;
        __free_parse_tree_nodes( cur->tree_next );
        cur = cur->next;
        free( prev );
    }

    /*---------------------------------
    If the current node is a literal
    token, then we need to free the
    string since it was allocated
    using malloc
    ---------------------------------*/
    if( TOK_LITERAL == n->tok.tok_class )
    {
        free( n->tok.literal_str );
    }

    /*---------------------------------
    Free the node
    ---------------------------------*/
    free( n );

}   /* __free_parse_tree() */


/**************************************************
*
*   FUNCTION: __free_parse_tree - "Free parse Tree"
*
*   DESCRIPTION:
*       Free the parse tree
*
**************************************************/
void __free_parse_tree
(
    void
)
{
    /*---------------------------------
    Check if there is a parse tree
    ---------------------------------*/
    if( NULL == __parse_tree )
    {
        return;
    }

    /*---------------------------------
    Free the parse tree nodes, free
    the parse tree, and set to NULL
    ---------------------------------*/
    __free_parse_tree_nodes( __parse_tree->top );
    free( __parse_tree );
    __parse_tree = NULL;

}   /* __free_parse_tree() */


/**************************************************
*
*   FUNCTION:
*       __get_next_token - "Get Next Token"
*
*   DESCRIPTION:
*       Grabs the next token from the lexical
*       scanner
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * PARSER_NO_ERROR is returned if there
*         were no errors
*       * PARSER_PARSE_ERROR is returned if
*         the token from the scanner is invalid
*       * PARSER_FILE_END_REACHED is returned
*         if the file's end has been reached
*
*   NOTES:
*       * tok can be NULL
*
**************************************************/
parser_error_t8 __get_next_token
(
    Token     **tok     /* where read token is stored   */
)
{
    boolean          finished;
    scanner_error_t8 temp;

    /*---------------------------------
    Initialize tok if the pointer is
    valid
    ---------------------------------*/
    if( NULL != tok )
    {
        *tok = NULL;
    }

    /*---------------------------------
    I don't know what this is for
    ---------------------------------*/
    finished = __end_reached;

    /*---------------------------------
    Copy the current token to the
    previous token, and then copy the
    next token to the current token
    ---------------------------------*/
    memcpy( &__prev_token, &__cur_token,  sizeof( __cur_token  ) );
    memcpy( &__cur_token,  &__next_token, sizeof( __next_token ) );

    /*---------------------------------
    Set tok if the pointer is valid
    ---------------------------------*/
    if( NULL != tok )
    {
        *tok = &__cur_token;
    }

    /*---------------------------------
    If we haven't reached the end of
    the file, grab the next token
    from the scanner
    ---------------------------------*/
    if( !__end_reached )
    {
        temp = read_next_token( &__next_token );
        if( SCN_TOKEN_INVALID == temp )
        {
            __parser_state = PARSER_CLOSE_FILE;
            return( PARSER_PARSE_ERROR );
        }
        else if( SCN_FILE_END_REACHED == temp )
        {
            __end_reached  = TRUE;
        }
    }

    if( finished )
    {
        __parser_state = PARSER_CLOSE_FILE;
        return( PARSER_FILE_END_REACHED );
    }

    return( PARSER_NO_ERROR );

}   /* __get_next_token() */


parser_error_t8 __init_link
(
    Link *l
)
{
    if( NULL == l )
    {
        return( PARSER_NULL_REF );
    }

    l->next      = NULL;
    l->tree_next = NULL;

    return( PARSER_NO_ERROR );

}   /* __init_link() */


parser_error_t8 __init_tree
(
    Tree *t
)
{
    if( NULL == t )
    {
        return( PARSER_NULL_REF );
    }

    t->top  = NULL;
    t->size = 0;

    return( PARSER_NO_ERROR );

}   /* __init_tree() */


Token *__peek
(
    void
)
{
    return( &__next_token );

}   /* __peek() */

void __print_error
(
    parser_error_t8 err_code
)
{
    switch( err_code )
    {
        case PARSER_ALREADY_INITIALIZED:
            fprintf( stderr, "Parser already initialized\n" );
            break;
        case PARSER_NO_ERROR:
            fprintf( stderr, "No errors\n" );
            break;
        case PARSER_MISMATCHED_LIST:
            fprintf( stderr, "Mismatched '[' or ']'\n" );
            break;
        case PARSER_PARSE_ERROR:
            fprintf( stderr, "Generic parse error happened\n" );
            break;
        case PARSER_NULL_REF:
            fprintf( stderr, "Internal pointer error\n" );
            break;
        case PARSER_INIT_ERROR:
            fprintf( stderr, "Error while initializing something\n" );
            break;
        case PARSER_LOAD_ERROR:
            fprintf( stderr, "Failed to load a file\n" );
            break;
        case PARSER_FILE_END_REACHED:
            fprintf( stderr, "File end reached unexpectedly\n" );
            break;
        case PARSER_UNEXPECTED_TOKEN:
            fprintf( stderr, "Unexpected token encountered\n" );
            break;
        case PARSER_NO_MEMORY:
            fprintf( stderr, "No memory\n" );
            break;
        default:
            fprintf(stderr, "Unexpected error code\n" );
            break;
    }

}   /* __print_error() */

void __print_tree
(
    Node *n,
    uint depth
)
{
#ifdef __DEBUG_PRINTS_PARSER
    uint i;
    Link *temp;

    if( NULL == n )
    {
        return;
    }

    temp = n->first_child;
    while( NULL != temp )
    {
        __print_tree( temp->tree_next, depth + 1 );
        temp = temp->next;
    }

    if( TOK_NO_CLASS == n->tok.tok_class )
    {
        return;
    }

    for( i = 0; i < depth; ++i )
    {
        fprintf( stdout, "    " );
    }
    print_minimal_token( &( n->tok ) );
#endif

}   /* __print_tree() */

/* S --> [S2 | Oper3 S | Oper3 */
parser_error_t8 __S
(
    Node *parent
)
{
    Node *new_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    if( ( TOK_LIST_TYPE != __peek()->tok_class ) || ( TOK_LIST_BEGIN != __peek()->list ) )
    {
        err_code = __Oper3( parent );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        if( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_END == __peek()->list ) )
        {
            return( PARSER_NO_ERROR );
        }

        err_code = __S( parent );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( PARSER_NO_ERROR );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    return( __S2( new_node ) );

}   /* __S() */


/* S2 --> ] | S] | Expr2] | ]S */
parser_error_t8 __S2
(
    Node *parent
)
{
    Node *new_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    if( ( TOK_LIST_TYPE != __peek()->tok_class ) || ( TOK_LIST_END != __peek()->list ) )
    {
        if( ( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_BEGIN == __peek()->list ) )
         || ( TOK_LITERAL    == __peek()->tok_class )
         || ( TOK_IDENT      == __peek()->tok_class ) )
        {
            err_code = __S( parent );
            if( PARSER_NO_ERROR != err_code )
            {
                return( err_code );
            }

            /*
            err_code = __S();
            if( PARSER_NO_ERROR != err_code )
            {
                return( err_code );
            }
            */
        }
        else
        {
            err_code = __Expr2( parent );
            if( PARSER_NO_ERROR != err_code )
            {
                return( err_code );
            }
        }
    }

    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_MISMATCHED_LIST );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( ( ( TOK_LIST_TYPE  == __peek()->tok_class )
       && ( TOK_LIST_BEGIN == __peek()->list ) )
     || ( TOK_LITERAL == __peek()->tok_class )
     || ( TOK_IDENT   == __peek()->tok_class ) )
    {
        return( __S( parent ) );
    }

    return( PARSER_NO_ERROR );

}   /* __S2() */

/* While --> while Expr Exprlist */
parser_error_t8 __While
(
    Node *parent
)
{
    Node *new_node;
    Node *temp_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_WHILE != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Expr( temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    return( __Exprlist( temp_node ) );

}   /* __While() */

/* Exprlist --> Expr | Expr Exprlist */
parser_error_t8 __Exprlist
(
    Node *parent
)
{
    Node *new_node;
    parser_error_t8 err_code;

    err_code = __add_node( parent, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Expr( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_BEGIN == __peek()->list ) )
    {
        err_code = __add_node( parent, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( __Exprlist( new_node ) );
    }

    return( PARSER_NO_ERROR );

}   /* __Exprlist() */

/* Print --> stdout Oper */
parser_error_t8 __Print
(
    Node *parent
)
{
    Node *new_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_STDOUT != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /* changed from __Expr( oper ) */
    return( __Expr( new_node ) );

}   /* __Print() */

/* Let --> let [Varlist] */
parser_error_t8 __Let
(
    Node *parent
)
{
    Node *new_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_LET != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( new_node, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Varlist( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( new_node, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    return( PARSER_NO_ERROR );

}   /* __Let() */


parser_error_t8 __Type
(
    Node *parent
)
{
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( TOK_RESERVED_WORD == cur_tok->tok_class )
    {
        switch( cur_tok->stmt )
        {
            case TOK_INT:
            case TOK_REAL:
            case TOK_BOOL:
            case TOK_STRING:
                err_code = __add_node( parent, cur_tok, NULL );
                if( PARSER_NO_ERROR != err_code )
                {
                    return( err_code );
                }

                return( PARSER_NO_ERROR );
            default:
                break;
        }
    }

    return( PARSER_UNEXPECTED_TOKEN );

}   /* __Type() */

parser_error_t8 __Constant
(
    Node *parent
)
{
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( TOK_LITERAL == cur_tok->tok_class )
    {
        switch( cur_tok->type )
        {
            case TOK_INT_TYPE:
            case TOK_REAL_TYPE:
            case TOK_BOOL_TYPE:
            case TOK_STRING_TYPE:
                err_code = __add_node( parent, cur_tok, NULL );
                if( PARSER_NO_ERROR != err_code )
                {
                    return( err_code );
                }
                return( PARSER_NO_ERROR );
            default:
                break;
        }
    }

    return( PARSER_UNEXPECTED_TOKEN );

}   /* __Constant() */

/* If --> if Expr If2 */
parser_error_t8 __If
(
    Node *parent
)
{
    Node *new_node;
    Node *temp_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_IF != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Expr( temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }
    return( __If2( temp_node ) );

}   /* __If() */

/* If2 --> Expr | Expr Expr */
parser_error_t8 __If2
(
    Node *parent
)
{
    parser_error_t8 err_code;
    Node *new_node;

    err_code = __add_node( parent, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Expr( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( ( TOK_LIST_TYPE != __peek()->tok_class ) || ( TOK_LIST_END != __peek()->list ) )
    {
        err_code = __add_node( parent, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( __Expr( new_node ) );
    }

    return( PARSER_NO_ERROR );

}   /* __If2() */

/* Stmt --> If | While | Let | Print */
parser_error_t8 __Stmt
(
    Node *parent
)
{
    parser_error_t8 err_code;

    if( TOK_RESERVED_WORD != __peek()->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    switch( __peek()->stmt )
    {
        case TOK_LET:
            return( __Let( parent ) );

        case TOK_WHILE:
            return( __While( parent ) );

        case TOK_STDOUT:
            return( __Print( parent ) );

        case TOK_IF:
            return( __If( parent ) );

        default:
            return( PARSER_UNEXPECTED_TOKEN );
    }

}   /* __Stmt() */

parser_error_t8 __Expr
(
    Node *parent
)
{
    Token *cur_tok;
    parser_error_t8 err_code;

    if( ( TOK_LITERAL == __peek()->tok_class ) || ( TOK_IDENT == __peek()->tok_class ) )
    {
        return( __Oper3( parent ) );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Expr2( parent );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }
    return( PARSER_NO_ERROR );

}   /* __Expr() */

/* Oper --> [Oper2] | Oper3 */
parser_error_t8 __Oper
(
    Node *parent
)
{

    Token *cur_tok;
    parser_error_t8 err_code;

    if( ( TOK_LITERAL == __peek()->tok_class ) || ( TOK_IDENT == __peek()->tok_class ) )
    {
        return( __Oper3( parent ) );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Oper2( parent );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }
    return( PARSER_NO_ERROR );

}   /* __Oper() */

/* Oper2 --> := Name Oper | Binop Oper Oper | Unop Oper */
parser_error_t8 __Oper2
(
    Node *parent
)
{
    Node *new_node;
    Node *temp_node;
    Token *cur_tok;
    Token  temp_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( TOK_BINARY_OPP == cur_tok->tok_class )
    {
        err_code = __add_node( parent, cur_tok, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        temp_node = new_node;
        err_code = __add_node( temp_node, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        err_code = __Oper( new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        err_code = __add_node( temp_node, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        return( __Oper( new_node ) );
    }
    else if( TOK_UNARY_OPP == cur_tok->tok_class )
    {
        err_code = __add_node( parent, cur_tok, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        err_code = __add_node( new_node, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        return( __Oper( new_node ) );
    }
    else if( TOK_ASSN_CLASS != cur_tok->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( TOK_IDENT != cur_tok->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    memcpy( &temp_tok, cur_tok, sizeof( temp_tok ) );

    err_code = __add_node( temp_node, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __Oper( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    return( __add_node( temp_node, &temp_tok, NULL ) );

}   /* __Oper2() */

/* Oper3 --> Constant | Name */
parser_error_t8 __Oper3
(
    Node *parent
)
{
    switch( __peek()->tok_class )
    {
        case TOK_LITERAL:
            return( __Constant( parent ) );

        case TOK_IDENT:
            return( __Name( parent ) );

        default:
            return( PARSER_UNEXPECTED_TOKEN );
    }

}   /* __Oper3() */

/* Expr2 --> Stmt | Oper2 */
parser_error_t8 __Expr2
(
    Node *parent
)
{
    if( TOK_RESERVED_WORD == __peek()->tok_class )
    {
        return( __Stmt( parent ) );
    }

    return( __Oper2( parent ) );

}   /* __Expr2() */

parser_error_t8 __Name
(
    Node *parent
)
{
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( TOK_IDENT == cur_tok->tok_class )
    {
        err_code = __add_node( parent, cur_tok, NULL );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        return( PARSER_NO_ERROR );
    }

    return( PARSER_UNEXPECTED_TOKEN );

}   /* __Name() */

/* Varlist --> [Name Type] | [Name Type] Varlist */
parser_error_t8 __Varlist
(
    Node *parent
)
{
    Node *new_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_PARSE_ERROR );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( TOK_IDENT != cur_tok->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __Type( parent );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __add_node( parent, &__prev_token, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __get_next_token( &cur_tok );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_FILE_END_REACHED:
            return( PARSER_FILE_END_REACHED );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_BEGIN == __peek()->list ) )
    {
        err_code = __add_node( parent, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( __Varlist( new_node ) );
    }

    return( PARSER_NO_ERROR );

}   /* __Varlist() */

/* T --> [S] */
parser_error_t8 __T
(
    Node *parent
)
{
    Node *new_node;
    Token *cur_tok;
    parser_error_t8 err_code;

    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        if( PARSER_FILE_END_REACHED == err_code )
        {
            return( PARSER_NO_ERROR );
        }
        return( err_code );
    }

    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_PARSE_ERROR );
    }

    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    err_code = __S( parent );
    switch( err_code )
    {
        case PARSER_PARSE_ERROR:
            return( PARSER_PARSE_ERROR );

        case PARSER_UNEXPECTED_TOKEN:
            return( PARSER_UNEXPECTED_TOKEN );

        case PARSER_FILE_END_REACHED:
        case PARSER_MISMATCHED_LIST:
            return( PARSER_MISMATCHED_LIST );
        case PARSER_NO_ERROR:
            break;
        default:
            return( PARSER_PARSE_ERROR );
    }

    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    if( ( TOK_LIST_TYPE == cur_tok->tok_class )
      &&( TOK_LIST_END  == cur_tok->list      ) )
    {
        err_code = __add_node( parent, cur_tok, NULL );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        return( PARSER_NO_ERROR );
    }

    return( PARSER_UNEXPECTED_TOKEN );

}   /* __T() */

//parser_error_t8 __gen_code
//(
//    Node *n
//)
//{
//    Link *cur_link;
//    char *forth_command;
//
//    if( NULL == n )
//    {
//        return( PARSER_NO_ERROR );
//    }
//
//    cur_link = n->first_child;
//    while( NULL != cur_link )
//    {
//        __gen_code( cur_link->tree_next );
//        cur_link = cur_link->next;
//    }
//
//    if( TOK_NO_CLASS == n->tok.tok_class )
//    {
//        return( PARSER_NO_ERROR );
//    }
//
//    forth_command = __get_forth_command( &( n->tok ),  );
//    if( NULL == forth_command )
//    {
//        return( PARSER_PARSE_ERROR );
//    }
//
//    fprintf( __cur_out_file, "%s", forth_command );
//    return( PARSER_NO_ERROR );
//
//}   /* __gen_code() */

//parser_error_t8 gen_code
//(
//    void
//)
//{
//    parser_error_t8 err_code;
//    if( PARSER_GEN_CODE != __parser_state )
//    {
//        return( PARSER_PARSE_ERROR );
//    }
//
//    if( NULL == __cur_out_file )
//    {
//        return( PARSER_PARSE_ERROR );
//    }
//
//    err_code = __gen_code( __parse_tree->top );
//    if( PARSER_NO_ERROR != err_code )
//    {
//        __print_error( err_code );
//    }
//    __parser_state = PARSER_CLOSE_FILE;
//    return( err_code );
//
//}   /* gen_code() */

Tree *get_parse_tree
(
    void
)
{
    return( __parse_tree );

}   /* get_parse_tree() */

parser_error_t8 init_parser
(
    void
)
{
    if( PARSER_INIT != __parser_state )
    {
        return( PARSER_ALREADY_INITIALIZED );
    }

    memset( &__prev_token, 0, sizeof( __prev_token ) );
    memset( &__cur_token,  0, sizeof( __cur_token  ) );
    memset( &__next_token, 0, sizeof( __next_token ) );

    __parse_tree = __create_tree();
    if( NULL == __parse_tree )
    {
        return( PARSER_INIT_ERROR );
    }

    if( PARSER_NO_ERROR != __init_tree( __parse_tree ) )
    {
        return( PARSER_INIT_ERROR );
    }

    if( SCN_NO_ERROR != init_scanner() )
    {
        return( PARSER_INIT_ERROR );
    }

    __parser_state = PARSER_LOAD_FILE;

    return( PARSER_NO_ERROR );

}   /* init_parser() */


parser_error_t8 load_file
(
    char *filename
)
{
    char *cur_char;
    char *cur_out_char;
    char out_fname[ MAX_FILENAME_LENGTH ];

    if( PARSER_LOAD_FILE != __parser_state )
    {
        return( PARSER_LOAD_ERROR );
    }

    if( SCN_OPEN_ERROR == load_scanner_file( filename ) )
    {
        return( PARSER_LOAD_ERROR );
    }

    if( NULL == __parse_tree )
    {
        __parse_tree = __create_tree();
        __init_tree( __parse_tree );
    }
    else
    {
        __free_parse_tree_nodes( __parse_tree->top );
    }

    memset( out_fname, 0, sizeof( out_fname ) / sizeof( *out_fname ) );
    cur_char = filename;
    cur_out_char = out_fname;
    while( ( '.' != *cur_char ) && ( '\0' != *cur_char ) )
    {
        *(cur_out_char++) = *(cur_char++);
    }
    *cur_out_char = '\0';
    strcat( out_fname, FORTH_EXTENSION );
    //__cur_out_file = fopen( out_fname, "w" );
    //if( NULL == __cur_out_file )
    //{
    //    return( PARSER_LOAD_ERROR );
    //}

    __parser_state = PARSER_PARSE_FILE;
    __end_reached  = FALSE;

    __get_next_token( NULL );

    return( PARSER_NO_ERROR );

}   /* load_file() */


parser_error_t8 parse_file
(
    void
)
{
    parser_error_t8 err_code;

    if( PARSER_PARSE_FILE != __parser_state )
    {
        return( PARSER_PARSE_ERROR );
    }

    __parse_tree->top = __create_node();
    __parse_tree->top->first_child = NULL;
    __parse_tree->top->last_child  = NULL;
    __parse_tree->top->num_children = 0;
    memset( &( __parse_tree->top->tok ), 0, sizeof( __parse_tree->top->tok ) );
    __parse_tree->top->tok.tok_class = TOK_NO_CLASS;

    err_code = __T( __parse_tree->top );

    if( ( PARSER_NO_ERROR == err_code ) && ( __end_reached ) )
    {
        __print_tree( __parse_tree->top, 0 );
    }
    else
    {
        __print_error( err_code );
    }

    //__free_parse_tree_nodes( __parse_tree->top );
    //__parse_tree->top = NULL;
    __parser_state = PARSER_GEN_CODE;

    if( __end_reached || ( PARSER_NO_ERROR != err_code ) )
    {
        return( err_code );
    }

    //gen_code( __parse_tree->top );

    __print_error( PARSER_PARSE_ERROR );
    return( PARSER_PARSE_ERROR );

}   /* parse_file() */


parser_error_t8 unload_file
(
    void
)
{
    unload_scanner_file();

    __free_parse_tree();
    __parser_state = PARSER_LOAD_FILE;

    //fclose( __cur_out_file );
    __cur_out_file = NULL;

    return( PARSER_NO_ERROR );

}   /* unload_file() */

parser_error_t8 unload_parser
(
    void
)
{
    unload_scanner();

    __free_parse_tree();
    __parser_state = PARSER_INIT;

    return( PARSER_NO_ERROR );

}   /* unload_parser() */
