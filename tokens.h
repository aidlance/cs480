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

/*-------------------------------------------------
                PROJECT INCLUDES
-------------------------------------------------*/

#include "types.h"

/*-------------------------------------------------
                  CONSTANTS
-------------------------------------------------*/

#define MAX_OPP_STR_LEN      8  /* maximum characters in an opperator string    */
#define MAX_RES_WORD_STR_LEN 8  /* maximum characters in a reserved word        */

/*-------------------------------------
Token error types
-------------------------------------*/
typedef sint8 token_error_t8
enum
{
    TOK_NO_ERROR  =  0,         /* No error                                     */
    TOK_NOT_VALID = -1          /* error for invalid token                      */
};

/*-------------------------------------
Token class types
-------------------------------------*/
typedef uint8 token_class_t8;
enum
{
    TOK_BINARY_OPP,             /* binary operator token class type             */
    TOK_UNARY_OPP,              /* unary operator token class type              */
    TOK_LITERAL,                /* literal token class type                     */
    TOK_IDENT,                  /* identifier token class type                  */
    TOK_RESERVED_WORD,          /* reserved word token class type               */
    TOK_NUM_TOKEN_TYPES         /* number of token classes                      */
};

/*-------------------------------------
Binary operator types
-------------------------------------*/
typedef uint8 bin_opp_class_t8;
enum
{
    TOK_ADD_OPP = 0,            /* binary addition opperator ("+")      */
    TOK_SUB_OPP,                /* binary subtraciton opperator ("-")   */
    TOK_MUL_OPP,                /* binary multiplication operator ("*") */
    TOK_DIV_OPP,                /* binary division operator ("/")       */
    TOK_MOD_OPP,                /* binary modulus operator ("%")        */
    TOK_AND_OPP,                /* binary and operator ("and")          */
    TOK_OR_OPP,                 /* binary or operator ("or")            */
    TOK_EXP_OPP,                /* binary exponential operator ("^")    */
    TOK_EQ_OPP,                 /* binary equality operator ("=")       */
    TOK_LT_OPP,                 /* binary less than operator ("<")      */
    TOK_GT_OPP,                 /* binary greater than operator (">")   */
    TOK_LE_OPP,                 /* binary less than equal to ("<=")     */
    TOK_GE_OPP,                 /* binary greater than equal to (">=")  */
    TOK_NE_OPP,                 /* binary not equal to ("!=")           */
    TOK_NUM_BIN_OPPS            /* number of binary operators           */
};

/*-------------------------------------
Unary operator types
-------------------------------------*/
typedef uint8 unary_opp_class_t8;
enum
{
    TOK_NOT_OPP = 0,            /* unary not operation ("not")          */
    TOK_NEG_OPP,                /* unary negation operation ("-")       */
    TOK_SIN_OPP,                /* unary sin operation ("sin")          */
    TOK_COS_OPP,                /* unary cos operation ("cos")          */
    TOK_TAN_OPP                 /* unary tan operation ("tan")          */
    TOK_NUM_UNARY_OPPS          /* number of unary operations           */
};

/*-------------------------------------
Primitive type classes
-------------------------------------*/
typedef uint8 type_class_t8;
enum
{
    TOK_INT_TYPE = 0,           /* integer type ("int")                 */
    TOK_REAL_TYPE,              /* float/double type ("real")           */
    TOK_STRING_TYPE,            /* string type ("string")               */
    TOK_BOOL_TYPE,              /* boolean type ("bool")                */
    TOK_NUM_TYPES               /* number of primitive types            */
};

/*-------------------------------------
Reserved word types
-------------------------------------*/
typedef uint8 res_word_class_t8;
enum
{
    TOK_WHILE,                  /* "while"                      */
    TOK_IF,                     /* "if"                         */
    TOK_SIN,                    /* "sin"                        */
    TOK_COS,                    /* "cos"                        */
    TOK_TAN,                    /* "tan"                        */
    TOK_TRUE,                   /* "true"--boolean constant     */
    TOK_FALSE,                  /* "false"--boolean constant    */
    TOK_BOOL,                   /* "bool"                       */
    TOK_INT,                    /* "int"                        */
    TOK_REAL,                   /* "real"                       */
    TOK_STRING,                 /* "string"                     */
    TOK_STDOUT,                 /* "stdout"                     */
    TOK_LET,                    /* "let"                        */
    TOK_AND,                    /* "and"                        */
    TOK_OR,                     /* "or"                         */
    TOK_NOT,                    /* "not"                        */
    TOK_NUM_RESERVED_WORDS      /* number of reserved words     */
};

/*-------------------------------------------------
                      TYPES
-------------------------------------------------*/

/*-------------------------------------
Operator types
-------------------------------------*/
typedef union
{
    struct
    {
        bin_opp_class_t8    opp_class;                  /* binary operator token class      */
        char                in_str[  MAX_OPP_STR_LEN ]; /* string read by scanner           */
        char                out_str[ MAX_OPP_STR_LEN ]; /* corresponding gforth equivalent  */
    };  /* binary operator */

    struct
    {
        unary_opp_class_t8  opp_class;                  /* unary operator token class       */
        char                in_str[  MAX_OPP_STR_LEN ]; /* streang read by scanner          */
        char                out_str[ MAX_OPP_STR_LEN ]; /* corresponding gforth equivalent  */
    };  /* unary operator */
} opp_type;

/*-------------------------------------
Literal token type
-------------------------------------*/
struct literal_type
{
    type_class_t8       type_class; /* type class of the literal        */
    char               *str;        /* literal's string representation  */
};

/*-------------------------------------
Identifier token type
-------------------------------------*/
struct identifier_type
{
    char               *in_str;     /* identifier received from scThe stdoutstatement has the form [stdout atom] or [stdout expression]. The anner */
    char               *out_str;    /* gforth output                    */
};

/*-------------------------------------
Reserved word type
-------------------------------------*/
struct reserved_word_type
{
    res_word_class_t8   word_class;                         /* reserved word class          */
    char                in_str[  MAX_RES_WORD_STR_LEN ];    /* string received from scanner */
    char                out_str[ MAX_RES_WORD_STR_LEN ];    /* gforth output                */
};

/*-------------------------------------
Token type
-------------------------------------*/
struct token_type;
{
    token_class_t8 token_class;                 /* token class              */
    union
    {
        opp_type                    opp;        /* operator                 */
        struct literal_type         literal;    /* literal                  */
        struct identifier_type      id;         /* identifier               */
        struct reserved_word_type   res_word;   /* reserved word            */
    };

};

#endif // __AIDAN_TOKENS_H__
