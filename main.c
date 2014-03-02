/***************************************************************************************//**
 *
 *  MODULE NAME:
 *      [ filename ] - [ short description ]
 *
 *  DESCRIPTION:
 *      [ description of file's usability ]
 *
 *  \author Aidan Lance
 *  \date   [ date ]
 *
 ******************************************************************************************/

/*------------------------------------------------------------------------------------------
                                     GENERAL INCLUDES
------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "parser.h"
#include "code_gen.h"

/*------------------------------------------------------------------------------------------
                                     LITERAL CONSTANTS
------------------------------------------------------------------------------------------*/
#define FILENAME_IDX 1
#define NUM_TESTS_PASS 41
#define NUM_TESTS_FAIL 5
//#define __CODEBLOCKS_TEST

/*------------------------------------------------------------------------------------------
                                      PROCEDURES
------------------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    int grr, i;
    init_parser();
    char fname[50];

#ifdef __CODEBLOCKS_TEST
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

    unload_parser();
    return( 0 );

}   /* main() */
