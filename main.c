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

/*------------------------------------------------------------------------------------------
                                     LITERAL CONSTANTS
------------------------------------------------------------------------------------------*/
#define FILENAME_IDX 1
#define __CODEBLOCKS_TEST

/*------------------------------------------------------------------------------------------
                                      PROCEDURES
------------------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    int grr, i;
    init_parser();

#ifdef __CODEBLOCKS_TEST
    load_file( "test1.txt" );
    parse_file();
    unload_file();

    load_file( "test2.txt" );
    parse_file();
    unload_file();

    load_file( "test3.txt" );
    parse_file();
    unload_file();

    load_file( "test4.txt" );
    parse_file();
    unload_file();

    load_file( "test5.txt" );
    parse_file();
    unload_file();

    load_file( "test6.txt" );
    parse_file();
    unload_file();

    load_file( "test7.txt" );
    parse_file();
    unload_file();

    load_file( "test8.txt" );
    parse_file();
    unload_file();

    load_file( "test9.txt" );
    parse_file();
    unload_file();

    load_file( "test10.txt" );
    parse_file();
    unload_file();

    load_file( "test11.txt" );
    parse_file();
    unload_file();

    load_file( "test12.txt" );
    parse_file();
    unload_file();

    load_file( "test13.txt" );
    parse_file();
    unload_file();

    load_file( "test14.txt" );
    parse_file();
    unload_file();

    load_file( "test15.txt" );
    parse_file();
    unload_file();

    load_file( "test16.txt" );
    parse_file();
    unload_file();

    load_file( "test17.txt" );
    parse_file();
    unload_file();

    load_file( "test18.txt" );
    parse_file();
    unload_file();

    load_file( "test19.txt" );
    parse_file();
    unload_file();

    load_file( "test20.txt" );
    parse_file();
    unload_file();

    load_file( "test21.txt" );
    parse_file();
    unload_file();

    load_file( "test22.txt" );
    parse_file();
    unload_file();

    load_file( "test23.txt" );
    parse_file();
    unload_file();
#else
    if( argc > 1 )
    {
        for( grr = 1; grr < argc; ++grr )
        {
            if( '-' != *argv )
            {
                for( i = 0; i < 32; ++i ) printf( "-" );
                printf("\nFile: %s\n");
                for( i = 0; i < 32; ++i ) printf( "-" );
                printf("\n");
                if( PARSER_NO_ERROR != load_file( argv[ grr ] ) )
                {
                    continue;
                }
                parse_file();
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
