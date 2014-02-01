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
#include "scanner.h"

/*------------------------------------------------------------------------------------------
                                     LITERAL CONSTANTS
------------------------------------------------------------------------------------------*/
#define FILENAME_IDX 1
//#define __CODEBLOCKS_TEST

/*------------------------------------------------------------------------------------------
                                      PROCEDURES
------------------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    int grr;
    init_scanner();

#ifdef __CODEBLOCKS_TEST
    tokenize( "test.txt" );
#else
    if( argc > 1 )
    {
        for( grr = 1; grr < argc; ++grr )
        {
            tokenize( argv[ FILENAME_IDX ] );
        }
    }
#endif

    printf( "\nThere were %lu scanning errors.\n", get_total_scanner_errors() );
    
#ifdef __CODEBLOCKS_TEST
    scanf("%i", &grr);
#endif
    unload_scanner();
    return( 0 );

}   /* main() */
