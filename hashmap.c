#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"
#include "types.h"

#define __INITIAL_SIZE 512

enum
{
    __LOWER_INDEX    = 0,
    __UPPER_INDEX    = 26,
    __NUMBER_INDEX   = 52,
    __MISC_INDEX     = 62,
    __NUM_CHARACTERS = 63
};


typedef __map_type_t8 uint8;
enum
{
    __MAP_TYPE_STATIC,
    __MAP_TYPE_DYNAMIC
};

struct __map_element
{
    key_t8      key;
    void       *val;
    int         size;
    struct __map_element
               *next;
};

struct map
{
    uint32      size;       /* size of hash table       */
    uint32      capacity;   /* maximum size of the table*/
    __map_type_t8
                map_type;   /* type of map              */
    struct __map_element 
              **table;      /* the table                */
};

static float sin_table[ __NUM_CHARACTERS ] = 
{
    (float)sin(  1.0f ), (float)sin(  2.0f ), (float)sin(  3.0f ), (float)sin(  4.0f ), (float)sin(  5.0f ),
    (float)sin(  6.0f ), (float)sin(  7.0f ), (float)sin(  8.0f ), (float)sin(  9.0f ), (float)sin( 10.0f ),
    (float)sin( 11.0f ), (float)sin( 12.0f ), (float)sin( 13.0f ), (float)sin( 14.0f ), (float)sin( 15.0f ),
    (float)sin( 16.0f ), (float)sin( 17.0f ), (float)sin( 18.0f ), (float)sin( 19.0f ), (float)sin( 20.0f ),
    (float)sin( 21.0f ), (float)sin( 22.0f ), (float)sin( 23.0f ), (float)sin( 24.0f ), (float)sin( 25.0f ),
    (float)sin( 26.0f ), (float)sin( 27.0f ), (float)sin( 28.0f ), (float)sin( 29.0f ), (float)sin( 30.0f ),
    (float)sin( 31.0f ), (float)sin( 32.0f ), (float)sin( 33.0f ), (float)sin( 34.0f ), (float)sin( 35.0f ),
    (float)sin( 36.0f ), (float)sin( 37.0f ), (float)sin( 23.0f ), (float)sin( 39.0f ), (float)sin( 40.0f ),
    (float)sin( 41.0f ), (float)sin( 42.0f ), (float)sin( 43.0f ), (float)sin( 44.0f ), (float)sin( 45.0f ),
    (float)sin( 46.0f ), (float)sin( 47.0f ), (float)sin( 48.0f ), (float)sin( 49.0f ), (float)sin( 50.0f ),
    (float)sin( 51.0f ), (float)sin( 52.0f ), (float)sin( 53.0f ), (float)sin( 54.0f ), (float)sin( 55.0f ),
    (float)sin( 56.0f ), (float)sin( 57.0f ), (float)sin( 58.0f ), (float)sin( 59.0f ), (float)sin( 60.0f ),
    (float)sin( 61.0f ), (float)sin( 62.0f ), (float)sin( 63.0f )
};

inline __ptr_to_handle
( 
    void       *ptr     /* pointer              */ 
)
{
    return( (uint32)ptr );
    
}   /* ptr_to_handle() */

struct __map_element *__create_element
(
    void
);

int __hash_key
( 
    key_t8      key,    /* the key to hash      */  
    int         n       /* number of elements   */
                        /*  in the hash table   */
);

map_error_code_t8 __init_element
(
    struct __map_element 
               *e,      /* element to initialize    */
    key_t8      key,    /* element's key            */
    void       *val,    /* element's value data     */
    uint32      size    /* size of val in bytes     */
);

map_error_code_t8 __init_map
(
    struct map *m,      /* map to initialize    */
    uint        size,   /* size of the map      */
    __map_type_t8
                type    /* type of map to init  */
);

boolean __is_lower
( 
    char        test    /* character to test    */ 
);

boolean __is_number
( 
    char        test    /* character to test    */ 
);

boolean __is_upper
( 
    char        test    /* character to test    */ 
);


map_error_code_t8 __resize_map
(
    struct map *m,      /* map to resize        */
    uint32      new_size/* new size of the map  */
);

struct __map_element *__create_element
(
    void
)
{
    return( (struct __map_element *)malloc( sizeof( struct __map_element ) ) );
    
}   /* __create_element() */

void __free_table
(
    struct __map_element **t
    uint32 cap;
)
{
    uint32 i;
    struct __map_element *cur;
    struct __map_element *next;
    
    for( i = 0; i < cap; ++i )
    {
        cur = t[ i ];
        while( NULL != cur )
        {
            next = cur->next;
            free( cur->key );
            free( cur->val );
            free( cur );
            cur = next;
        }
    }
    free( t );
    
}   /* __free_table() */

uint32 __hash_key
( 
    key_t8      key,    /* the key to hash      */  
    int         n       /* number of elements   */
                        /*  in the hash table   */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    key_t8      ptr;    /* character in key     */
    float       tot;    /* accumulator          */
    uint32      idx;    /* index for an array   */
    
    ptr = key;
    while( *ptr != '\0' )
    {
        if( __is_upper( (char)( *ptr ) ) )
        {
            idx = __UPPER_INDEX + *ptr - 'A';
        }
        else if( __is_lower( (char)( *ptr ) ) )
        {
            idx = __LOWER_INDEX + *ptr - 'a';
        }
        else if( __is_number( (char)( *ptr ) ) )
        {
            idx = __NUMBER_INDEX + *ptr - '0';
        }
        else
        {
            idx = __MISC_INDEX;
        }
        
        tot += sin_table[ idx ];
        ++ptr;
    }
        
    tot *= n;
    return( ( (uint32)tot ) % n );
    
}   /* __hash_str() */

map_error_code_t8 __init_element
(
    struct __map_element 
               *e,      /* element to initialize    */
    key_t8      key,    /* element's key            */
    void       *val,    /* element's value data     */
    uint32      size    /* size of val in bytes     */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32  len = strlen( key );
    
    /*---------------------------------
    Make sure that our element is
    valid
    ---------------------------------*/
    if( NULL == e )
    {
        return( ERR_NULL_REF );
    }
    
    /*---------------------------------
    Allocate space for the key
    and copy the data over
    ---------------------------------*/
    e->key = (key_t8)malloc( sizeof( char ) * len );
    if( NULL == e->key )
    {
        return( ERR_NO_MEMORY );
    }
    memcpy( (void *)e->key, (void *)key, len );
    
    /*---------------------------------
    Allocate space for the value
    and copy the data over
    ---------------------------------*/
    e->val = (void *)malloc( sizeof( char ) * size );
    if( NULL == e->val )
    {
        return( ERR_NO_MEMORY );
    }
    memcpy( (void *)e->val, (void *)val, size );
    
    e->size = size;
    e->next = NULL;
    
    return( ERR_NO_ERROR );
    
}   /* __init_element() */

map_error_code_t8 __init_map
(
    struct map *m,      /* map to initialize    */
    uint        size,   /* size of the map      */
    __map_type_t8
                type    /* type of map to init  */
)
{
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }
    
    m->table = (struct __map_element **)malloc( sizeof( struct __map_element * ) * n );
    if( NULL == m->table )
    {
        return( ERR_NO_MEMORY );
    }
    
    m->size = 0;
    m->capacity = n;
    m->map_type = type;
    
    return( ERR_NO_ERROR );
    
}   /* __init_map() */

boolean __is_lower
( 
    char        test    /* character to test    */ 
)
{
    if( ( test >= 'a' ) && ( test <= 'z' ) )
    {
        return( TRUE );
    }
    
    return( FALSE );

}   /* __is_lower() */

boolean __is_number
( 
    char        test    /* character to test    */ 
)
{
    if( ( test >= '0' ) && ( test <= '9' ) )
    {
        return( TRUE );
    }
    
    return( FALSE );

}   /* __is_lower() */

boolean __is_upper
( 
    char        test    /* character to test    */ 
)
{
    if( ( test >= 'A' ) && ( test <= 'Z' ) )
    {
        return( TRUE );
    }
    
    return( FALSE );

}   /* __is_upper() */

map_error_code_t8 __resize_map
(
    struct map *m,      /* map to resize        */
    uint32      new_size/* new size of the map  */
)
{
    int i;
    map_error_code_t8 error;
    uint32 old_cap;
    struct __map_element *cur;
    struct __map_element **old_map;
    
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }
    
    old_map = m->table;
    old_cap = m->capacity;
    
    error = __init_map( m, new_size, __MAP_TYPE_DYNAMIC );
    if( ERR_NO_ERROR != error )
    {
        __free_table( old_map, old_cap );
        return( error );
    }
    
    for( i = 0; i < old_cap; ++i )
    {
        cur = old_map[ i ];
        while( NULL != cur )
        {
            error = add_map( m, cur->key, cur->val, cur->size );
            if( ERR_NO_ERROR != error )
            {
                __free_table( old_map, old_cap );
                return( error );
            }
            
            cur = cur->next;
        }
    }
    
    __free_table( old_map, old_cap );
    
}   /* __resize_map() */

struct map *create_map
(
    void
)
{
    return( (struct map *)malloc( sizeof( struct map ) ) );
    
}   /* create() */

map_error_code_t8 init_dynamic_map
( 
    struct map *m       /* map to initialize    */ 
)
{
    return( __init_map( m, __INITIAL_SIZE, __MAP_TYPE_DYNAMIC ) );
    
}   /* init_dynamic_map() */

map_error_code_t8 init_static_map
( 
    struct map *m       /* map to initialize    */ 
)
{
    return( __init_map( m, __INITIAL_SIZE, __MAP_TYPE_STATIC ) );
    
}   /* init_static_map() */

uint32 add_map
( 
    struct map *m,      /* map we're adding to  */
    key_t8      key,    /* the element's key    */
    void       *val,    /* the element's value  */
    uint32      size    /* size of val in bytes */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32 idx;
    struct __map_element *new_element;
    
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }
    
    if( ( __MAP_TYPE_DYNAMIC == m->map_type )
     && ( ( (float)m->size / m->capacity ) >= 1.75f ) )
    {
        if( ERR_NO_ERROR != __resize_map( m, m->capacity << 1 ) )
        {
            return( 0 );
        }
    }
    
    new_element = __create_element();
    if( NULL == new_element )
    {
        return( 0 );
    }
    
    if( ERR_NO_ERROR != __init_element( new_element, key, val, size ) )
    {
        return( 0 );
    }
    
    idx = __hash_key( key, m->capacity );
    new_element->next = m->table[ idx ];
    m->table[ idx ] = new_element;
    ++m->size;
    
    return( __ptr_to_handle( new_element ) );
    
}   /* add_map() */

boolean is_in_map
( 
    struct map *m,      /* map                  */ 
    key_t8      key     /* key to find          */
)
{
    int idx;
    struct __map_element *cur;
    
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }
    
    idx = __hash_key( key, m->capacity );
    cur = m->table[ idx ];
    
    while( NULL != cur )
    {
        if( 0 == strcmp( key, cur->key ) )
        {
            return( TRUE );
        }
        cur = cur->next;
    }
    
    return( FALSE );
    
}   /* is_in_map() */

void *get
(
    struct map *m,      /* map                  */
    key_t8      key     /* key to get           */
)
{
    int idx;
    struct __map_element *cur;
    
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }
    
    idx = __hash_key( key, m->capacity );
    cur = m->table[ idx ];
    
    while( NULL != cur )
    {
        if( 0 == strcmp( key, cur->key ) )
        {
            return( cur->val );
        }
        cur = cur->next;
    }
    
    return( NULL );
    
}   /* get() */


void free_map
(
    struct map *m       /* map to free          */
)
{
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }
    
    __free_table( m->table );
    free( m );
    
}   /* free_map() */