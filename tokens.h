/**************************************************
*
*   NAME:
*       tokens.h
*
*   DESCRIPTION:
*       Defines the basic types for the compiler's
*       tokens
*
**************************************************/

#ifdef __AIDAN_TOKENS_H__
#define __AIDAN_TOKENS_H__

#include "types.h"

#define MAX_OPP_STR_LEN 8
#define MAX_RES_WORD_STR_LEN 8

typedef uint8 token_class_t8;
enum
{
    TOK_BINARY_OPP,
    TOK_UNARY_OPP,
    TOK_LITERAL,
    TOK_IDENT,
    TOK_RESERVED_WORD
};

typedef uint8 bin_opp_class_t8;
enum
{
    TOK_ADD_OPP = 0,
    TOK_SUB_OPP,
    TOK_MUL_OPP,
    TOK_DIV_OPP,
    TOK_MOD_OPP,
    TOK_AND_OPP,
    TOK_OR_OPP,
    TOK_EXP_OPP,
    TOK_EQ_OPP,
    TOK_LT_OPP,
    TOK_GT_OPP,
    TOK_LE_OPP,
    TOK_GE_OPP,
    TOK_NE_OPP,
    TOK_NUM_BIN_OPPS
};

typedef uint8 unary_opp_class_t8;
enum
{
    TOK_NOT_OPP = TOK_NUM_BIN_OPPS,
    TOK_NEG_OPP,
    TOK_SIN_OPP,
    TOK_COS_OPP,
    TOK_TAN_OPP
    TOK_NUM_UNARY_OPPS
};

typedef uint8 type_class_t8;
enum
{
    TOK_INT_TYPE = TOK_NUM_UNARY_OPPS,
    TOK_REAL_TYPE,
    TOK_STRING_TYPE,
    TOK_BOOL_TYPE,
    TOK_NUM_TYPES
};

typedef uint8 res_word_class_t8;
enum
{
    TOK_WHILE,
    TOK_IF,
    TOK_SIN,
    TOK_COS,
    TOK_TAN,
    TOK_TRUE,
    TOK_FALSE,
    TOK_BOOL,
    TOK_INT,
    TOK_REAL,
    TOK_STRING,
    TOK_STDOUT,
    TOK_LET,
    TOK_NUM_RESERVED_WORDS
};


struct bin_opp_type
{
    bin_opp_class_t8 opp_class;
    char in_str[ MAX_OPP_STR_LEN ];
    char out_str[ MAX_OPP_STR_LEN ];
};

struct un_opp_type
{
    unary_opp_class_t8 opp_class;
    char in_str[ MAX_OPP_STR_LEN ];
    char out_str[ MAX_OPP_STR_LEN ];
};

struct literal_type
{
    type_class_t8 type_class;
    char *str;
};

struct identifier_type
{
    ident_class_t8 ident_class;
    char *in_str;
    char *out_str;
};

struct reserved_word_type
{
    res_word_class_t8 word_class;
    char in_str[ MAX_RES_WORD_STR_LEN ];
    char out_str[ MAX_RES_WORD_STR_LEN ];
};

struct token_type;
{
    TOK_token_class_t8 token_class;
    union
    {
        struct bin_opp_type;
        struct un_opp_type;
        struct literal_type;
        struct identifier_type;
        struct reserved_word_type
    } token_data;

};
#endif // __AIDAN_TOKENS_H__
