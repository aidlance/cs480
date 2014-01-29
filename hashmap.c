/**************************************************
*
*   MODULE NAME:
*       hashmap.c
*
*   DESCRIPTION:
*       Implementation of the hashmap
*       functionality.
*
**************************************************/

/*-------------------------------------------------
                PROJECT INCLUDES
-------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"
#include "types.h"

/*-------------------------------------------------
                      CONSTANTS
-------------------------------------------------*/
#define __INITIAL_SIZE     512
#define __HASH_PRIME       16777619
#define __INITIAL_HASH_IDX 2166136261UL

typedef uint8 __map_type_t8;
enum
{
    __MAP_TYPE_STATIC,      /* static map identifier    */
    __MAP_TYPE_DYNAMIC      /* dynamic map identifier   */
};

/*-------------------------------------------------
                      TYPES
-------------------------------------------------*/

struct __map_element
{
    key_t8      key;        /* element's key data       */
    void       *val;        /* element's value data     */
    int         size;       /* val's size in bytes      */
    struct __map_element
               *next;       /* pointer to next element  */
};  /* __map_element */

struct map
{
    uint32      size;       /* size of hash table       */
    uint32      capacity;   /* maximum size of the table*/
    __map_type_t8
                map_type;   /* type of map              */
    struct __map_element
              **table;      /* the table                */
};  /* map */

/*-------------------------------------------------
                      MACROS
-------------------------------------------------*/

/**************************************************
*
*   FUNCTION:
*       __ptr_to_handle - "Pointer to handle"
*
*   DESCRIPTION:
*       This function takes a pointer as an
*       input and returns a 32-bit integer
*       handle.
*
*   RETURNS:
*       Returns a handle
*
**************************************************/
inline uint32 __ptr_to_handle
(
    void       *ptr     /* pointer              */
)
{
    return( (uint32)ptr );

}   /* __ptr_to_handle() */

/*-------------------------------------------------
            PRIVATE FUNCTION PROTOTYPES
-------------------------------------------------*/

struct __map_element *__create_element
(
    void
);

void __free_element_data
(
   struct __map_element *e  /* element to free  */
);

void __free_table
(
    struct __map_element
              **t,      /* the table we're freeing  */
    uint32      cap     /* capacity of the table    */
);

struct __map_element *__get_element
(
    struct map *m,      /* map              */
    key_t8      key     /* key to grab      */
);

uint32 __hash_key
(
    key_t8      key,    /* the key to hash      */
    uint32      n       /* number of elements   */
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

map_error_code_t8 __resize_map
(
    struct map *m,      /* map to resize        */
    uint32      new_size/* new size of the map  */
);

/*-------------------------------------------------
                   PROCEDURES
-------------------------------------------------*/


/**************************************************
*
*   FUNCTION:
*       __create_element - "Create Element"
*
*   DESCRIPTION:
*       This function allocates space for a
*       map element.
*
*   RETURNS:
*       Returns a pointer to a map element.
*
*   ERRORS:
*       * If this function was unable to allocate
*         memory for the new element, then this
*         returns NULL.
*
**************************************************/
struct __map_element *__create_element
(
    void
)
{
    return( (struct __map_element *)malloc( sizeof( struct __map_element ) ) );

}   /* __create_element() */


/**************************************************
*
*   FUNCTION:
*       __free_element_data - "Free Element Data"
*
*   DESCRIPTION:
*       This function frees a map element's data
*
**************************************************/
void __free_element_data
(
   struct __map_element *e  /* element to free  */
)
{
    if( e->key != NULL )
    {
        free( e->key );
        e->key = NULL;
    }

    if( e->val != NULL )
    {
        free( e->val );
        e->val = NULL;
    }

    e->size = 0;

}   /* __free_element_data() */


/**************************************************
*
*   FUNCTION:
*       __free_table - "Free Table"
*
*   DESCRIPTION:
*       This function frees a map's table.
*
**************************************************/
void __free_table
(
    struct __map_element
              **t,      /* the table we're freeing  */
    uint32      cap     /* capacity of the table    */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32                i;    /* for-loop iterator            */
    struct __map_element *cur;  /* pointer to current element   */
    struct __map_element *next; /* pointer to next element      */

    for( i = 0; i < cap; ++i )
    {
        cur = t[ i ];
        while( NULL != cur )
        {
            next = cur->next;
            __free_element_data( cur );
            free( cur );
            cur = next;
        }
    }
    free( t );

}   /* __free_table() */


/**************************************************
*
*   FUNCTION:
*       __get_element - "Get Map Element"
*
*   DESCRIPTION:
*       This function returns a pointer to a
*       map element if its key matches the
*       supplied key. If no such element exists,
*       then this function returns NULL.
*
**************************************************/
struct __map_element *__get_element
(
    struct map *m,      /* map              */
    key_t8      key     /* key to grab      */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    int                     idx;    /* hashed key index     */
    struct __map_element   *cur;    /* current element      */

    /*---------------------------------
    Check for a valid map reference
    ---------------------------------*/
    if( NULL == m )
    {
        return( NULL );
    }

    /*---------------------------------
    Hash the key to an index
    ---------------------------------*/
    idx = __hash_key( key, m->capacity );
    cur = m->table[ idx ];

    /*---------------------------------
    Return the value of the element
    whose key matches the supplied
    key.
    ---------------------------------*/
    while( NULL != cur )
    {
        if( 0 == strcmp( key, cur->key ) )
        {
            return( cur );
        }
        cur = cur->next;
    }

    return( NULL );

}   /* __get_element() */


/**************************************************
*
*   FUNCTION:
*       __hash_key - "Hash Key"
*
*   DESCRIPTION:
*       This function hashes a key and returns
*       an integer between 0 and n.
*
*       Uses FNV-1a hashing algorithm found
*       here:
*
*       http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
*
*   RETURNS:
*       Returns an unsigned integer between
*       0 and n.
*
**************************************************/
uint32 __hash_key
(
    key_t8      key,    /* the key to hash      */
    uint32      n       /* number of elements   */
                        /*  in the hash table   */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    key_t8      ptr;    /* character in key     */
    uint32      idx;    /* index for an array   */

    idx = __INITIAL_HASH_IDX;
    ptr = key;
    while( *ptr != '\0' )
    {
        idx = ( idx ^ *ptr ) * __HASH_PRIME;
        ++ptr;
    }

    return( idx % n );

}   /* __hash_key() */


/**************************************************
*
*   FUNCTION:
*       __init_element - "Initialize Element"
*
*   DESCRIPTION:
*       This initializes an element with the
*       provided values
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * ERR_NULL_REF is returned if the
*         element hasn't been created yet (i.e.
*         it is equal to NULL).
*       * ERR_NO_MEMORY is returned if this
*         function was unable to allocate memory
*         for the element's values.
*       * ERR_NO_ERROR is returned if there were
*         no errors.
*
**************************************************/
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
    uint32  len = strlen( key ) + 1;

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


/**************************************************
*
*   FUNCTION:
*       __init_map - "Initialize Map"
*
*   DESCRIPTION:
*       This initializes a map.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * ERR_NULL_REF is returned if the
*         map hasn't been created yet (i.e.
*         it is equal to NULL).
*       * ERR_NO_MEMORY is returned if this
*         function was unable to allocate memory
*         for the map's values.
*       * ERR_NO_ERROR is returned if there were
*         no errors.
*
**************************************************/
map_error_code_t8 __init_map
(
    struct map *m,      /* map to initialize    */
    uint        size,   /* size of the map      */
    __map_type_t8
                type    /* type of map to init  */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint        i;      /* for-loop iterator    */

    /*---------------------------------
    Check for null reference
    ---------------------------------*/
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }

    /*---------------------------------
    Allocate space for the table
    ---------------------------------*/
    m->table = (struct __map_element **)malloc( sizeof( struct __map_element * ) * size );
    if( NULL == m->table )
    {
        return( ERR_NO_MEMORY );
    }

    /*---------------------------------
    Initialize all table element
    pointers
    ---------------------------------*/
    for( i = 0; i < size; ++i )
    {
        m->table[ i ] = NULL;
    }

    /*---------------------------------
    Set the rest of he stuff
    ---------------------------------*/
    m->size = 0;
    m->capacity = size;
    m->map_type = type;

    return( ERR_NO_ERROR );

}   /* __init_map() */


/**************************************************
*
*   FUNCTION:
*       __resize_map - "Resize Map"
*
*   DESCRIPTION:
*       This resizes a map to the supplied size
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * ERR_NULL_REF is returned if the
*         map hasn't been created yet (i.e.
*         it is equal to NULL).
*       * ERR_NO_MEMORY is returned if this
*         function was unable to allocate memory
*         for the map's values.
*       * ERR_NO_ERROR is returned if there were
*         no errors.
*
**************************************************/
map_error_code_t8 __resize_map
(
    struct map *m,      /* map to resize        */
    uint32      new_size/* new size of the map  */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32                  i;          /* for-loop iterator    */
    map_error_code_t8       error;      /* error code           */
    uint32                  old_cap;    /* old capacity         */
    struct __map_element   *cur;        /* current element      */
    struct __map_element  **old_map;    /* old table            */

    /*---------------------------------
    Check for a valid map reference
    ---------------------------------*/
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }

    /*---------------------------------
    Store the old table values
    ---------------------------------*/
    old_map = m->table;
    old_cap = m->capacity;

    /*---------------------------------
    Resize the map
    ---------------------------------*/
    error = __init_map( m, new_size, __MAP_TYPE_DYNAMIC );
    if( ERR_NO_ERROR != error )
    {
        __free_table( old_map, old_cap );
        return( error );
    }

    /*---------------------------------
    Add all of the elements from the
    old table to the new table
    ---------------------------------*/
    for( i = 0; i < old_cap; ++i )
    {
        cur = old_map[ i ];
        while( NULL != cur )
        {
            error = add_map( m, cur->key, cur->val, cur->size );
            if( 0 == error )
            {
                __free_table( old_map, old_cap );
                return( ERR_ADD_ERROR );
            }

            cur = cur->next;
        }
    }

    /*---------------------------------
    Free the old table
    ---------------------------------*/
    __free_table( old_map, old_cap );

    return( ERR_NO_ERROR );

}   /* __resize_map() */


/**************************************************
*
*   FUNCTION:
*       create_map - "Create Map"
*
*   DESCRIPTION:
*       This creates a map
*
*   RETURNS:
*       Returns a pointer to a map
*
*   ERRORS:
*       * This function returns NULL if a map
*         couldn't be allocated
*
**************************************************/
struct map *create_map
(
    void
)
{
    return( (struct map *)malloc( sizeof( struct map ) ) );

}   /* create() */


/**************************************************
*
*   FUNCTION:
*       init_dynamic_map - "Initialize Dynamic Map"
*
*   DESCRIPTION:
*       This initializes a dynamically allocated
*       map. This means that the map could
*       potentially be resized if the load
*       factor gets large enough.
*
*       Providing a positive integer as n results
*       in the hash map being initialized with n
*       buckets. If a negative number (or zero) is
*       provided, then the table will have the default
*       size of 512 buckets.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * ERR_NULL_REF is returned if the
*         map hasn't been created yet (i.e.
*         it is equal to NULL).
*       * ERR_NO_MEMORY is returned if this
*         function was unable to allocate memory
*         for the map's values.
*       * ERR_NO_ERROR is returned if there were
*         no errors.
*
*   NOTES:
*       * DO NOT USE THIS IF YOU PLAN ON USING
*         THE HANDLES THAT ARE RETURNED WHEN YOU
*         USE THE ADD FUNCTION. THESE HANDLES
*         WILL CHANGE WHEN THE MAP IS RESIZED.
*
**************************************************/
map_error_code_t8 init_dynamic_map
(
    struct map *m,      /* map to initialize    */
    sint        n       /* size of map          */
)
{
    if( n >= 0 )
    {
        return( __init_map( m, n, __MAP_TYPE_DYNAMIC ) );
    }

    return( __init_map( m, __INITIAL_SIZE, __MAP_TYPE_DYNAMIC ) );

}   /* init_dynamic_map() */


/**************************************************
*
*   FUNCTION:
*       init_static_map - "Initialize Static Map"
*
*   DESCRIPTION:
*       This initializes a static map. This map
*       does not get resized even if the load
*       factor gets outrageously large.
*
*       Providing a positive integer as n results
*       in the hash map being initialized with n
*       buckets. If a negative number (or zero) is
*       provided, then the table will have the default
*       size of 512 buckets.
*
*   RETURNS:
*       Returns an error code
*
*   ERRORS:
*       * ERR_NULL_REF is returned if the
*         map hasn't been created yet (i.e.
*         it is equal to NULL).
*       * ERR_NO_MEMORY is returned if this
*         function was unable to allocate memory
*         for the map's values.
*       * ERR_NO_ERROR is returned if there were
*         no errors.
*
**************************************************/
map_error_code_t8 init_static_map
(
    struct map *m,      /* map to initialize    */
    sint        n       /* size of the map      */
)
{
    if( n >= 0 )
    {
        return( __init_map( m, n, __MAP_TYPE_STATIC ) );
    }

    return( __init_map( m, __INITIAL_SIZE, __MAP_TYPE_STATIC ) );

}   /* init_static_map() */


/**************************************************
*
*   FUNCTION:
*       add_map - "Add to Map"
*
*   DESCRIPTION:
*       This adds a key-value pair to a map.
*
*       If an element has the same key as the
*       provided key, then the function
*       will do one of the following:
*           1. The function will return the
*              handle of the element's value
*              data to the user if the values
*              are the same
*           2. The function will replace the
*              element's value data with the
*              new data if the value data is
*              different (and then return
*              the handle to the element's
*              new value data)
*
*   RETURNS:
*       Returns a handle to the value stored
*       in the map element.
*
*   ERRORS:
*       * 0 is returned if there was an error
*         adding to the map.
*
**************************************************/
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
    uint32                  idx;            /* hashed index of the new element  */
    struct __map_element   *new_element;    /* element to add                   */

    /*---------------------------------
    Check if the map reference is valid
    ---------------------------------*/
    if( NULL == m )
    {
        return( 0 );
    }

    /*---------------------------------
    Check if key exists in map already
    ---------------------------------*/
    new_element = __get_element( m, key );
    if( NULL != new_element )
    {
        /*-----------------------------
        Check if values are the same
        -----------------------------*/
        if( 0 == memcmp( val, new_element->val, size ) )
        {
            return( __ptr_to_handle( new_element->val ) );
        }

        /*-----------------------------
        Clear all element data and
        copy over the new stuff
        -----------------------------*/
        __free_element_data( new_element );
        if( ERR_NO_ERROR != __init_element( new_element, key, val, size ) )
        {
            return( 0 );
        }
        return( __ptr_to_handle( new_element->val ) );
    }

    new_element = NULL;

    /*---------------------------------
    Check if we need to resize the map
    ---------------------------------*/
    if( ( __MAP_TYPE_DYNAMIC == m->map_type )
     && ( ( (float)m->size / m->capacity ) >= 1.50f ) )
    {
        if( ERR_NO_ERROR != __resize_map( m, m->capacity << 1 ) )
        {
            return( 0 );
        }
    }

    /*---------------------------------
    Create a new map element
    ---------------------------------*/
    new_element = __create_element();
    if( NULL == new_element )
    {
        return( 0 );
    }

    /*---------------------------------
    Initialize the map element
    ---------------------------------*/
    if( ERR_NO_ERROR != __init_element( new_element, key, val, size ) )
    {
        return( 0 );
    }

    /*---------------------------------
    Do some other stuff
    ---------------------------------*/
    idx = __hash_key( key, m->capacity );
    new_element->next = m->table[ idx ];
    m->table[ idx ] = new_element;
    ++m->size;

    return( (uint32)( new_element->val ) );

}   /* add_map() */


/**************************************************
*
*   FUNCTION:
*       is_in_map - "Is Key in the Map?"
*
*   DESCRIPTION:
*       This determines whether there is an
*       existing element in the map whose key
*       is the same as the supplied key
*
*   RETURNS:
*       Returns TRUE if key is found in the map,
*       and FALSE otherwise.
*
**************************************************/
boolean is_in_map
(
    struct map *m,      /* map                  */
    key_t8      key     /* key to find          */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    struct __map_element   *e;      /* current element      */

    e = __get_element( m, key );
    if( NULL == e )
    {
        return( FALSE );
    }
    return( TRUE );

}   /* is_in_map() */


/**************************************************
*
*   FUNCTION:
*       get - "Get Value from Key"
*
*   DESCRIPTION:
*       This returns the value of an element
*       in the hash table if the keys match.
*
*   RETURNS:
*       Returns a pointer to an element's value
*       if any elements have a key matching the
*       supplied key. If they don't this function
*       returns NULL.
*
**************************************************/
void *get
(
    struct map *m,      /* map                  */
    key_t8      key     /* key to get           */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    struct __map_element   *e;      /* map element          */

    e = __get_element( m, key );
    if( NULL == e )
    {
        return( NULL );
    }
    return( e->val );

}   /* get() */


/**************************************************
*
*   NAME:
*       get_map_size - "Get Map Size"
*
*   DESCRIPTION:
*       Returns the size of the map. Size, in
*       this context, is the number of elements
*       in the map.
*
*   ERRORS:
*       * Returns 0 if there is an error (or
*         if there are no elements in the map).
*
**************************************************/
uint32 get_map_size
(
    struct map *m       /* map                  */
)
{
    if( NULL == m )
    {
        return( 0 );
    }
    return( m->size );

}   /* get_map_size() */


/**************************************************
*
*   NAME:
*       get_map_capacity - "Get Map Capacity"
*
*   DESCRIPTION:
*       Returns the capacity of the map. Capacity,
*       in this context, is the number of buckets
*       in the map.
*
*   ERRORS:
*       * Returns 0 if there is an error (or if
*         there are no buckets).
*
**************************************************/
uint32 get_map_capacity
(
    struct map *m       /* map                  */
)
{
    if( NULL == m )
    {
        return( 0 );
    }
    return( m->capacity );

}   /* get_map_capacity() */


/**************************************************
*
*   FUNCTION:
*       free_map - "Free Map"
*
*   DESCRIPTION:
*       This frees a map from memory
*
**************************************************/
void free_map
(
    struct map *m       /* map to free          */
)
{
    /*---------------------------------
    Check for valid map reference
    ---------------------------------*/
    if( NULL == m )
    {
        return;
    }

    /*---------------------------------
    Free the table, and then free the
    map
    ---------------------------------*/
    __free_table( m->table, m->capacity );
    free( m );

}   /* free_map() */


/**************************************************
*
*   FUNCTION:
*       show_map - "Show Map"
*
*   DESCRIPTION:
*       Prints the hash table.
*
*   NOTE:
*       A callback function is required to be
*       provided as a parameter for this.
*       The callback function should be designed
*       to print out whatever value is stored
*       with the key.
*
**************************************************/
map_error_code_t8 show_map
(
    struct map     *m,          /* map to print             */
    disp_callback   disp_func   /* display function to use  */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    uint32                  i;      /* for-loop iterator    */
    struct __map_element   *cur;    /* current map element  */

    /*---------------------------------
    Ensure the map reference is valid
    ---------------------------------*/
    if( NULL == m )
    {
        return( ERR_NULL_REF );
    }

    /*---------------------------------
    Loop through all non-null entries
    in the map, print the keys, and
    call the callback function to
    print the values
    ---------------------------------*/
    for( i = 0; i < m->capacity; ++i )
    {
        cur = m->table[ i ];
        while( NULL != cur )
        {
            printf( "Key: %s\t\tValue: ", cur->key );
            disp_func( cur->val );
            cur = cur->next;
        }
    }

    return( ERR_NO_ERROR );

}   /* show_map() */
