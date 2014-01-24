/**************************************************
*
*   HEADER NAME:
*       hashmap.h
*
*   DESCRIPTION:
*       Provides a public interface for hashmap
*       functionality.
*
**************************************************/

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

/*-------------------------------------------------
PROJECT INCLUDES
-------------------------------------------------*/
#include "types.h"

/*-------------------------------------------------
                      CONSTANTS
-------------------------------------------------*/
typedef sint8 map_error_code_t8;
enum
{
    ERR_NO_ERROR  =  0,     /* "no error" identifier    */
    ERR_NO_MEMORY = -1,     /* "out of memory" error    */
    ERR_NULL_REF  = -2,     /* "null reference" error   */
    ERR_ADD_ERROR = -3      /* "element adding" error   */
};

/*-------------------------------------------------
                        TYPES
-------------------------------------------------*/

typedef char* key_t8;
struct map;
typedef struct map HashMap;

/*-------------------------------------------------
                      VARIABLES
-------------------------------------------------*/

typedef void (*disp_callback)( void *data );

/*-------------------------------------------------
                FUNCTION PROTOTYPES
-------------------------------------------------*/

struct map *create_map
(
    void
);

map_error_code_t8 init_dynamic_map
(
    struct map *m       /* map to initialize    */
);

map_error_code_t8 init_static_map
(
    struct map *m       /* map to initialize    */
);

uint32 add_map
(
    struct map *m,      /* map we're adding to  */
    key_t8      key,    /* the element's key    */
    void       *val,    /* the element's value  */
    uint32      size    /* size of val in bytes */
);

boolean is_in_map
(
    struct map *m,      /* map                  */
    key_t8      key     /* key to find          */
);

void *get
(
    struct map *m,      /* map                  */
    key_t8      key     /* key to return        */
);

uint32 get_map_size
(
    struct map *m       /* map                  */
);

uint32 get_map_capacity
(
    struct map *m       /* map                  */
);

void free_map
(
    struct map *m       /* map to free          */
);

map_error_code_t8 show_map
(
    struct map     *m,          /* map to print             */
    disp_callback   disp_func   /* display function to use  */
);

#endif  /* __HASHMAP_H__ */
