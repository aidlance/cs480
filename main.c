/**************************************************
*
*   MODULE NAME:
*       main.c
*
*   DESCRIPTION:
*       I don't think I need on for this file
*
**************************************************/

/*-------------------------------------------------
                  GENERAL INCLUDES
-------------------------------------------------*/
#include <stdio.h>

#include "parser.h"
#include "code_gen.h"

/*-------------------------------------------------
                LITERAL CONSTANTS
-------------------------------------------------*/

#define FILENAME_IDX   1    /* index for filenames to start     */
#define NUM_TESTS_PASS 51   /* number of tests that should pass */
#define NUM_TESTS_FAIL 31   /* number of tests that should fail */

#define __CODEBLOCKS_TEST

/*-------------------------------------------------
                    PROCEDURES
-------------------------------------------------*/

/**************************************************
*
*   FUNCTION:
*       main - "Main"
*
*   DESCRIPTION:
*       The main function that does everything.
*       I really don't think that I need this.
*
**************************************************/
int main
(
    int     argc,   /* number of command line arguments */
    char  **argv    /* the command line arguments       */
)
{
    /*---------------------------------
    Local variables
    ---------------------------------*/
    int     grr;        /* for-loop iterator        */
    int     i;          /* pronounced "eye"         */
    char    fname[50];  /* filename                 */

    /*---------------------------------
    Initialize the parser
    ---------------------------------*/
    init_parser();

#ifdef __CODEBLOCKS_TEST
    /*---------------------------------
    Loop through all passing tests
    and create the Gforth code files
    only if the parser can parse the
    file.
    ---------------------------------*/
    for( i = 0; i < NUM_TESTS_PASS; ++i )
    {
        sprintf( fname, "test%i.txt", i + 1 );
        load_file( fname );
        if( PARSER_NO_ERROR == parse_file() )
        {
            if( CGEN_NO_ERROR != gen_code( fname ) )
            {
                printf( "%s: semantics ERROR\n", fname );
            }
        }
        else
        {
            printf("%s: PARSER ERROR\n", fname );
        }
        unload_file();
    }

    /*---------------------------------
    Loop through all failing tests
    and make sure that they fail
    ---------------------------------*/
    for( i = 0; i < NUM_TESTS_FAIL; ++i )
    {
        sprintf( fname, "test_fail%i.txt", i + 1 );
        load_file( fname );
        if( PARSER_NO_ERROR == parse_file() )
        {
            if( CGEN_NO_ERROR != gen_code( fname ) )
            {
                printf( "%s: semantics ERROR\n", fname );
            }
        }
        else
        {
            printf("%s: PARSER ERROR\n", fname );
        }
        unload_file();
    }
#else
    if( argc > 1 )
    {
        /*-----------------------------
        Loop through all files supplied
        to the program and create the
        code files for each program
        -----------------------------*/
        for( grr = 1; grr < argc; ++grr )
        {
            if( '-' != *argv[grr] )
            {
                for( i = 0; i < 32; ++i ) printf( "-" );
                printf("\nFile: %s\n", argv[grr]);
                for( i = 0; i < 32; ++i ) printf( "-" );
                printf("\n");

                if( PARSER_NO_ERROR != load_file( argv[ grr ] ) )
                {
                    continue;
                }

                if( PARSER_NO_ERROR == parse_file() )
                {
                    if( CGEN_NO_ERROR != gen_code( argv[ grr ] ) )
                    {
                        printf( "%s: semantics ERROR\n\n\n", argv[ grr ] );
                    }
                    else
                    {

                        printf( "%s: no errors\n\n\n", argv[ grr]);
                    }
                }
                else
                {
                    printf("%s: PARSER ERROR\n\n\n", argv[grr] );
                }
                unload_file();
            }
        }
    }
#endif

    //printf( "\nThere were %lu scanning errors.\n", get_total_scanner_errors() );

#ifdef __CODEBLOCKS_TEST
    scanf("%i", &grr);
#endif

    /*---------------------------------
    Unload the parser
    ---------------------------------*/
    unload_parser();
    return( 0 );

}   /* main() */
