/**************************************************
*
*  MODULE NAME:
*      parser.h
*
*  DESCRIPTION:
*      Public API for the parser
*
**************************************************/

#ifndef __PARSER_H__
#define __PARSER_H__

/*-------------------------------------------------
                  PROJECT INCLUDES
-------------------------------------------------*/

#include "types.h"
#include "tokens.h"

/*-------------------------------------------------
                  LITERAL CONSTANTS
-------------------------------------------------*/

typedef sint8 parser_error_t8;
enum
{
    PARSER_NO_ERROR =  0,
    PARSER_NULL_REF = -1,
    PARSER_INIT_ERROR = -2,
    PARSER_LOAD_ERROR = -3,
    PARSER_ALREADY_INITIALIZED = -4,
    PARSER_PARSE_ERROR = -5,
    PARSER_FILE_END_REACHED = -6,
    PARSER_UNEXPECTED_TOKEN = -7,
    PARSER_MISMATCHED_LIST = -8,
    PARSER_NO_MEMORY       = -9
};

/*-------------------------------------------------
                        TYPES
-------------------------------------------------*/

struct __parser_tree_type;
struct __parser_tree_node_type;
struct __parser_node_linked_list_link_type;

typedef struct __parser_node_linked_list_link_type
{
    struct __parser_node_linked_list_link_type *next;       /* pointer to next link     */
    struct __parser_tree_node_type             *tree_next;  /* pointer to child node    */
} Link;

typedef struct __parser_tree_node_type
{
    Link               *first_child;    /* node's last child        */
    Link               *last_child;     /* node's first child       */
    uint32              num_children;   /* number of children       */
    Token               tok;            /* node's token data        */
} Node;

typedef struct __parser_tree_type
{
    Node       *top;        /* top of the tree          */
    uint32      size;       /* size of the tree-not used*/
} Tree;

/*-------------------------------------------------
                  VARIABLE CONSTANTS
-------------------------------------------------*/

/*-------------------------------------------------
                        MACROS
-------------------------------------------------*/

/*-------------------------------------------------
                      PROCEDURES
-------------------------------------------------*/

//parser_error_t8 gen_code
//(
//    void
//);

Tree *get_parse_tree
(
    void
);

parser_error_t8 init_parser
(
    void
);

parser_error_t8 load_file
(
    char *filename
);

parser_error_t8 parse_file
(
    void
);

parser_error_t8 unload_file
(
    void
);

parser_error_t8 unload_parser
(
    void
);

#endif // __PARSER_H__
