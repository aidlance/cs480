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

    /*---------------------------------
    If we've hit the end of file,
    during the last call to this
    function, return the EOF
    ---------------------------------*/
    if( finished )
    {
        __parser_state = PARSER_CLOSE_FILE;
        return( PARSER_FILE_END_REACHED );
    }

    /*---------------------------------
    Otherwise, just return normally
    ---------------------------------*/
    return( PARSER_NO_ERROR );

}   /* __get_next_token() */


/**************************************************
*
*   FUNCTION:
*       __init_link - "Initialize Link"
*
*   DESCRIPTION:
*       Initializes a link for the tree's
*       list of children nodes
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_NULL_REF if the
*         link reference is invalid
*
**************************************************/
parser_error_t8 __init_link
(
    Link       *l       /* link to be initialized       */
)
{
    /*---------------------------------
    Check if reference is valid
    ---------------------------------*/
    if( NULL == l )
    {
        return( PARSER_NULL_REF );
    }

    /*---------------------------------
    Initialize the link's values and
    return
    ---------------------------------*/
    l->next      = NULL;
    l->tree_next = NULL;

    return( PARSER_NO_ERROR );

}   /* __init_link() */


/**************************************************
*
*   FUNCTION:
*       __init_tree - "Initialize Tree"
*
*   DESCRIPTION:
*       Initializes a tree
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there were
*         no errors
*       - Returns PARSER_NULL_REF if the tree
*         reference is invalid
*
**************************************************/
parser_error_t8 __init_tree
(
    Tree       *t       /* tree to be initialized       */
)
{
    /*---------------------------------
    Check for invalid reference
    ---------------------------------*/
    if( NULL == t )
    {
        return( PARSER_NULL_REF );
    }

    /*---------------------------------
    Initialize tree values and return
    ---------------------------------*/
    t->top  = NULL;
    t->size = 0;

    return( PARSER_NO_ERROR );

}   /* __init_tree() */


/**************************************************
*
*   FUNCTION:
*       __peek - "Peek"
*
*   DESCRIPTION:
*       Returns a pointer to the next token
*       to add to the parse tree
*
*   RETURNS
*       Returns a pointer to a token
*
**************************************************/
Token *__peek
(
    void
)
{
    return( &__next_token );

}   /* __peek() */


/**************************************************
*
*   FUNCTION:
*       __print_error - "Print Error"
*
*   DESCRIPTION:
*       Given an error code, this prints out
*       an error message
*
**************************************************/
void __print_error
(
    parser_error_t8     err_code        /* error code to print      */
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


/**************************************************
*
*   FUNCTION:
*       __print_tree - "Print Tree"
*
*   DESCRIPTION:
*       This function prints the parse tree
*
*   NOTES:
*       This is only to be used as a debugging
*       tool
*
**************************************************/
void __print_tree
(
    Node       *n,      /* current tree node        */
    uint        depth   /* current depth            */
)
{
#ifdef __DEBUG_PRINTS_PARSER
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint        i;      /* for-loop iterator        */
    Link       *temp;   /* temporary link           */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == n )
    {
        return;
    }

    /*---------------------------------
    Perform a post-order traversal on
    the tree by looping through every
    child of the current node and
    recursively calling this function
    ---------------------------------*/
    temp = n->first_child;
    while( NULL != temp )
    {
        __print_tree( temp->tree_next, depth + 1 );
        temp = temp->next;
    }

    /*---------------------------------
    If the node is just a place-holder,
    don't print anything
    ---------------------------------*/
    if( TOK_NO_CLASS == n->tok.tok_class )
    {
        return;
    }

    /*---------------------------------
    Print the token and return
    ---------------------------------*/
    for( i = 0; i < depth; ++i )
    {
        fprintf( stdout, "    " );
    }
    print_minimal_token( &( n->tok ) );
#endif

}   /* __print_tree() */


/**************************************************
*
*   FUNCTION:
*       __S - "Start 1"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for whatever
*       S stands for:
*
*           S --> [S2 | Oper3 S | Oper3
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of a statement
*
**************************************************/
parser_error_t8 __S
(
    Node        *parent     /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Check if the next token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != __peek()->tok_class ) || ( TOK_LIST_BEGIN != __peek()->list ) )
    {
        /*-----------------------------
        If it isn't, call the __Oper3()
        function
        -----------------------------*/
        err_code = __Oper3( parent );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        If the next token is a "]",
        then return
        ----------------------------*/
        if( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_END == __peek()->list ) )
        {
            return( PARSER_NO_ERROR );
        }

        /*-----------------------------
        Otherwise, recursively call
        this function again and return
        -----------------------------*/
        err_code = __S( parent );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( PARSER_NO_ERROR );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add an empty node to the tree
    ---------------------------------*/
    err_code = __add_node( parent, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( new_node, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Call this function and return
    ---------------------------------*/
    return( __S2( new_node ) );

}   /* __S() */


/**************************************************
*
*   FUNCTION:
*       __S2 - "Start 2"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for an
*       expression:
*
*           S --> ] | S] | Expr2] | ]S
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of a statement
*
**************************************************/
parser_error_t8 __S2
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Check if the next token is a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != __peek()->tok_class ) || ( TOK_LIST_END != __peek()->list ) )
    {
        /*-----------------------------
        If the next token is a "[,"
        an identifier, or a constant,
        then call some function.
        Otherwise, call the expression
        function.
        -----------------------------*/
        if( ( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_BEGIN == __peek()->list ) )
         || ( TOK_LITERAL    == __peek()->tok_class )
         || ( TOK_IDENT      == __peek()->tok_class ) )
        {
            err_code = __S( parent );
            if( PARSER_NO_ERROR != err_code )
            {
                return( err_code );
            }
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

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_MISMATCHED_LIST );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    If the next character is a "[," an
    identifier, or a constant, then
    call __S()
    ---------------------------------*/
    if( ( ( TOK_LIST_TYPE  == __peek()->tok_class )
       && ( TOK_LIST_BEGIN == __peek()->list ) )
     || ( TOK_LITERAL == __peek()->tok_class )
     || ( TOK_IDENT   == __peek()->tok_class ) )
    {
        return( __S( parent ) );
    }

    /*---------------------------------
    And return
    ---------------------------------*/
    return( PARSER_NO_ERROR );

}   /* __S2() */


/**************************************************
*
*   FUNCTION:
*       __While - "While"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       while statement:
*
*           While --> while Expr Exprlist
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the while statement
*
**************************************************/
parser_error_t8 __While
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;       /* new node             */
    Node               *temp_node;      /* temporary node       */
    Token              *cur_tok;        /* current token        */
    parser_error_t8     err_code;       /* error code           */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    make sure that the token is a
    while keyword
        -----------------------------*/
    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_WHILE != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add a blank node to the tree
    ---------------------------------*/
    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Evaluate the conditional part of
    the while statement
    ---------------------------------*/
    err_code = __Expr( temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add a blank node to the tree
    ---------------------------------*/
    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Evaluate the body of the while
    ---------------------------------*/
    return( __Exprlist( temp_node ) );

}   /* __While() */


/**************************************************
*
*   FUNCTION:
*       __Exptlist - "Expression List"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       while statement's expression list:
*
*           Exprlist --> Expr | Expr Exprlist
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the expression list
*
**************************************************/
parser_error_t8 __Exprlist
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Add a new blank node to the tree
    ---------------------------------*/
    err_code = __add_node( parent, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Evaluate the expression's
    grammatical correctness
    ---------------------------------*/
    err_code = __Expr( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    If there's another expression
    (a "[" is the next token), then
    evaluate the expression for
    grammatical correctness and return
    ---------------------------------*/
    if( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_BEGIN == __peek()->list ) )
    {
        err_code = __add_node( parent, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( __Exprlist( new_node ) );
    }

    /*---------------------------------
    and return
    ---------------------------------*/
    return( PARSER_NO_ERROR );

}   /* __Exprlist() */


/**************************************************
*
*   FUNCTION:
*       __Print - "Print"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       stdout function:
*
*           Print --> stdout Oper
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*
**************************************************/
parser_error_t8 __Print
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Check if it's the stdout keyword
    ---------------------------------*/
    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_STDOUT != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add a node for  the statement
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add a new blank node to be the
    child of the keyword
    ---------------------------------*/
    err_code = __add_node( new_node, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Process the next expression
    ---------------------------------*/
    return( __Oper( new_node ) );

}   /* __Print() */


/**************************************************
*
*   FUNCTION:
*       __Let - "Let"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       let statement:
*
*           Let --> let [Varlist]
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the let statement
*
**************************************************/
parser_error_t8 __Let
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a let
    keyword
    ---------------------------------*/
    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_LET != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add a blank node to the tree
    ---------------------------------*/
    err_code = __add_node( new_node, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( new_node, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Check if the variable list conforms
    to the grammar
    ---------------------------------*/
    err_code = __Varlist( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that it's a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( new_node, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    And return
    ---------------------------------*/
    return( PARSER_NO_ERROR );

}   /* __Let() */


/**************************************************
*
*   FUNCTION:
*       __Type - "Type"
*
*   DESCRIPTION:
*       Verifies that the next token is type
*       keyword
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the token wasn't a type keyword
*
**************************************************/
parser_error_t8 __Type
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Token           *cur_tok;   /* current token        */
    parser_error_t8  err_code;  /* error code           */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is a type
    keyword
    ---------------------------------*/
    if( TOK_RESERVED_WORD == cur_tok->tok_class )
    {
        switch( cur_tok->stmt )
        {
            case TOK_INT:
            case TOK_REAL:
            case TOK_BOOL:
            case TOK_STRING:
                /*---------------------
                Add a new node to the
                parse tree and return
                ---------------------*/
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

    /*---------------------------------
    If it isn't, then throw an error
    ---------------------------------*/
    return( PARSER_UNEXPECTED_TOKEN );

}   /* __Type() */


/**************************************************
*
*   FUNCTION:
*       __Constant - "Constant"
*
*   DESCRIPTION:
*       Verifies that the next token is a constant
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the token wasn't a constant
*
**************************************************/
parser_error_t8 __Constant
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Token              *cur_tok;    /* current token        */
    parser_error_t8     err_code;   /* error code           */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is a
    literal and has a valid type
    ---------------------------------*/
    if( TOK_LITERAL == cur_tok->tok_class )
    {
        switch( cur_tok->type )
        {
            case TOK_INT_TYPE:
            case TOK_REAL_TYPE:
            case TOK_BOOL_TYPE:
            case TOK_STRING_TYPE:
                /*---------------------
                Add a node to the tree
                and return
                --------------------*/
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

    /*---------------------------------
    If it wasn't a literal or the
    type wasn't valid, throw an error
    ---------------------------------*/
    return( PARSER_UNEXPECTED_TOKEN );

}   /* __Constant() */


/**************************************************
*
*   FUNCTION:
*       __If - "If"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       if statement:
*
*           If --> if Expr If2
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the if statement
*
**************************************************/
parser_error_t8 __If
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Node               *temp_node;  /* temporary node   */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Grab next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is an if
    keyword
    ---------------------------------*/
    if( ( TOK_RESERVED_WORD != cur_tok->tok_class ) || ( TOK_IF != cur_tok->stmt ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Create a new blank node
    ---------------------------------*/
    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    The next expression is the
    conditional part of the if
    ---------------------------------*/
    err_code = __Expr( temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add a new node and check whether
    the branches conform to the grammar
    ---------------------------------*/
    err_code = __add_node( new_node, NULL, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }
    return( __If2( temp_node ) );

}   /* __If() */


/**************************************************
*
*   FUNCTION:
*       __If2 - "If 2"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       if statement's branches:
*
*           If2 --> Expr | Expr Expr
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the if statement's
*         branches
*
**************************************************/
parser_error_t8 __If2
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    parser_error_t8     err_code;   /* error code           */
    Node               *new_node;   /* new node             */

    /*---------------------------------
    Add a blank node to the tree
    ---------------------------------*/
    err_code = __add_node( parent, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Evaluate the first branch
    ---------------------------------*/
    err_code = __Expr( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    If there is a second branch ("["
    is the next token), then evaluate
    the branch and return
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != __peek()->tok_class ) || ( TOK_LIST_END != __peek()->list ) )
    {
        err_code = __add_node( parent, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }
        return( __Expr( new_node ) );
    }

    /*---------------------------------
    And return
    ---------------------------------*/
    return( PARSER_NO_ERROR );

}   /* __If2() */


/**************************************************
*
*   FUNCTION:
*       __Stmt - "Statement"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for a
*       statement:
*
*           Stmt --> If | While | Let | Print
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of a statement
*
**************************************************/
parser_error_t8 __Stmt
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Make sure that the next token is
    a keyword
    ---------------------------------*/
    if( TOK_RESERVED_WORD != __peek()->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Call the corresponding function
    and return
    ---------------------------------*/
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


/**************************************************
*
*   FUNCTION:
*       __Expr - "Expression 1"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for an
*       expression:
*
*           Expr --> [Expr2] | Oper3
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of an expression
*
**************************************************/
parser_error_t8 __Expr
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Token              *cur_tok;    /* current token        */
    parser_error_t8     err_code;   /* error code           */

    /*---------------------------------
    If the next token is a literal
    or an identifier, then make sure
    that the next token conforms
    to the corresponding grammar
    ---------------------------------*/
    if( ( TOK_LITERAL == __peek()->tok_class ) || ( TOK_IDENT == __peek()->tok_class ) )
    {
        return( __Oper3( parent ) );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the parse tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the innards conform to
    the grammar of the inner expression
    ---------------------------------*/
    err_code = __Expr2( parent );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the parse tree
    and return
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }
    return( PARSER_NO_ERROR );

}   /* __Expr() */


/**************************************************
*
*   FUNCTION:
*       __Oper - "Operator 1"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for an
*       operator:
*
*           Oper --> [Oper2] | Oper3
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of an operator
*
**************************************************/
parser_error_t8 __Oper
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Token              *cur_tok;    /* current token        */
    parser_error_t8     err_code;   /* error code           */

    /*---------------------------------
    Make sure the next token is a "["
    ---------------------------------*/
    if( ( TOK_LITERAL == __peek()->tok_class ) || ( TOK_IDENT == __peek()->tok_class ) )
    {
        return( __Oper3( parent ) );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the next few tokens
    conform to the grammar of the
    inner operator expression
    ---------------------------------*/
    err_code = __Oper2( parent );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    and return
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }
    return( PARSER_NO_ERROR );

}   /* __Oper() */


/**************************************************
*
*   FUNCTION:
*       __Oper2 - "Operator 2"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the inner
*       operator expression:
*
*           Oper2 --> := Name Oper
                    | Binop Oper Oper
                    | Unop Oper
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the inner operator
*         expression
*
**************************************************/
parser_error_t8 __Oper2
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Node               *temp_node;  /* temporary node   */
    Token              *cur_tok;    /* current token    */
    Token               temp_tok;   /* temporary token  */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Check if it's a binary operator
    ---------------------------------*/
    if( TOK_BINARY_OPP == cur_tok->tok_class )
    {
        /*-----------------------------
        Add the token to the tree
        -----------------------------*/
        err_code = __add_node( parent, cur_tok, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Add an empty node to the tree
        -----------------------------*/
        temp_node = new_node;
        err_code = __add_node( temp_node, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Confirm that the next few
        tokens conform to the grammar
        of an operator expression
        -----------------------------*/
        err_code = __Oper( new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Add another blank node
        -----------------------------*/
        err_code = __add_node( temp_node, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Make sure that the second
        operation conforms to the
        operation grammar
        -----------------------------*/
        return( __Oper( new_node ) );
    }

    /*---------------------------------
    Check if we are a unary operator
    ---------------------------------*/
    else if( TOK_UNARY_OPP == cur_tok->tok_class )
    {
        /*-----------------------------
        Add the token to the tree
        -----------------------------*/
        err_code = __add_node( parent, cur_tok, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Add an empty node to the tree
        -----------------------------*/
        err_code = __add_node( new_node, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Make sure that we conform to
        the grammar of an operation
        -----------------------------*/
        return( __Oper( new_node ) );
    }
    else if( TOK_ASSN_CLASS != cur_tok->tok_class )
    {
        /*-----------------------------
        If we aren't an assignment,
        then this doesn't conform
        to our grammar
        -----------------------------*/
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, &temp_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Verify that the token is an
    identifier
    ---------------------------------*/
    if( TOK_IDENT != cur_tok->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Copy the token over to the
    temporary token. We add the
    identifier and the operation to
    the tree in reverse order so that
    the code generation is easier
    (the assignment operator needs
     to have two children).
    ---------------------------------*/
    memcpy( &temp_tok, cur_tok, sizeof( temp_tok ) );

    /*---------------------------------
   *** 3/14: CHANGED cur_tok TO NULL ***

    Add a blank node to the tree
    ---------------------------------*/
    err_code = __add_node( temp_node, NULL, &new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the operation conforms
    to the grammar of the operation
    ---------------------------------*/
    err_code = __Oper( new_node );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Finally, add the token to the
    tree and return.
    ---------------------------------*/
    return( __add_node( temp_node, &temp_tok, NULL ) );

}   /* __Oper2() */


/**************************************************
*
*   FUNCTION:
*       __Oper3 - "Operator 3"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       third operator production:
*
*           Oper3 --> Constant | Name
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the operator type
*
**************************************************/
parser_error_t8 __Oper3
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Determine which function to call
    based on the next token's
    token class
    ---------------------------------*/
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


/**************************************************
*
*   FUNCTION:
*       __Expr2 - "Expression 2"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for the
*       second expression production:
*
*           Expr2 --> Stmt | Oper2
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of the expression type
*
**************************************************/
parser_error_t8 __Expr2
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    If the next token is a statement,
    call the statement function
    ---------------------------------*/
    if( TOK_RESERVED_WORD == __peek()->tok_class )
    {
        return( __Stmt( parent ) );
    }

    /*---------------------------------
    Otherwise, call the operator
    function
    ---------------------------------*/
    return( __Oper2( parent ) );

}   /* __Expr2() */


/**************************************************
*
*   FUNCTION:
*       __Name - "Name"
*
*   DESCRIPTION:
*       Verifies that the next token is an
*       identifier
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the token wasn't an identifier
*
**************************************************/
parser_error_t8 __Name
(
    Node       *parent  /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Token              *cur_tok;    /* current token        */
    parser_error_t8     err_code;   /* error code           */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Is the token an identifier?
    ---------------------------------*/
    if( TOK_IDENT == cur_tok->tok_class )
    {
        /*-----------------------------
        Add the identifier to the tree
        and return
        -----------------------------*/
        err_code = __add_node( parent, cur_tok, NULL );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        return( PARSER_NO_ERROR );
    }

    /*---------------------------------
    Not an identifier? Not a problem!
    Just fail.
    ---------------------------------*/
    return( PARSER_UNEXPECTED_TOKEN );

}   /* __Name() */


/**************************************************
*
*   FUNCTION:
*       __Varlist - "Variable List"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for a
*       variable list:
*
*           Varlist --> [Name Type]
*                     | [Name Type] Varlist
*
*       The sub-tree of a Varlist will look
*       like:
*               Parent
*                 |
*             ----|---------------
*            /    |    \         |
*           [     |     ]        |
*                / \           BLANK
*               /   \            |
*              ID  Type       Varlist
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of a variable list
*
**************************************************/
parser_error_t8 __Varlist
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_PARSE_ERROR );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is an
    identifier

    NOTE: We do this instead of calling
          __Name() so that we can add
          the identifier to the tree
          after the type. This makes
          the code generator produce
          valid code during its
          traversal
    ---------------------------------*/
    if( TOK_IDENT != cur_tok->tok_class )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Make sure the next thing is a type
    ---------------------------------*/
    err_code = __Type( parent );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Add the identifier to the tree

    NOTE: We know it's safe to do this
          because the type always
          follows the identifier in
          a valid program
    ---------------------------------*/
    err_code = __add_node( parent, &__prev_token, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_END != cur_tok->list ) )
    {
        return( PARSER_UNEXPECTED_TOKEN );
    }

    /*---------------------------------
    Add the node to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Check if the next token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE == __peek()->tok_class ) && ( TOK_LIST_BEGIN == __peek()->list ) )
    {
        /*-----------------------------
        Add an empty node to the tree
        -----------------------------*/
        err_code = __add_node( parent, NULL, &new_node );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        /*-----------------------------
        Recursively call this function
        and return
        -----------------------------*/
        return( __Varlist( new_node ) );
    }

    /*---------------------------------
    If it isn't, then return
    ---------------------------------*/
    return( PARSER_NO_ERROR );

}   /* __Varlist() */


/**************************************************
*
*   FUNCTION:
*       __T - "T?"
*
*   DESCRIPTION:
*       Verifies that the next few tokens
*       match the grammar's rules for a
*       program:
*
*           T --> [S] | Empty
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the statement didn't conform to
*         the grammar of a program
*       - Returns PARSER_PARSE_ERROR if
*         the program doesn't have "[" as the
*         first token
*       - Returns PARSER_MISMATCHED_LIST if
*         "[" and "]" are mismatched.
*
**************************************************/
parser_error_t8 __T
(
    Node       *parent      /* parent node          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    Node               *new_node;   /* new node         */
    Token              *cur_tok;    /* current token    */
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Grab the next token. If the token
    is an EOF, accept the program
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        if( PARSER_FILE_END_REACHED == err_code )
        {
            return( PARSER_NO_ERROR );
        }
        return( err_code );
    }

    /*---------------------------------
    Make sure the token is a "["
    ---------------------------------*/
    if( ( TOK_LIST_TYPE != cur_tok->tok_class ) || ( TOK_LIST_BEGIN != cur_tok->list ) )
    {
        return( PARSER_PARSE_ERROR );
    }

    /*---------------------------------
    Add the token to the tree
    ---------------------------------*/
    err_code = __add_node( parent, cur_tok, NULL );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure the program conforms to
    the grammar of the S production
    ---------------------------------*/
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

    /*---------------------------------
    Grab the next token
    ---------------------------------*/
    err_code = __get_next_token( &cur_tok );
    if( PARSER_NO_ERROR != err_code )
    {
        return( err_code );
    }

    /*---------------------------------
    Make sure that the token is a "]"
    ---------------------------------*/
    if( ( TOK_LIST_TYPE == cur_tok->tok_class )
      &&( TOK_LIST_END  == cur_tok->list      ) )
    {
        /*-----------------------------
        Add the token to the tree
        -----------------------------*/
        err_code = __add_node( parent, cur_tok, NULL );
        if( PARSER_NO_ERROR != err_code )
        {
            return( err_code );
        }

        return( PARSER_NO_ERROR );
    }

    /*---------------------------------
    And return
    ---------------------------------*/
    return( PARSER_UNEXPECTED_TOKEN );

}   /* __T() */


/**************************************************
*
*   FUNCTION:
*       get_parse_tree - "Get Parse Tree"
*
*   DESCRIPTION:
*       Returns the parse tree
*
*   RETURNS:
*       Returns the parse tree
*
**************************************************/
Tree *get_parse_tree
(
    void
)
{
    return( __parse_tree );

}   /* get_parse_tree() */


/**************************************************
*
*   FUNCTION:
*       init_parser - "Initialize Parser"
*
*   DESCRIPTION:
*       Initializes the parser
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_ALREADY_INNITIALIZED
*         if the parser has already been
*         initialized
*       - Returns PARSER_INIT_ERROR if the
*         parser was unable to create or
*         initialize its tree, or if the
*         scanner couldn't be initialized
*
**************************************************/
parser_error_t8 init_parser
(
    void
)
{
    /*---------------------------------
    Check if we've already been
    initialized
    ---------------------------------*/
    if( PARSER_INIT != __parser_state )
    {
        return( PARSER_ALREADY_INITIALIZED );
    }

    /*---------------------------------
    Clear the tokens
    ---------------------------------*/
    memset( &__prev_token, 0, sizeof( __prev_token ) );
    memset( &__cur_token,  0, sizeof( __cur_token  ) );
    memset( &__next_token, 0, sizeof( __next_token ) );

    /*---------------------------------
    Create and initialize the parse
    tree
    ---------------------------------*/
    __parse_tree = __create_tree();
    if( NULL == __parse_tree )
    {
        return( PARSER_INIT_ERROR );
    }

    if( PARSER_NO_ERROR != __init_tree( __parse_tree ) )
    {
        return( PARSER_INIT_ERROR );
    }

    /*---------------------------------
    Initialize the scanner
    ---------------------------------*/
    if( SCN_NO_ERROR != init_scanner() )
    {
        return( PARSER_INIT_ERROR );
    }

    /*---------------------------------
    Make sure we update our state
    and return
    ---------------------------------*/
    __parser_state = PARSER_LOAD_FILE;

    return( PARSER_NO_ERROR );

}   /* init_parser() */


/**************************************************
*
*   FUNCTION:
*       load_file
*
*   DESCRIPTION:
*       Loads a file so that it can be parsed
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_LOAD_ERROR if
*         the parser isn't ready to load
*         a new file, or if the scanner
*         couldn't load the file
*
**************************************************/
parser_error_t8 load_file
(
    char       *filename    /* name of the file to open */
)
{
    /*---------------------------------
    Check if we're in the right state
    ---------------------------------*/
    if( PARSER_LOAD_FILE != __parser_state )
    {
        return( PARSER_LOAD_ERROR );
    }

    /*---------------------------------
    Tell the scanner to load the file
    ---------------------------------*/
    if( SCN_OPEN_ERROR == load_scanner_file( filename ) )
    {
        return( PARSER_LOAD_ERROR );
    }

    /*---------------------------------
    Create the parse tree if it doesn't
    already exist. If it does, free all
    of its nodes
    ---------------------------------*/
    if( NULL == __parse_tree )
    {
        __parse_tree = __create_tree();
        __init_tree( __parse_tree );
    }
    else
    {
        __free_parse_tree_nodes( __parse_tree->top );
    }

    /*---------------------------------
    Update our state, initialize some
    other globals, and then return
    ---------------------------------*/
    __parser_state = PARSER_PARSE_FILE;
    __end_reached  = FALSE;

    /*---------------------------------
    Pre-load the first token
    ---------------------------------*/
    __get_next_token( NULL );

    return( PARSER_NO_ERROR );

}   /* load_file() */


/**************************************************
*
*   FUNCTION:
*       parse_file - "Parse File"
*
*   DESCRIPTION:
*       Parses the currently loaded file
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there
*         were no errors
*       - Returns PARSER_UNEXPECTED_TOKEN
*         if the file didn't conform to
*         the grammar of the language
*       - Returns PARSER_PARSE_ERROR if
*         there was an unspecified error
*       - Returns PARSER_MISMATCHED_LIST if
*         "[" and "]" are mismatched.
*
**************************************************/
parser_error_t8 parse_file
(
    void
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    parser_error_t8     err_code;   /* error code       */

    /*---------------------------------
    Check if we are in the correct
    state
    ---------------------------------*/
    if( PARSER_PARSE_FILE != __parser_state )
    {
        return( PARSER_PARSE_ERROR );
    }

    /*---------------------------------
    Add a blank node to the top
    of the tree
    ---------------------------------*/
    __parse_tree->top = __create_node();
    __parse_tree->top->first_child = NULL;
    __parse_tree->top->last_child  = NULL;
    __parse_tree->top->num_children = 0;
    memset( &( __parse_tree->top->tok ), 0, sizeof( __parse_tree->top->tok ) );
    __parse_tree->top->tok.tok_class = TOK_NO_CLASS;

    /*---------------------------------
    Perform the recursive decent
    algorithm on the file's program
    ---------------------------------*/
    err_code = __T( __parse_tree->top );

    /*---------------------------------
    If we are finished reading the file
    and there were no errors, then
    print the parse tree. Otherwise,
    print the error
    ---------------------------------*/
    if( ( PARSER_NO_ERROR == err_code ) && ( __end_reached ) )
    {
        __print_tree( __parse_tree->top, 0 );
    }
    else
    {
        __print_error( err_code );
    }

    /*---------------------------------
    Update the parser state
    ---------------------------------*/
    __parser_state = PARSER_GEN_CODE;

    /*---------------------------------
    Return the error code if there were
    no errors or if we reached the
    end of the file (and there were
    errors)
    ---------------------------------*/
    if( __end_reached || ( PARSER_NO_ERROR != err_code ) )
    {
        return( err_code );
    }

    /*---------------------------------
    Print an error and return
    ---------------------------------*/
    __print_error( PARSER_PARSE_ERROR );
    return( PARSER_PARSE_ERROR );

}   /* parse_file() */


/**************************************************
*
*   FUNCTION:
*       unload_file - "Unload File"
*
*   DESCRIPTION:
*       Unloads the current input file so
*       that a new file can be parsed
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there were
*         no errors
*
**************************************************/
parser_error_t8 unload_file
(
    void
)
{
    /*---------------------------------
    Tell the scanner to unload the
    file
    ---------------------------------*/
    unload_scanner_file();

    /*---------------------------------
    Free the parse tree and set
    the state so that we can't parse
    stuff without loading another
    file
    ---------------------------------*/
    __free_parse_tree();
    __parser_state = PARSER_LOAD_FILE;

    return( PARSER_NO_ERROR );

}   /* unload_file() */


/**************************************************
*
*   FUNCTION:
*       unload_parser - "Unload Parser"
*
*   DESCRIPTION:
*       Unloads the parser
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       - Returns PARSER_NO_ERROR if there were
*         no errors
*
**************************************************/
parser_error_t8 unload_parser
(
    void
)
{
    /*---------------------------------
    Unload the scanner
    ---------------------------------*/
    unload_scanner();

    /*---------------------------------
    Free the parse tree and set the
    state so that the parser cannot
    be used without a re-initialization
    ---------------------------------*/
    __free_parse_tree();
    __parser_state = PARSER_INIT;

    return( PARSER_NO_ERROR );

}   /* unload_parser() */
