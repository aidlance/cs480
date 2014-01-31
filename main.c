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

/*------------------------------------------------------------------------------------------
                                          TYPES
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                    VARIABLE CONSTANTS
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                        MACROS
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                      PROCEDURES
------------------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    int grr;
    init_scanner();
    tokenize( "test.txt");

scanf("%i", &grr);
unload_scanner();
    return( 0 );

}   /* main() */
